// DatabaseManager.h - PostgreSQL connection management

#ifndef SDRS_COMMON_DATABASE_MANAGER_H
#define SDRS_COMMON_DATABASE_MANAGER_H

#include <pqxx/pqxx>
#include <string>
#include <format>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <stdexcept>

namespace sdrs::database
{

// Database configuration
struct DatabaseConfig
{
    std::string host = "localhost";
    int port = 5432;
    std::string database = "sdrs";
    std::string user = "sdrs_user";
    std::string password = "sdrs_password";
    int poolSize = 5;
    int connectionTimeout = 10;
    
    std::string getConnectionString() const
    {
        return std::format(
            "host={} port={} dbname={} user={} password={} connect_time_out={}",
            host,
            port,
            database,
            user,
            password,
            connectionTimeout
        );
    }
};

// RAII wrapper for connection from pool
class PooledConnection
{
private:
    std::shared_ptr<pqxx::connection> _connection;
    std::function<void(std::shared_ptr<pqxx::connection>)> _returnToPool;

public:
    PooledConnection(std::shared_ptr<pqxx::connection> conn,
                     std::function<void(std::shared_ptr<pqxx::connection>)> returnFunc)
        : _connection(std::move(conn)), _returnToPool(std::move(returnFunc))
    {
        // Do nothing
    }
    
    ~PooledConnection()
    {
        if (_connection && _returnToPool)
        {
            _returnToPool(_connection);
        }
    }
    
    // Non-copyable
    PooledConnection(const PooledConnection&) = delete;
    PooledConnection& operator=(const PooledConnection&) = delete;
    
    // Movable
    PooledConnection(PooledConnection&& other) noexcept
        : _connection(std::move(other._connection)),
          _returnToPool(std::move(other._returnToPool))
    {
        other._connection = nullptr;
    }
    
    PooledConnection& operator=(PooledConnection&& other) noexcept
    {
        if (this != &other)
        {
            if (_connection && _returnToPool)
            {
                _returnToPool(_connection);
            }
            _connection = std::move(other._connection);
            _returnToPool = std::move(other._returnToPool);
            other._connection = nullptr;
        }
        return *this;
    }
    
    pqxx::connection& get() { return *_connection; }
    pqxx::connection* operator->() { return _connection.get(); }
};

// Connection pool for PostgreSQL
class ConnectionPool
{
private:
    std::queue<std::shared_ptr<pqxx::connection>> _pool;
    std::mutex _mutex;
    std::condition_variable _condition;
    DatabaseConfig _config;
    int _maxSize;
    int _currentSize;
    bool _stopped;

public:
    explicit ConnectionPool(const DatabaseConfig& config)
        : _config(config), _maxSize(config.poolSize), _currentSize(0), _stopped(false)
    {
        // Pre-create connections
        for (int i = 0; i < _maxSize; ++i)
        {
            try
            {
                auto conn = std::make_shared<pqxx::connection>(_config.getConnectionString());
                _pool.push(conn);
                ++_currentSize;
            }
            catch (const std::exception& e)
            {
                // Log error but continue - pool will create connections on demand
                break;
            }
        }
    }
    
    ~ConnectionPool()
    {
        shutdown();
    }
    
    PooledConnection acquire()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        
        // Wait for available connection
        _condition.wait(lock, [this] {
            return !_pool.empty() || _stopped;
        });
        
        if (_stopped)
        {
            throw std::runtime_error("Connection pool is stopped");
        }
        
        auto conn = _pool.front();
        _pool.pop();
        
        // Check if connection is still valid
        if (!conn->is_open())
        {
            try
            {
                conn = std::make_shared<pqxx::connection>(_config.getConnectionString());
            }
            catch (const std::exception& e)
            {
                // Return invalid connection to trigger retry
                throw std::runtime_error("Failed to reconnect: " + std::string(e.what()));
            }
        }
        
        return PooledConnection(conn, [this](std::shared_ptr<pqxx::connection> c) {
            release(c);
        });
    }
    
    void release(std::shared_ptr<pqxx::connection> conn)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_stopped)
        {
            _pool.push(conn);
            _condition.notify_one();
        }
    }
    
    void shutdown()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stopped = true;
        while (!_pool.empty())
        {
            _pool.pop();
        }
        _condition.notify_all();
    }
    
    size_t availableConnections() const
    {
        return _pool.size();
    }
    
    bool isHealthy() const
    {
        return !_stopped && _currentSize > 0;
    }
};

// Singleton Database Manager
class DatabaseManager
{
private:
    std::unique_ptr<ConnectionPool> _pool;
    DatabaseConfig _config;
    bool _initialized;
    
    DatabaseManager() : _initialized(false) {}

public:
    // Singleton instance
    static DatabaseManager& getInstance()
    {
        static DatabaseManager instance;
        return instance;
    }
    
    // Non-copyable
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    // Initialize with config
    void initialize(const DatabaseConfig& config)
    {
        if (_initialized)
        {
            return;
        }
        
        _config = config;
        _pool = std::make_unique<ConnectionPool>(_config);
        _initialized = true;
    }
    
    // Initialize from environment variables
    void initializeFromEnv()
    {
        DatabaseConfig config;
        
        if (const char* host = std::getenv("SDRS_DB_HOST"))
            config.host = host;
        if (const char* port = std::getenv("SDRS_DB_PORT"))
            config.port = std::stoi(port);
        if (const char* db = std::getenv("SDRS_DB_NAME"))
            config.database = db;
        if (const char* user = std::getenv("SDRS_DB_USER"))
            config.user = user;
        if (const char* pass = std::getenv("SDRS_DB_PASSWORD"))
            config.password = pass;
        if (const char* poolSize = std::getenv("SDRS_DB_POOL_SIZE"))
            config.poolSize = std::stoi(poolSize);
            
        initialize(config);
    }
    
    // Get connection from pool
    PooledConnection getConnection()
    {
        if (!_initialized)
        {
            throw std::runtime_error("DatabaseManager not initialized. Call initialize() first.");
        }
        return _pool->acquire();
    }
    
    // Execute query with result
    template<typename Func>
    auto executeQuery(Func&& func) -> decltype(func(std::declval<pqxx::work&>()))
    {
        auto conn = getConnection();
        pqxx::work txn(conn.get());
        
        try
        {
            auto result = func(txn);
            txn.commit();
            return result;
        }
        catch (...)
        {
            txn.abort();
            throw;
        }
    }
    
    // Execute query without result (INSERT, UPDATE, DELETE)
    template<typename Func>
    void executeCommand(Func&& func)
    {
        auto conn = getConnection();
        pqxx::work txn(conn.get());
        
        try
        {
            func(txn);
            txn.commit();
        }
        catch (...)
        {
            txn.abort();
            throw;
        }
    }
    
    // Health check
    bool isHealthy() const
    {
        return _initialized && _pool && _pool->isHealthy();
    }
    
    // Shutdown
    void shutdown()
    {
        if (_pool)
        {
            _pool->shutdown();
        }
        _initialized = false;
    }
    
    const DatabaseConfig& getConfig() const { return _config; }
};

} // namespace sdrs::database

#endif // SDRS_COMMON_DATABASE_MANAGER_H
