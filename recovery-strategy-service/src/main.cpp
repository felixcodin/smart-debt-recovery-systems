#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Logger.h"
#include "../../common/include/utils/Constants.h"
#include "../../common/include/models/Response.h"
#include "../include/strategies/AutomatedReminderStrategy.h"
#include "../include/strategies/SettlementOfferStrategy.h"
#include "../include/strategies/LegalActionStrategy.h"

using json = nlohmann::json;

int main() {
    std::cout << "Starting Recovery Strategy Service..." << std::endl;
    
    httplib::Server server;
    
    // Health check endpoint
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "healthy"},
            {"service", "recovery-strategy-service"}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // POST /execute-strategy - Execute a recovery strategy
    server.Post("/execute-strategy", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            
            std::string strategyType = j["strategy_type"].get<std::string>();
            int accountId = j["account_id"].get<int>();
            
            // Mock strategy execution
            json response = {
                {"success", true},
                {"message", "Strategy executed successfully"},
                {"status_code", 200},
                {"data", {
                    {"strategy_type", strategyType},
                    {"account_id", accountId},
                    {"status", "Completed"}
                }}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = sdrs::models::Response<void>::error(std::string("Strategy execution failed: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    const int port = sdrs::constants::ports::RECOVERY_SERVICE_PORT;
    std::cout << "Recovery Strategy Service listening on port " << port << std::endl;
    sdrs::utils::Logger::Info("Recovery Strategy Service started on port " + std::to_string(port));
    
    server.listen("0.0.0.0", port);
    
    return 0;
}
