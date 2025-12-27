// Logger.cpp - Implementation

#include "../../include/utils/Logger.h"

namespace sdrs::utils
{

// Static members initialization
LogLevel Logger::s_globalLevel = LogLevel::INFO;
std::mutex Logger::s_mutex;

// Constructors

Logger::Logger()
    : _name("App"),
      _minLevel(LogLevel::INFO),
      _writeToFile(false),
      _logFilePath("")
{
    // Do nothing
}

Logger::Logger(const std::string& name)
    : _name(name),
      _minLevel(LogLevel::INFO),
      _writeToFile(false),
      _logFilePath("")
{
    // Do nothing
}

Logger::Logger(const std::string& name, LogLevel minLevel, const std::string& logFilePath)
    : _name(name),
      _minLevel(minLevel),
      _writeToFile(!logFilePath.empty()),
      _logFilePath(logFilePath)
{
    // Do nothing
}

// Private methods

std::string Logger::getCurrentTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif
    
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return std::string(buffer);
}

std::string Logger::levelToString(LogLevel level) const
{
    switch (level)
    {
        case LogLevel::DEBUG: return "[DEBUG]";
        case LogLevel::INFO:  return "[INFO] ";
        case LogLevel::WARN:  return "[WARN] ";
        case LogLevel::ERROR: return "[ERROR]";
        default:              return "[?????]";
    }
}

void Logger::writeLog(LogLevel level, const std::string& message) const
{
    if (level < _minLevel)
    {
        return;
    }
    
    std::string logLine = std::format(
        "{} {} [{}] {}",
        getCurrentTimestamp(),
        levelToString(level),
        _name,
        message
    );
    
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // WARN and ERROR go to stderr, others to stdout
    if (level >= LogLevel::WARN)
    {
        std::cerr << logLine << std::endl;
    }
    else
    {
        std::cout << logLine << std::endl;
    }
    
    // Write to file if enabled
    if (_writeToFile && !_logFilePath.empty())
    {
        std::ofstream file(_logFilePath, std::ios::app);
        if (file.is_open())
        {
            file << logLine << std::endl;
            file.close();
        }
    }
}

// Instance logging methods

void Logger::debug(const std::string& message) const
{
    writeLog(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) const
{
    writeLog(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) const
{
    writeLog(LogLevel::WARN, message);
}

void Logger::error(const std::string& message) const
{
    writeLog(LogLevel::ERROR, message);
}

// Static logging methods

void Logger::Debug(const std::string& message)
{
    if (s_globalLevel <= LogLevel::DEBUG)
    {
        Logger temp("App", s_globalLevel);
        temp.debug(message);
    }
}

void Logger::Info(const std::string& message)
{
    if (s_globalLevel <= LogLevel::INFO)
    {
        Logger temp("App", s_globalLevel);
        temp.info(message);
    }
}

void Logger::Warn(const std::string& message)
{
    if (s_globalLevel <= LogLevel::WARN)
    {
        Logger temp("App", s_globalLevel);
        temp.warn(message);
    }
}

void Logger::Error(const std::string& message)
{
    Logger temp("App", s_globalLevel);
    temp.error(message);
}

// Configuration

void Logger::setLevel(LogLevel level)
{
    _minLevel = level;
}

void Logger::setGlobalLevel(LogLevel level)
{
    s_globalLevel = level;
}

void Logger::enableFileLogging(const std::string& filePath)
{
    _writeToFile = true;
    _logFilePath = filePath;
}

void Logger::disableFileLogging()
{
    _writeToFile = false;
}

// Getters

std::string Logger::getName() const
{
    return _name;
}

LogLevel Logger::getLevel() const
{
    return _minLevel;
}

} // namespace sdrs::utils
