#include "../../include/registry/ServiceRegistry.h"
#include "../../../common/include/utils/Logger.h"
#include <httplib.h>

namespace sdrs::gateway
{

ServiceRegistry::ServiceRegistry(int maxFailures)
    : _maxFailures(maxFailures)
{
}

void ServiceRegistry::registerService(const std::string& name, const std::string& host, int port)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    ServiceInfo info;
    info.name = name;
    info.host = host;
    info.port = port;
    info.isHealthy = true;
    info.lastCheck = std::chrono::steady_clock::now();
    info.consecutiveFailures = 0;
    
    _services[name] = info;
    
    sdrs::utils::Logger::Info("[ServiceRegistry] Registered service: " + name + 
                              " at " + host + ":" + std::to_string(port));
}

bool ServiceRegistry::isServiceHealthy(const std::string& name)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _services.find(name);
    if (it == _services.end())
    {
        return false;
    }
    
    // Check if we need to refresh health status (every 30 seconds)
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.lastCheck);
    
    if (elapsed.count() >= 30)
    {
        return checkServiceHealth(name);
    }
    
    return it->second.isHealthy;
}

ServiceInfo ServiceRegistry::getService(const std::string& name)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _services[name];
}

void ServiceRegistry::markServiceHealthy(const std::string& name)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _services.find(name);
    if (it != _services.end())
    {
        it->second.isHealthy = true;
        it->second.consecutiveFailures = 0;
        it->second.lastCheck = std::chrono::steady_clock::now();
    }
}

void ServiceRegistry::markServiceUnhealthy(const std::string& name)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _services.find(name);
    if (it != _services.end())
    {
        it->second.consecutiveFailures++;
        it->second.lastCheck = std::chrono::steady_clock::now();
        
        if (it->second.consecutiveFailures >= _maxFailures)
        {
            it->second.isHealthy = false;
            sdrs::utils::Logger::Error("[ServiceRegistry] Service marked unhealthy: " + name + 
                                       " (failures: " + std::to_string(it->second.consecutiveFailures) + ")");
        }
    }
}

std::map<std::string, ServiceInfo> ServiceRegistry::getAllServices()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _services;
}

bool ServiceRegistry::checkServiceHealth(const std::string& name)
{
    auto it = _services.find(name);
    if (it == _services.end())
    {
        return false;
    }
    
    try
    {
        httplib::Client client(it->second.host.c_str(), it->second.port);
        client.set_connection_timeout(2, 0);
        
        auto result = client.Get("/health");
        
        if (result && result->status == 200)
        {
            it->second.isHealthy = true;
            it->second.consecutiveFailures = 0;
            it->second.lastCheck = std::chrono::steady_clock::now();
            return true;
        }
        else
        {
            it->second.consecutiveFailures++;
            if (it->second.consecutiveFailures >= _maxFailures)
            {
                it->second.isHealthy = false;
            }
            it->second.lastCheck = std::chrono::steady_clock::now();
            return false;
        }
    }
    catch (...)
    {
        it->second.consecutiveFailures++;
        if (it->second.consecutiveFailures >= _maxFailures)
        {
            it->second.isHealthy = false;
        }
        it->second.lastCheck = std::chrono::steady_clock::now();
        return false;
    }
}

} // namespace sdrs::gateway
