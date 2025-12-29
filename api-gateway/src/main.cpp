// API Gateway - Central entry point for all client requests

#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Constants.h"
#include "../../common/include/utils/Logger.h"

using json = nlohmann::json;

// Helper function to forward requests
void forwardRequest(const httplib::Request& req, httplib::Response& res, 
                   const std::string& targetHost, int targetPort, const std::string& path) {
    try {
        httplib::Client client(targetHost.c_str(), targetPort);
        client.set_connection_timeout(5, 0); // 5 seconds
        
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
        } else {
            json error = {
                {"success", false},
                {"message", "Service unavailable"},
                {"status_code", 503}
            };
            res.status = 503;
            res.set_content(error.dump(), "application/json");
        }
    } catch (const std::exception& e) {
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
    std::cout << "Starting API Gateway..." << std::endl;
    
    httplib::Server server;
    
    // Health check endpoint
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "healthy"},
            {"service", "api-gateway"}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // Root endpoint
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        json info = {
            {"service", "Smart Debt Recovery System API Gateway"},
            {"version", "1.0.0"},
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
    
    // Route /api/borrowers/* to borrower-service
    server.Get(R"(/api/borrowers(.*))", [](const httplib::Request& req, httplib::Response& res) {
        std::string path = "/borrowers" + std::string(req.matches[1]);
        forwardRequest(req, res, "localhost", sdrs::constants::ports::BORROWER_SERVICE_PORT, path);
    });
    
    server.Post(R"(/api/borrowers(.*))", [](const httplib::Request& req, httplib::Response& res) {
        std::string path = "/borrowers" + std::string(req.matches[1]);
        forwardRequest(req, res, "localhost", sdrs::constants::ports::BORROWER_SERVICE_PORT, path);
    });
    
    server.Put(R"(/api/borrowers(.*))", [](const httplib::Request& req, httplib::Response& res) {
        std::string path = "/borrowers" + std::string(req.matches[1]);
        forwardRequest(req, res, "localhost", sdrs::constants::ports::BORROWER_SERVICE_PORT, path);
    });
    
    server.Delete(R"(/api/borrowers(.*))", [](const httplib::Request& req, httplib::Response& res) {
        std::string path = "/borrowers" + std::string(req.matches[1]);
        forwardRequest(req, res, "localhost", sdrs::constants::ports::BORROWER_SERVICE_PORT, path);
    });
    
    // Route /api/loans/* to borrower-service
    server.Post(R"(/api/loans(.*))", [](const httplib::Request& req, httplib::Response& res) {
        std::string path = "/loans" + std::string(req.matches[1]);
        forwardRequest(req, res, "localhost", sdrs::constants::ports::BORROWER_SERVICE_PORT, path);
    });
    
    // Route /api/risk/* to risk-assessment-service
    server.Post("/api/risk/assess", [](const httplib::Request& req, httplib::Response& res) {
        forwardRequest(req, res, "localhost", sdrs::constants::ports::RISK_SERVICE_PORT, "/assess-risk");
    });
    
    // Route /api/strategy/* to recovery-strategy-service
    server.Post("/api/strategy/execute", [](const httplib::Request& req, httplib::Response& res) {
        forwardRequest(req, res, "localhost", sdrs::constants::ports::RECOVERY_SERVICE_PORT, "/execute-strategy");
    });
    
    // Route /api/communication/* to communication-service
    server.Post("/api/communication/email", [](const httplib::Request& req, httplib::Response& res) {
        forwardRequest(req, res, "localhost", sdrs::constants::ports::COMMUNICATION_SERVICE_PORT, "/send-email");
    });
    
    server.Post("/api/communication/sms", [](const httplib::Request& req, httplib::Response& res) {
        forwardRequest(req, res, "localhost", sdrs::constants::ports::COMMUNICATION_SERVICE_PORT, "/send-sms");
    });
    
    server.Post("/api/communication/voice", [](const httplib::Request& req, httplib::Response& res) {
        forwardRequest(req, res, "localhost", sdrs::constants::ports::COMMUNICATION_SERVICE_PORT, "/send-voice-call");
    });
    
    const int port = sdrs::constants::ports::API_GATEWAY_PORT;
    std::cout << "API Gateway listening on port " << port << std::endl;
    server.listen("0.0.0.0", port);
    
    return 0;
}
