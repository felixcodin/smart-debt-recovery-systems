// API Gateway - Central entry point with middleware support

#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Constants.h"
#include "../../common/include/utils/Logger.h"
#include "../include/middleware/Middleware.h"
#include "../include/registry/ServiceRegistry.h"

using json = nlohmann::json;
using namespace sdrs::gateway;

// Global service registry and middleware chain
ServiceRegistry g_serviceRegistry;
MiddlewareChain g_middlewareChain;

// Helper function to forward requests with service registry
void forwardRequest(const httplib::Request& req, httplib::Response& res, 
                   const std::string& serviceName, const std::string& path) {
    try {
        // Check service health
        if (!g_serviceRegistry.isServiceHealthy(serviceName)) {
            json error = {
                {"success", false},
                {"message", "Service '" + serviceName + "' is currently unavailable"},
                {"status_code", 503}
            };
            res.status = 503;
            res.set_content(error.dump(), "application/json");
            return;
        }
        
        ServiceInfo service = g_serviceRegistry.getService(serviceName);
        httplib::Client client(service.host.c_str(), service.port);
        client.set_connection_timeout(5, 0);
        
        httplib::Result result;
        
        if (req.method == "GET") {
            result = client.Get(path.c_str());
        } else if (req.method == "POST") {
            result = client.Post(path.c_str(), req.body, "application/json");
        } else if (req.method == "PUT") {
            result = client.Put(path.c_str(), req.body, "application/json");
        } else if (req.method == "DELETE") {
            result = client.Delete(path.c_str());
        }
        
        if (result) {
            res.status = result->status;
            res.set_content(result->body, "application/json");
            g_serviceRegistry.markServiceHealthy(serviceName);
        } else {
            g_serviceRegistry.markServiceUnhealthy(serviceName);
            json error = {
                {"success", false},
                {"message", "Service temporarily unavailable"},
                {"status_code", 503}
            };
            res.status = 503;
            res.set_content(error.dump(), "application/json");
        }
    } catch (const std::exception& e) {
        g_serviceRegistry.markServiceUnhealthy(serviceName);
        json error = {
            {"success", false},
            {"message", std::string("Gateway error: ") + e.what()},
            {"status_code", 500}
        };
        res.status = 500;
        res.set_content(error.dump(), "application/json");
    }
}

