#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Logger.h"
#include "../../common/include/utils/Constants.h"

using json = nlohmann::json;

int main() {
    std::cout << "Starting Borrower Service..." << std::endl;
    
    httplib::Server server;
    
    // Health check endpoint
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "healthy"},
            {"service", "borrower-service"}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // Root endpoint
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Borrower Service API", "text/plain");
    });
    
    const int port = sdrs::constants::ports::BORROWER_SERVICE_PORT;
    std::cout << "Borrower Service listening on port " << port << std::endl;
    server.listen("0.0.0.0", port);
    
    return 0;
}
