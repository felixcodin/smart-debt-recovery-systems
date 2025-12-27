// Logger.h - Application logging utility

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <format>
#include <mutex>

namespace sdrs::utils
{

// Log severity levels: DEBUG < INFO < WARN < ERROR
enum class LogLevel
{
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

// Thread-safe logger with console and file output support
class Logger
{
private:
    std::string _name;
    LogLevel _minLevel;
    bool _writeToFile;
    std::string _logFilePath;
    
    static LogLevel s_globalLevel;
    static std::mutex s_mutex;
    
    std::string getCurrentTimestamp() const;
    std::string levelToString(LogLevel level) const;
    void writeLog(LogLevel level, const std::string& message) const;

public:
    Logger();
    explicit Logger(const std::string& name);
    Logger(const std::string& name, LogLevel minLevel, const std::string& logFilePath = "");
    
    // Instance logging methods
    void debug(const std::string& message) const;
    void info(const std::string& message) const;
    void warn(const std::string& message) const;
    void error(const std::string& message) const;
    
    // Static logging methods (quick access without creating instance)
    static void Debug(const std::string& message);
    static void Info(const std::string& message);
    static void Warn(const std::string& message);
    static void Error(const std::string& message);
    
    // Configuration
    void setLevel(LogLevel level);
    static void setGlobalLevel(LogLevel level);
    void enableFileLogging(const std::string& filePath);
    void disableFileLogging();
    
    std::string getName() const;
    LogLevel getLevel() const;
};

} // namespace sdrs::utils

#endif // LOGGER_H
