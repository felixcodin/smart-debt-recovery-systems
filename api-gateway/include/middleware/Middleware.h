#pragma once

#include <string>
#include <functional>
#include <memory>
#include <httplib.h>

namespace sdrs::gateway
{

// Middleware interface - each middleware can process request/response
class Middleware
{
public:
    using Handler = std::function<void(const httplib::Request&, httplib::Response&)>;
    
    virtual ~Middleware() = default;
    
    // Process request before forwarding to next middleware or handler
    virtual bool process(const httplib::Request& req, httplib::Response& res, Handler next) = 0;
};

// Logging middleware - logs all requests
class LoggingMiddleware : public Middleware
{
public:
    bool process(const httplib::Request& req, httplib::Response& res, Handler next) override;
};

// Authentication middleware - validates API keys or JWT tokens
class AuthenticationMiddleware : public Middleware
{
private:
    bool _requireAuth;
    
public:
    explicit AuthenticationMiddleware(bool requireAuth = true);
    bool process(const httplib::Request& req, httplib::Response& res, Handler next) override;
    
private:
    bool validateApiKey(const std::string& apiKey) const;
    bool validateJWT(const std::string& token) const;
};

// Rate limiting middleware - prevents abuse
class RateLimitMiddleware : public Middleware
{
private:
    struct ClientStats
    {
        int requestCount;
        std::chrono::steady_clock::time_point windowStart;
    };
    
    std::map<std::string, ClientStats> _clientStats;
    int _maxRequestsPerMinute;
    std::mutex _mutex;
    
public:
    explicit RateLimitMiddleware(int maxRequestsPerMinute = 60);
    bool process(const httplib::Request& req, httplib::Response& res, Handler next) override;
    
private:
    std::string getClientIdentifier(const httplib::Request& req) const;
    bool isRateLimited(const std::string& clientId);
};

// CORS middleware - handles cross-origin requests
class CORSMiddleware : public Middleware
{
public:
    bool process(const httplib::Request& req, httplib::Response& res, Handler next) override;
};

// Middleware chain - executes middlewares in order
class MiddlewareChain
{
private:
    std::vector<std::shared_ptr<Middleware>> _middlewares;
    
public:
    void add(std::shared_ptr<Middleware> middleware);
    void execute(const httplib::Request& req, httplib::Response& res, 
                Middleware::Handler finalHandler);
};

} // namespace sdrs::gateway
