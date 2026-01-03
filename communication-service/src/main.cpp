#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <vector>
#include <mutex>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Logger.h"
#include "../../common/include/utils/Constants.h"
#include "../../common/include/models/Response.h"
#include "../include/managers/CommunicationManager.h"
#include "../include/channels/EmailChannel.h"
#include "../include/channels/SMSChannel.h"
#include "../include/channels/VoiceCallChannel.h"

using json = nlohmann::json;
using namespace sdrs::communication;

// In-memory storage for strategy communications (would be database in production)
std::vector<json> strategyCommLogs;
std::mutex commLogsMutex;

int main()
{
    std::cout << "Starting Communication Service..." << std::endl;
    
    httplib::Server server;
    CommunicationManager manager;
    
    // Register channels
    manager.registerChannel(std::make_shared<EmailChannel>());
    manager.registerChannel(std::make_shared<SMSChannel>());
    manager.registerChannel(std::make_shared<VoiceCallChannel>());
    
    // Health check endpoint
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "healthy"},
            {"service", "communication-service"}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // GET /history/:borrower_id - Get communication history
    server.Get(R"(/history/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        try {
            int borrowerId = std::stoi(req.matches[1]);
            
            // Mock data: Only borrowers 1-6 have communication history
            json communications = json::array();
            
            if (borrowerId >= 1 && borrowerId <= 6) {
                // Generate unique communication IDs based on borrower_id
                int baseId = borrowerId * 1000;
                
                // Add email communication
                communications.push_back({
                    {"communication_id", baseId + 1},
                    {"channel", "email"},
                    {"sent_at", "2026-01-02 10:00:00"},
                    {"status", "delivered"},
                    {"subject", "Payment Reminder for Borrower #" + std::to_string(borrowerId)},
                    {"note", "Mock data - DB integration pending"}
                });
                
                // Add SMS communication
                communications.push_back({
                    {"communication_id", baseId + 2},
                    {"channel", "sms"},
                    {"sent_at", "2026-01-02 10:01:00"},
                    {"status", "delivered"},
                    {"message", "Payment overdue reminder for account " + std::to_string(borrowerId)},
                    {"note", "Mock data - DB integration pending"}
                });
            }
            
            // Add strategy communications from in-memory storage
            {
                std::lock_guard<std::mutex> lock(commLogsMutex);
                for (const auto& log : strategyCommLogs) {
                    if (log.value("borrower_id", 0) == borrowerId) {
                        communications.push_back(log);
                    }
                }
            }
            
            json history = {
                {"success", true},
                {"message", communications.empty() ? 
                    "No communication history found" : 
                    "Retrieved communication history"},
                {"status_code", 200},
                {"data", {
                    {"borrower_id", borrowerId},
                    {"communications", communications}
                }}
            };
            res.set_content(history.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = sdrs::models::Response<void>::error(std::string("Failed to get communication history: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // POST /send-email - Send email notification
    server.Post("/send-email", [&manager](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string to = j["to"].get<std::string>();
            std::string subject = j["subject"].get<std::string>();
            std::string body = j["body"].get<std::string>();
            
            // Mock sending via manager
            manager.sendMessage(1, body);
            
            json response = {
                {"success", true},
                {"message", "Email sent successfully"},
                {"status_code", 200},
                {"data", {{"to", to}, {"subject", subject}}}
            };
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = sdrs::models::Response<void>::error(std::string("Email send failed: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // POST /send-sms - Send SMS notification
    server.Post("/send-sms", [&manager](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string phone = j["phone"].get<std::string>();
            std::string message = j["message"].get<std::string>();
            
            manager.sendMessage(2, message);
            
            json response = {
                {"success", true},
                {"message", "SMS sent successfully"},
                {"status_code", 200},
                {"data", {{"phone", phone}}}
            };
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = sdrs::models::Response<void>::error(std::string("SMS send failed: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // POST /send-voice-call - Initiate voice call
    server.Post("/send-voice-call", [&manager](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string phone = j["phone"].get<std::string>();
            std::string script = j["script"].get<std::string>();
            
            manager.sendMessage(3, script);
            
            json response = {
                {"success", true},
                {"message", "Voice call initiated successfully"},
                {"status_code", 200},
                {"data", {{"phone", phone}}}
            };
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = sdrs::models::Response<void>::error(std::string("Voice call failed: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // POST /log-strategy-communication - Log communication from strategy execution
    server.Post("/log-strategy-communication", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            int borrowerId = j.value("borrower_id", 0);
            int accountId = j.value("account_id", 0);
            std::string strategyId = j.value("strategy_id", "");
            std::string channel = j.value("channel", "email");
            std::string subject = j.value("subject", "");
            std::string message = j.value("message", "");
            
            // Get current timestamp
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            std::string timestamp = ss.str();
            
            int commId = static_cast<int>(now.time_since_epoch().count() % 100000);
            
            // Store in memory for history retrieval
            json commLog = {
                {"communication_id", commId},
                {"borrower_id", borrowerId},
                {"account_id", accountId},
                {"strategy_id", strategyId},
                {"channel", channel},
                {"subject", subject},
                {"message", message},
                {"status", "sent"},
                {"sent_at", timestamp},
                {"source", "strategy_execution"}
            };
            
            {
                std::lock_guard<std::mutex> lock(commLogsMutex);
                strategyCommLogs.push_back(commLog);
            }
            
            sdrs::utils::Logger::Info("Strategy Communication Logged - Borrower: " + 
                std::to_string(borrowerId) + ", Strategy: " + strategyId + ", Channel: " + channel);
            
            json response = {
                {"success", true},
                {"message", "Communication logged successfully"},
                {"status_code", 200},
                {"data", commLog}
            };
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = sdrs::models::Response<void>::error(std::string("Log failed: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    const int port = sdrs::constants::ports::COMMUNICATION_SERVICE_PORT;
    std::cout << "Communication Service listening on port " << port << std::endl;
    sdrs::utils::Logger::Info("Communication Service started on port " + std::to_string(port));
    
    server.listen("0.0.0.0", port);
    
    return 0;
}
