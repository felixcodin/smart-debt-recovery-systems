#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Logger.h"
#include "../../common/include/utils/Constants.h"
#include "../../common/include/models/Response.h"
#include "../include/models/Borrower.h"
#include "../include/models/LoanAccount.h"
#include "../include/repositories/BorrowerRepository.h"

using json = nlohmann::json;
using sdrs::models::Response;
using sdrs::borrower::Borrower;
using sdrs::borrower::BorrowerRepository;

int main() {
    std::cout << "Starting Borrower Service..." << std::endl;
    
    httplib::Server server;
    BorrowerRepository repository;
    
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
    
    // POST /borrowers - Create new borrower
    server.Post("/borrowers", [&repository](const httplib::Request& req, httplib::Response& res) {
        try {
            // Parse request body
            auto borrower = Borrower::fromJson(req.body);
            
            // Create in repository
            auto created = repository.create(borrower);
            
            // Return success response
            auto response = Response<Borrower>::success(created, "Borrower created successfully", 201);
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<Borrower>::badRequest(std::string("Failed to create borrower: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // GET /borrowers/:id - Get borrower by ID
    server.Get(R"(/borrowers/(\d+))", [&repository](const httplib::Request& req, httplib::Response& res) {
        try {
            int id = std::stoi(req.matches[1]);
            
            auto borrower = repository.findById(id);
            
            if (borrower.has_value()) {
                auto response = Response<Borrower>::success(borrower.value(), "Borrower found");
                res.status = response.getStatusCode();
                res.set_content(response.toJson(), "application/json");
            }
            else {
                auto response = Response<Borrower>::notFound("Borrower not found with ID: " + std::to_string(id));
                res.status = response.getStatusCode();
                res.set_content(response.toJson(), "application/json");
            }
        }
        catch (const std::exception& e) {
            auto response = Response<Borrower>::error(std::string("Failed to retrieve borrower: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // PUT /borrowers/:id - Update borrower
    server.Put(R"(/borrowers/(\d+))", [&repository](const httplib::Request& req, httplib::Response& res) {
        try {
            int id = std::stoi(req.matches[1]);
            
            // Parse request body
            auto borrower = Borrower::fromJson(req.body);
            
            // Verify ID matches
            if (borrower.getId() != id && borrower.getId() != 0) {
                auto response = Response<Borrower>::badRequest("Borrower ID mismatch");
                res.status = response.getStatusCode();
                res.set_content(response.toJson(), "application/json");
                return;
            }
            
            // Update in repository
            auto updated = repository.update(borrower);
            
            auto response = Response<Borrower>::success(updated, "Borrower updated successfully");
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<Borrower>::error(std::string("Failed to update borrower: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // DELETE /borrowers/:id - Delete borrower
    server.Delete(R"(/borrowers/(\d+))", [&repository](const httplib::Request& req, httplib::Response& res) {
        try {
            int id = std::stoi(req.matches[1]);
            
            bool deleted = repository.deleteById(id);
            
            if (deleted) {
                auto response = Response<void>::success("Borrower deleted successfully");
                res.status = response.getStatusCode();
                res.set_content(response.toJson(), "application/json");
            }
            else {
                auto response = Response<void>::notFound("Borrower not found with ID: " + std::to_string(id));
                res.status = response.getStatusCode();
                res.set_content(response.toJson(), "application/json");
            }
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to delete borrower: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // GET /borrowers - Get all borrowers
    server.Get("/borrowers", [&repository](const httplib::Request&, httplib::Response& res) {
        try {
            auto borrowers = repository.findAll();
            
            // Build JSON array
            json j = json::array();
            for (const auto& borrower : borrowers) {
                j.push_back(json::parse(borrower.toJson()));
            }
            
            json response = {
                {"success", true},
                {"message", "Borrowers retrieved successfully"},
                {"status_code", 200},
                {"data", j}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to retrieve borrowers: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // POST /loans/:accountId/payment - Record payment for a loan account
    server.Post(R"(/loans/(\d+)/payment)", [](const httplib::Request& req, httplib::Response& res) {
        try {
            int accountId = std::stoi(req.matches[1]);
            auto j = json::parse(req.body);
            
            double amount = j["amount"].get<double>();
            
            // Mock: Create a loan account and record payment
            // In real implementation, would fetch from repository
            sdrs::borrower::LoanAccount account(accountId, 1, 10000.0, 0.05, 12);
            account.recordPayment(sdrs::money::Money(amount));
            
            json response = {
                {"success", true},
                {"message", "Payment recorded successfully"},
                {"status_code", 200},
                {"data", {
                    {"account_id", accountId},
                    {"payment_amount", amount},
                    {"remaining_balance", account.getRemainingAmount().getAmount()}
                }}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to record payment: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // POST /loans/:accountId/missed-payment - Mark payment as missed
    server.Post(R"(/loans/(\d+)/missed-payment)", [](const httplib::Request& req, httplib::Response& res) {
        try {
            int accountId = std::stoi(req.matches[1]);
            
            // Mock: Create a loan account and mark payment missed
            sdrs::borrower::LoanAccount account(accountId, 1, 10000.0, 0.05, 12);
            account.markPaymentMissed();
            
            json response = {
                {"success", true},
                {"message", "Payment marked as missed"},
                {"status_code", 200},
                {"data", {
                    {"account_id", accountId},
                    {"days_past_due", account.getDaysPastDue()},
                    {"missed_payments", account.getMissedPayments()},
                    {"late_fees", account.getLateFees().getAmount()},
                    {"status", sdrs::borrower::LoanAccount::statusToString(account.getStatus())}
                }}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to mark missed payment: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // POST /borrowers/:id/assess-risk - Assess risk for a borrower (calls risk-assessment-service)
    server.Post(R"(/borrowers/(\d+)/assess-risk)", [](const httplib::Request& req, httplib::Response& res) {
        try {
            int borrowerId = std::stoi(req.matches[1]);
            auto j = json::parse(req.body);
            
            // Prepare request for risk-assessment-service
            json riskRequest = {
                {"account_id", j["account_id"].get<int>()},
                {"borrower_id", borrowerId},
                {"days_past_due", j["days_past_due"].get<int>()},
                {"missed_payments", j["missed_payments"].get<int>()},
                {"loan_amount", j["loan_amount"].get<double>()},
                {"remaining_amount", j["remaining_amount"].get<double>()},
                {"interest_rate", j["interest_rate"].get<double>()},
                {"monthly_income", j["monthly_income"].get<double>()},
                {"account_age_months", j["account_age_months"].get<int>()}
            };
            
            // Call risk-assessment-service
            httplib::Client client("localhost", sdrs::constants::ports::RISK_SERVICE_PORT);
            auto riskRes = client.Post("/assess-risk", riskRequest.dump(), "application/json");
            
            if (riskRes && riskRes->status == 200) {
                auto riskJson = json::parse(riskRes->body);
                
                json response = {
                    {"success", true},
                    {"message", "Risk assessment completed"},
                    {"status_code", 200},
                    {"data", riskJson["data"]}
                };
                
                res.set_content(response.dump(), "application/json");
            } else {
                auto response = Response<void>::error("Failed to assess risk: Risk service unavailable");
                res.status = 503;
                res.set_content(response.toJson(), "application/json");
            }
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Risk assessment failed: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    const int port = sdrs::constants::ports::BORROWER_SERVICE_PORT;
    std::cout << "Borrower Service listening on port " << port << std::endl;
    sdrs::utils::Logger::Info("Borrower Service started on port " + std::to_string(port));
    
    server.listen("0.0.0.0", port);
    
    return 0;
}
