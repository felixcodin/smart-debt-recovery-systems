#include <iostream>
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
    
    const int port = sdrs::constants::ports::COMMUNICATION_SERVICE_PORT;
    std::cout << "Communication Service listening on port " << port << std::endl;
    sdrs::utils::Logger::Info("Communication Service started on port " + std::to_string(port));
    
    server.listen("0.0.0.0", port);
    
    return 0;
}
