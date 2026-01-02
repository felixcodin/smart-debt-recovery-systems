#include "../../include/middleware/Middleware.h"
#include "../../../common/include/utils/Logger.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <sstream>

using json = nlohmann::json;

namespace sdrs::gateway
{

// ============================================================================
// LoggingMiddleware
// ============================================================================

bool LoggingMiddleware::process(const httplib::Request& req, httplib::Response& res, Handler next)
{
    auto startTime = std::chrono::steady_clock::now();
    
    std::string logMsg = "[Gateway] " + std::string(req.method) + " " + req.path;
    if (!req.get_header_value("X-Forwarded-For").empty())
    {
        logMsg += " from " + req.get_header_value("X-Forwarded-For");
    }
    
    sdrs::utils::Logger::Info(logMsg);
    
    // Call next middleware or handler
    next(req, res);
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    sdrs::utils::Logger::Info("[Gateway] Response " + std::to_string(res.status) + 
                              " in " + std::to_string(duration.count()) + "ms");
    
    return true;
}

// ============================================================================
// AuthenticationMiddleware
// ============================================================================

AuthenticationMiddleware::AuthenticationMiddleware(bool requireAuth)
    : _requireAuth(requireAuth)
{
}

bool AuthenticationMiddleware::process(const httplib::Request& req, httplib::Response& res, Handler next)
{
    // Skip auth for health check and root endpoints
    if (req.path == "/health" || req.path == "/" || req.path == "/api")
    {
        next(req, res);
        return true;
    }
    
    if (!_requireAuth)
    {
        next(req, res);
        return true;
    }
    
    // Check for API Key in header
    std::string apiKey = req.get_header_value("X-API-Key");
    if (!apiKey.empty() && validateApiKey(apiKey))
    {
        next(req, res);
        return true;
    }
    
    // Check for JWT token in Authorization header
    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.starts_with("Bearer "))
    {
        std::string token = authHeader.substr(7);
        if (validateJWT(token))
        {
            next(req, res);
            return true;
        }
    }
    
    // Authentication failed
    json error = {
        {"success", false},
        {"message", "Authentication required. Provide X-API-Key header or Bearer token."},
        {"status_code", 401}
    };
    
    res.status = 401;
    res.set_content(error.dump(), "application/json");
    
    sdrs::utils::Logger::Warn("[Gateway] Authentication failed for " + req.path);
    return false;
}

bool AuthenticationMiddleware::validateApiKey(const std::string& apiKey) const
{
    // Simple API key validation - in production, check against database
    // For demo: accept "demo-api-key-12345"
    return apiKey == "demo-api-key-12345" || apiKey == "test-key";
}

bool AuthenticationMiddleware::validateJWT(const std::string& token) const
{
    // JWT validation - in production, verify signature and claims
    // For demo: accept any non-empty token starting with "eyJ"
    return !token.empty() && token.starts_with("eyJ");
}

// ============================================================================
// RateLimitMiddleware
// ============================================================================

RateLimitMiddleware::RateLimitMiddleware(int maxRequestsPerMinute)
    : _maxRequestsPerMinute(maxRequestsPerMinute)
{
}

bool RateLimitMiddleware::process(const httplib::Request& req, httplib::Response& res, Handler next)
{
    std::string clientId = getClientIdentifier(req);
    
    if (isRateLimited(clientId))
    {
        json error = {
            {"success", false},
            {"message", "Rate limit exceeded. Maximum " + std::to_string(_maxRequestsPerMinute) + " requests per minute."},
            {"status_code", 429}
        };
        
        res.status = 429;
        res.set_content(error.dump(), "application/json");
        res.set_header("Retry-After", "60");
        
        sdrs::utils::Logger::Warn("[Gateway] Rate limit exceeded for client: " + clientId);
        return false;
    }
    
    next(req, res);
    return true;
}

std::string RateLimitMiddleware::getClientIdentifier(const httplib::Request& req) const
{
    // Try to get client IP from X-Forwarded-For header first
    std::string forwardedFor = req.get_header_value("X-Forwarded-For");
    if (!forwardedFor.empty())
    {
        // Take first IP if multiple proxies
        size_t commaPos = forwardedFor.find(',');
        if (commaPos != std::string::npos)
        {
            return forwardedFor.substr(0, commaPos);
        }
        return forwardedFor;
    }
    
    // Fallback to API key if available
    std::string apiKey = req.get_header_value("X-API-Key");
    if (!apiKey.empty())
    {
        return "apikey:" + apiKey;
    }
    
    // Default identifier
    return "unknown";
}

bool RateLimitMiddleware::isRateLimited(const std::string& clientId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto now = std::chrono::steady_clock::now();
    auto& stats = _clientStats[clientId];
    
    // Reset window if 1 minute has passed
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - stats.windowStart);
    if (elapsed.count() >= 60)
    {
        stats.requestCount = 0;
        stats.windowStart = now;
    }
    
    stats.requestCount++;
    
    return stats.requestCount > _maxRequestsPerMinute;
}

// ============================================================================
// CORSMiddleware
// ============================================================================

bool CORSMiddleware::process(const httplib::Request& req, httplib::Response& res, Handler next)
{
    // Add CORS headers
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-API-Key");
    res.set_header("Access-Control-Max-Age", "86400");
    
    // Handle preflight OPTIONS requests
    if (req.method == "OPTIONS")
    {
        res.status = 204;
        return true;
    }
    
    next(req, res);
    return true;
}

// ============================================================================
// MiddlewareChain
// ============================================================================

void MiddlewareChain::add(std::shared_ptr<Middleware> middleware)
{
    _middlewares.push_back(middleware);
}

void MiddlewareChain::execute(const httplib::Request& req, httplib::Response& res, 
                              Middleware::Handler finalHandler)
{
    if (_middlewares.empty())
    {
        finalHandler(req, res);
        return;
    }
    
    // Create chain of handlers
    size_t index = 0;
    std::function<void(const httplib::Request&, httplib::Response&)> chain;
    
    chain = [this, &index, &chain, finalHandler](const httplib::Request& req, httplib::Response& res) {
        if (index < _middlewares.size())
        {
            auto currentIndex = index++;
            _middlewares[currentIndex]->process(req, res, chain);
        }
        else
        {
            finalHandler(req, res);
        }
    };
    
    chain(req, res);
}

} // namespace sdrs::gateway
