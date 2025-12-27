// API Gateway - Central entry point for all client requests

#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Constants.h"

using json = nlohmann::json;

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
        res.set_content("Smart Debt Recovery System API Gateway", "text/plain");
    });
    
    const int port = sdrs::constants::ports::API_GATEWAY_PORT;
    std::cout << "API Gateway listening on port " << port << std::endl;
    server.listen("0.0.0.0", port);
    
    return 0;
}
