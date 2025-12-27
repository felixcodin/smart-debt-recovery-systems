// Config.h - Application configuration manager (Singleton)

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <optional>
#include "Constants.h"

namespace sdrs::utils
{

// Singleton configuration manager
// Loads config from environment variables and/or config files
class Config
{
private:
    std::map<std::string, std::string> _configMap;
    std::string _configFilePath;
    bool _isLoaded;
    
    // Singleton: private constructor
    Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    void loadFromEnvironment();
    void loadFromFile(const std::string& filePath);
    void parseLine(const std::string& line);

public:
    // Get singleton instance
    static Config& getInstance();
    
    // Load configuration
    void load(const std::string& filePath);
    void loadFromEnv();
    void reload();
    
    // Get config values with type conversion
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    double getDouble(const std::string& key, double defaultValue = 0.0) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    bool hasKey(const std::string& key) const;
    std::optional<std::string> get(const std::string& key) const;
    
    // Set config value at runtime
    void set(const std::string& key, const std::string& value);
    
    // Utility
    void printAll() const;
    void clear();
    
    // Convenience getters (use Constants.h defaults)
    std::string getDatabaseHost() const;
    int getDatabasePort() const;
    std::string getDatabaseName() const;
    std::string getDatabaseUser() const;
    std::string getDatabasePassword() const;
    
    int getApiGatewayPort() const;
    int getBorrowerServicePort() const;
    int getRiskServicePort() const;
    int getRecoveryServicePort() const;
    int getCommunicationServicePort() const;
};

} // namespace sdrs::utils

#endif // CONFIG_H
