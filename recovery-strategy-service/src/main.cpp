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
    
    // GET /list - Get available recovery strategies
    server.Get("/list", [](const httplib::Request&, httplib::Response& res) {
        try {
            json strategies = {
                {"success", true},
                {"message", "Retrieved available strategies"},
                {"status_code", 200},
                {"data", {
                    {"strategies", {
                        {
                            {"id", "automated_reminder"},
                            {"name", "Automated Reminder"},
                            {"description", "Send automated email and SMS reminders"},
                            {"risk_level", "LOW_TO_MEDIUM"},
                            {"estimated_duration", "3-7 days"}
                        },
                        {
                            {"id", "settlement_offer"},
                            {"name", "Settlement Offer"},
                            {"description", "Propose reduced payment plan"},
                            {"risk_level", "MEDIUM"},
                            {"estimated_duration", "7-14 days"}
                        },
                        {
                            {"id", "legal_action"},
                            {"name", "Legal Action"},
                            {"description", "Initiate legal proceedings"},
                            {"risk_level", "HIGH"},
                            {"estimated_duration", "30+ days"}
                        }
                    }}
                }}
            };
            res.set_content(strategies.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = sdrs::models::Response<void>::error(std::string("Failed to list strategies: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // POST /execute-strategy - Execute a recovery strategy
    server.Post("/execute-strategy", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            
            // Support both "strategy_id" and "strategy_type" for flexibility
            std::string strategyId;
            if (j.contains("strategy_id") && !j["strategy_id"].is_null()) {
                strategyId = j["strategy_id"].get<std::string>();
            } else if (j.contains("strategy_type") && !j["strategy_type"].is_null()) {
                strategyId = j["strategy_type"].get<std::string>();
            } else {
                throw std::runtime_error("Missing required field: strategy_id or strategy_type");
            }
            
            int accountId = j.value("account_id", 0);
            int borrowerId = j.value("borrower_id", 0);
            double expectedAmount = j.value("expected_amount", 0.0);
            
            // Execute appropriate strategy based on ID
            std::string statusStr = "Completed";
            std::string message = "Strategy executed";
            std::string communicationChannel = "";
            std::string communicationSubject = "";
            std::string communicationMessage = "";
            
            if (strategyId == "automated_reminder") {
                message = "Automated reminders sent via email and SMS";
                statusStr = "Completed";
                communicationChannel = "email";
                communicationSubject = "Payment Reminder - Account #" + std::to_string(accountId);
                communicationMessage = "Dear Customer, this is a reminder that your payment of " + 
                    std::to_string(expectedAmount) + " VND is overdue. Please make payment at your earliest convenience.";
            }
            else if (strategyId == "settlement_offer") {
                message = "Settlement offer proposed with reduced payment plan";
                statusStr = "Pending Acceptance";
                communicationChannel = "email";
                communicationSubject = "Settlement Offer - Account #" + std::to_string(accountId);
                communicationMessage = "Dear Customer, we are pleased to offer you a settlement plan for your outstanding balance of " +
                    std::to_string(expectedAmount) + " VND. Please contact us to discuss payment options.";
            }
            else if (strategyId == "legal_action") {
                message = "Legal proceedings initiated";
                statusStr = "In Progress";
                communicationChannel = "email";
                communicationSubject = "Legal Notice - Account #" + std::to_string(accountId);
                communicationMessage = "LEGAL NOTICE: This is to inform you that legal proceedings have been initiated for your outstanding debt of " +
                    std::to_string(expectedAmount) + " VND. Please contact our legal department immediately.";
            }
            else {
                throw std::runtime_error("Unknown strategy: " + strategyId);
            }
            
            // Call communication service to log the strategy communication
            bool communicationSent = false;
            try {
                httplib::Client cli("sdrs-communication", 8084);
                cli.set_connection_timeout(5);
                cli.set_read_timeout(5);
                
                json commPayload = {
                    {"borrower_id", borrowerId},
                    {"account_id", accountId},
                    {"strategy_id", strategyId},
                    {"channel", communicationChannel},
                    {"subject", communicationSubject},
                    {"message", communicationMessage}
                };
                
                auto commRes = cli.Post("/log-strategy-communication", commPayload.dump(), "application/json");
                if (commRes && commRes->status == 200) {
                    communicationSent = true;
                }
            } catch (...) {
                // Communication service call failed, continue anyway
            }
            
            json response = {
                {"success", true},
                {"message", message},
                {"status_code", 200},
                {"data", {
                    {"strategy_id", strategyId},
                    {"account_id", accountId},
                    {"borrower_id", borrowerId},
                    {"expected_amount", expectedAmount},
                    {"status", statusStr},
                    {"communication_sent", communicationSent},
                    {"communication_channel", communicationChannel},
                    {"communication_subject", communicationSubject}
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
