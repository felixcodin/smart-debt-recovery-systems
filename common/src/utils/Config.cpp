// Config.cpp - Implementation

#include "../../include/utils/Config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>

using namespace sdrs::constants;

namespace sdrs::utils
{

// Constructor (private - singleton)

Config::Config()
    : _configFilePath(""),
      _isLoaded(false)
{
    loadFromEnvironment();
}

// Singleton instance

Config& Config::getInstance()
{
    static Config instance;
    return instance;
}

// Load methods

void Config::load(const std::string& filePath)
{
    _configFilePath = filePath;
    loadFromFile(filePath);
    _isLoaded = true;
}

void Config::loadFromEnv()
{
    loadFromEnvironment();
    _isLoaded = true;
}

void Config::reload()
{
    _configMap.clear();
    loadFromEnvironment();
    
    if (!_configFilePath.empty())
    {
        loadFromFile(_configFilePath);
    }
}

void Config::loadFromEnvironment()
{
    const char* commonKeys[] = {
        "DB_HOST", "DB_PORT", "DB_NAME", "DB_USER", "DB_PASSWORD",
        "API_PORT", "LOG_LEVEL", "LOG_FILE",
        "BORROWER_SERVICE_PORT", "RISK_SERVICE_PORT",
        "RECOVERY_SERVICE_PORT", "COMMUNICATION_SERVICE_PORT"
    };
    
    for (const char* key : commonKeys)
    {
        const char* value = std::getenv(key);
        if (value != nullptr)
        {
            _configMap[key] = value;
        }
    }
}

void Config::loadFromFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    
    if (!file.is_open())
    {
        std::cerr << "[Config] Warning: Cannot open config file: " << filePath << "\n";
        return;
    }
    
    std::string line;
    while (std::getline(file, line))
    {
        parseLine(line);
    }
    
    file.close();
    std::cout << "[Config] Loaded " << _configMap.size() << " configs from " << filePath << "\n";
}

void Config::parseLine(const std::string& line)
{
    if (line.empty())
    {
        return;
    }
    
    // Skip comments
    if (line[0] == '#' || (line.length() > 1 && line[0] == '/' && line[1] == '/'))
    {
        return;
    }
    
    size_t equalPos = line.find('=');
    if (equalPos == std::string::npos)
    {
        return;
    }
    
    std::string key = line.substr(0, equalPos);
    std::string value = line.substr(equalPos + 1);
    
    // Trim whitespace
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t\r\n") + 1);
    
    // Remove surrounding quotes
    if (value.length() >= 2)
    {
        if ((value.front() == '"' && value.back() == '"') ||
            (value.front() == '\'' && value.back() == '\''))
        {
            value = value.substr(1, value.length() - 2);
        }
    }
    
    if (!key.empty())
    {
        _configMap[key] = value;
    }
}

// Getter methods

std::string Config::getString(const std::string& key, const std::string& defaultValue) const
{
    auto it = _configMap.find(key);
    if (it != _configMap.end())
    {
        return it->second;
    }
    return defaultValue;
}

int Config::getInt(const std::string& key, int defaultValue) const
{
    auto it = _configMap.find(key);
    if (it != _configMap.end())
    {
        try
        {
            return std::stoi(it->second);
        }
        catch (const std::exception&)
        {
            return defaultValue;
        }
    }
    return defaultValue;
}

double Config::getDouble(const std::string& key, double defaultValue) const
{
    auto it = _configMap.find(key);
    if (it != _configMap.end())
    {
        try
        {
            return std::stod(it->second);
        }
        catch (const std::exception&)
        {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool Config::getBool(const std::string& key, bool defaultValue) const
{
    auto it = _configMap.find(key);
    if (it != _configMap.end())
    {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        
        if (value == "true" || value == "1" || value == "yes" || value == "on")
        {
            return true;
        }
        if (value == "false" || value == "0" || value == "no" || value == "off")
        {
            return false;
        }
    }
    return defaultValue;
}

bool Config::hasKey(const std::string& key) const
{
    return _configMap.find(key) != _configMap.end();
}

std::optional<std::string> Config::get(const std::string& key) const
{
    auto it = _configMap.find(key);
    if (it != _configMap.end())
    {
        return it->second;
    }
    return std::nullopt;
}

// Setter

void Config::set(const std::string& key, const std::string& value)
{
    _configMap[key] = value;
}

// Utility methods

void Config::printAll() const
{
    std::cout << "=== Current Configuration ===\n";
    for (const auto& [key, value] : _configMap)
    {
        // Hide sensitive values
        if (key.find("PASSWORD") != std::string::npos ||
            key.find("SECRET") != std::string::npos)
        {
            std::cout << key << " = ********\n";
        }
        else
        {
            std::cout << key << " = " << value << "\n";
        }
    }
    std::cout << "=============================\n";
}

void Config::clear()
{
    _configMap.clear();
    _isLoaded = false;
}

// Convenience getters

std::string Config::getDatabaseHost() const
{
    return getString("DB_HOST", database::DB_HOST);
}

int Config::getDatabasePort() const
{
    return getInt("DB_PORT", database::DB_PORT);
}

std::string Config::getDatabaseName() const
{
    return getString("DB_NAME", database::DB_NAME);
}

std::string Config::getDatabaseUser() const
{
    return getString("DB_USER", database::DB_USER);
}

std::string Config::getDatabasePassword() const
{
    return getString("DB_PASSWORD", database::DB_PASSWORD);
}

int Config::getApiGatewayPort() const
{
    return getInt("API_PORT", ports::API_GATEWAY_PORT);
}

int Config::getBorrowerServicePort() const
{
    return getInt("BORROWER_SERVICE_PORT", ports::BORROWER_SERVICE_PORT);
}

int Config::getRiskServicePort() const
{
    return getInt("RISK_SERVICE_PORT", ports::RISK_SERVICE_PORT);
}

int Config::getRecoveryServicePort() const
{
    return getInt("RECOVERY_SERVICE_PORT", ports::RECOVERY_SERVICE_PORT);
}

int Config::getCommunicationServicePort() const
{
    return getInt("COMMUNICATION_SERVICE_PORT", ports::COMMUNICATION_SERVICE_PORT);
}

} // namespace sdrs::utils
