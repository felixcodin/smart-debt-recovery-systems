#pragma once

#include <string>
#include <map>
#include <chrono>
#include <mutex>

namespace sdrs::gateway
{

struct ServiceInfo
{
    std::string name;
    std::string host;
    int port;
    bool isHealthy;
    std::chrono::steady_clock::time_point lastCheck;
    int consecutiveFailures;
};

class ServiceRegistry
{
private:
    std::map<std::string, ServiceInfo> _services;
    std::mutex _mutex;
    int _maxFailures;
    
public:
    explicit ServiceRegistry(int maxFailures = 3);
    
    void registerService(const std::string& name, const std::string& host, int port);
    bool isServiceHealthy(const std::string& name);
    ServiceInfo getService(const std::string& name);
    void markServiceHealthy(const std::string& name);
    void markServiceUnhealthy(const std::string& name);
    std::map<std::string, ServiceInfo> getAllServices();
    
private:
    bool checkServiceHealth(const std::string& name);
};

} // namespace sdrs::gateway