int main() {
    sdrs::utils::Logger::Info("Starting API Gateway with middleware support...");
    
    // Register backend services - use environment variables for host names (Docker-friendly)
    const char* borrowerHost = std::getenv("BORROWER_SERVICE_HOST");
    const char* riskHost = std::getenv("RISK_SERVICE_HOST");
    const char* recoveryHost = std::getenv("RECOVERY_SERVICE_HOST");
    const char* commHost = std::getenv("COMMUNICATION_SERVICE_HOST");
    
    g_serviceRegistry.registerService("borrower-service", 
        borrowerHost ? borrowerHost : "localhost", 
        sdrs::constants::ports::BORROWER_SERVICE_PORT);
    g_serviceRegistry.registerService("risk-assessment-service", 
        riskHost ? riskHost : "localhost", 
        sdrs::constants::ports::RISK_SERVICE_PORT);
    g_serviceRegistry.registerService("recovery-strategy-service", 
        recoveryHost ? recoveryHost : "localhost", 
        sdrs::constants::ports::RECOVERY_SERVICE_PORT);
    g_serviceRegistry.registerService("communication-service", 
        commHost ? commHost : "localhost", 
        sdrs::constants::ports::COMMUNICATION_SERVICE_PORT);
    
    // Setup middleware chain
    g_middlewareChain.add(std::make_shared<CORSMiddleware>());
    g_middlewareChain.add(std::make_shared<LoggingMiddleware>());
    g_middlewareChain.add(std::make_shared<RateLimitMiddleware>(100)); // 100 requests/minute
    g_middlewareChain.add(std::make_shared<AuthenticationMiddleware>(false)); // Auth disabled for demo
    
    httplib::Server server;
    
    // Add universal OPTIONS handler for CORS preflight
    server.Options(".*", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [](const httplib::Request&, httplib::Response& res) {
            // CORS headers already added by CORSMiddleware
            res.status = 204;
        });
    });
    
    // Health check endpoint
    server.Get("/health", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [](const httplib::Request&, httplib::Response& res) {
            auto services = g_serviceRegistry.getAllServices();
            json serviceStatus;
            
            for (const auto& [name, info] : services) {
                serviceStatus[name] = {
                    {"healthy", info.isHealthy},
                    {"host", info.host},
                    {"port", info.port},
                    {"failures", info.consecutiveFailures}
                };
            }
            
            json response = {
                {"status", "healthy"},
                {"service", "api-gateway"},
                {"services", serviceStatus},
                {"timestamp", std::time(nullptr)}
            };
            res.set_content(response.dump(2), "application/json");
        });
    });
    
    // Root endpoint
    server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [](const httplib::Request&, httplib::Response& res) {
            json info = {
                {"service", "Smart Debt Recovery System API Gateway"},
                {"version", "1.0.0"},
                {"features", {
                    "Request/Response Logging",
                    "Rate Limiting",
                    "Service Health Monitoring",
                    "CORS Support",
                    "Circuit Breaker Pattern"
                }},
                {"routes", {
                    {"borrowers", "/api/borrowers/*"},
                    {"loans", "/api/loans/*"},
                    {"risk", "/api/risk/*"},
                    {"strategy", "/api/strategy/*"},
                    {"communication", "/api/communication/*"}
                }}
            };
            res.set_content(info.dump(2), "application/json");
        });
    });
    
    // Route /api/borrowers/* to borrower-service
    server.Get(R"(/api/borrowers(.*))", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            std::string path = "/borrowers" + std::string(req.matches[1]);
            forwardRequest(req, res, "borrower-service", path);
        });
    });
    
    server.Post(R"(/api/borrowers(.*))", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            std::string path = "/borrowers" + std::string(req.matches[1]);
            forwardRequest(req, res, "borrower-service", path);
        });
    });
    
    server.Put(R"(/api/borrowers(.*))", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            std::string path = "/borrowers" + std::string(req.matches[1]);
            forwardRequest(req, res, "borrower-service", path);
        });
    });
    
    server.Delete(R"(/api/borrowers(.*))", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            std::string path = "/borrowers" + std::string(req.matches[1]);
            forwardRequest(req, res, "borrower-service", path);
        });
    });
    
    // Update borrower risk segment - workaround route
    server.Post(R"(/api/update-segment/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            std::string path = "/update-segment/" + std::string(req.matches[1]);
            forwardRequest(req, res, "borrower-service", path);
        });
    });
    
    // Route /api/loans/* to borrower-service
    server.Get(R"(/api/loans(.*))", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            std::string path = "/loans" + std::string(req.matches[1]);
            forwardRequest(req, res, "borrower-service", path);
        });
    });
    
    server.Post(R"(/api/loans(.*))", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            std::string path = "/loans" + std::string(req.matches[1]);
            forwardRequest(req, res, "borrower-service", path);
        });
    });
    
    // Route /api/payments/* to borrower-service
    server.Get(R"(/api/payments(.*))", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            std::string path = "/payments" + std::string(req.matches[1]);
            forwardRequest(req, res, "borrower-service", path);
        });
    });
    
    // Route /api/risk/* to risk-assessment-service
    server.Post("/api/risk/assess", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            forwardRequest(req, res, "risk-assessment-service", "/assess-risk");
        });
    });
    
    server.Post("/api/risk/cluster", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            forwardRequest(req, res, "risk-assessment-service", "/cluster/borrowers");
        });
    });
    
    server.Get("/api/risk/model/status", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            forwardRequest(req, res, "risk-assessment-service", "/model/status");
        });
    });
    
    // Route /api/strategy/* to recovery-strategy-service
    server.Get("/api/strategy/list", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            forwardRequest(req, res, "recovery-strategy-service", "/list");
        });
    });
    
    server.Post("/api/strategy/execute", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            forwardRequest(req, res, "recovery-strategy-service", "/execute-strategy");
        });
    });
    
    // Route /api/communication/* to communication-service
    server.Get(R"(/api/communication/history/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            std::string borrowerId = req.matches[1];
            forwardRequest(req, res, "communication-service", "/history/" + borrowerId);
        });
    });
    server.Post("/api/communication/email", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            forwardRequest(req, res, "communication-service", "/send-email");
        });
    });
    
    server.Post("/api/communication/sms", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            forwardRequest(req, res, "communication-service", "/send-sms");
        });
    });
    
    server.Post("/api/communication/voice", [](const httplib::Request& req, httplib::Response& res) {
        g_middlewareChain.execute(req, res, [&req](const httplib::Request&, httplib::Response& res) {
            forwardRequest(req, res, "communication-service", "/send-voice-call");
        });
    });
    
    const int port = sdrs::constants::ports::API_GATEWAY_PORT;
    sdrs::utils::Logger::Info("API Gateway listening on port " + std::to_string(port));
    std::cout << "\n=== API Gateway Ready ===" << std::endl;
    std::cout << "Features: Logging | Rate Limiting | CORS | Health Monitoring" << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "========================\n" << std::endl;
    server.listen("0.0.0.0", port);
    
    return 0;
}
