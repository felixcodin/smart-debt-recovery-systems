#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Logger.h"
#include "../../common/include/utils/Constants.h"
#include "../../common/include/models/Response.h"
#include "../../common/include/database/DatabaseManager.h"
#include "../include/models/Borrower.h"
#include "../include/models/LoanAccount.h"
#include "../include/models/PaymentHistory.h"
#include "../include/repositories/BorrowerRepository.h"
#include "../include/repositories/LoanAccountRepository.h"
#include "../include/repositories/PaymentHistoryRepository.h"

using json = nlohmann::json;
using sdrs::models::Response;
using sdrs::borrower::Borrower;
using sdrs::borrower::BorrowerRepository;
using sdrs::borrower::LoanAccountRepository;
using sdrs::borrower::PaymentHistory;
using sdrs::borrower::PaymentHistoryRepository;

/**
 * @brief Check if database mode is enabled via environment variable
 * @return true if SDRS_USE_DATABASE is set to "1" or "true"
 */
bool isDatabaseEnabled() {
    const char* useDb = std::getenv("SDRS_USE_DATABASE");
    if (!useDb) return false;
    std::string value(useDb);
    return value == "1" || value == "true" || value == "TRUE";
}

int main() {
    std::cout << "Starting Borrower Service..." << std::endl;
    
    // Check if database mode is enabled
    bool useMock = !isDatabaseEnabled();
    
    if (!useMock) {
        sdrs::utils::Logger::Info("Database mode enabled - initializing connection pool...");
        try {
            // Initialize database connection
            auto& db = sdrs::database::DatabaseManager::getInstance();
            db.initializeFromEnv();
            sdrs::utils::Logger::Info("Database connection pool initialized successfully");
        }
        catch (const std::exception& e) {
            sdrs::utils::Logger::Error("Failed to initialize database: " + std::string(e.what()));
            sdrs::utils::Logger::Info("Falling back to mock mode");
            useMock = true;
        }
    } else {
        sdrs::utils::Logger::Info("Running in mock mode (set SDRS_USE_DATABASE=1 to enable database)");
    }
    
    httplib::Server server;
    BorrowerRepository borrowerRepo(useMock);
    LoanAccountRepository loanRepo(useMock);
    PaymentHistoryRepository paymentRepo(useMock);
    
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
    
    // Test endpoint
    server.Post("/test-segment", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("{\"message\":\"Test endpoint works!\"}", "application/json");
    });
    
    // POST /update-segment/:id - Update borrower's risk segment
    // Workaround for cpp-httplib route matching issue
    server.Post(R"(/update-segment/(\d+))", [&borrowerRepo](const httplib::Request& req, httplib::Response& res) {
        try {
            int id = std::stoi(req.matches[1]);
            auto j = json::parse(req.body);
            
            std::string segmentStr = j.value("risk_segment", "Unclassified");
            
            // Find borrower
            auto borrowerOpt = borrowerRepo.findById(id);
            if (!borrowerOpt.has_value()) {
                auto response = Response<void>::notFound("Borrower not found with ID: " + std::to_string(id));
                res.status = response.getStatusCode();
                res.set_content(response.toJson(), "application/json");
                return;
            }
            
            auto borrower = borrowerOpt.value();
            
            // Convert string to RiskSegment enum
            sdrs::borrower::RiskSegment segment = sdrs::borrower::RiskSegment::Unclassified;
            if (segmentStr == "Low") segment = sdrs::borrower::RiskSegment::Low;
            else if (segmentStr == "Medium") segment = sdrs::borrower::RiskSegment::Medium;
            else if (segmentStr == "High") segment = sdrs::borrower::RiskSegment::High;
            
            // Update segment
            borrower.assignSegment(segment);
            
            // Save to repository
            auto updated = borrowerRepo.update(borrower);
            
            auto response = Response<Borrower>::success(updated, "Risk segment updated successfully");
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to update segment: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // POST /borrowers - Create new borrower
    server.Post("/borrowers", [&borrowerRepo](const httplib::Request& req, httplib::Response& res) {
        try {
            // Log the incoming request body for debugging
            sdrs::utils::Logger::Info("Received POST /borrowers request with body: " + req.body);
            
            // Parse request body
            auto borrower = Borrower::fromJson(req.body);
            
            // Create in repository
            auto created = borrowerRepo.create(borrower);
            
            // Return success response
            auto response = Response<Borrower>::success(created, "Borrower created successfully", 201);
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
        catch (const std::exception& e) {
            sdrs::utils::Logger::Error("Failed to create borrower: " + std::string(e.what()));
            auto response = Response<Borrower>::badRequest(std::string("Failed to create borrower: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // GET /borrowers/:id - Get borrower by ID
    server.Get(R"(/borrowers/(\d+))", [&borrowerRepo](const httplib::Request& req, httplib::Response& res) {
        try {
            int id = std::stoi(req.matches[1]);
            
            auto borrower = borrowerRepo.findById(id);
            
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
    server.Put(R"(/borrowers/(\d+))", [&borrowerRepo](const httplib::Request& req, httplib::Response& res) {
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
            auto updated = borrowerRepo.update(borrower);
            
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
    server.Delete(R"(/borrowers/(\d+))", [&borrowerRepo](const httplib::Request& req, httplib::Response& res) {
        try {
            int id = std::stoi(req.matches[1]);
            
            bool deleted = borrowerRepo.deleteById(id);
            
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
    server.Get("/borrowers", [&borrowerRepo](const httplib::Request&, httplib::Response& res) {
        try {
            auto borrowers = borrowerRepo.findAll();
            
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
    server.Post(R"(/loans/(\d+)/payment)", [&loanRepo, &paymentRepo](const httplib::Request& req, httplib::Response& res) {
        try {
            int accountId = std::stoi(req.matches[1]);
            auto j = json::parse(req.body);
            
            double amount = j["amount"].get<double>();
            
            // Find existing loan account
            auto accountOpt = loanRepo.findById(accountId);
            
            if (!accountOpt.has_value()) {
                auto response = Response<void>::notFound("Loan account not found with ID: " + std::to_string(accountId));
                res.status = response.getStatusCode();
                res.set_content(response.toJson(), "application/json");
                return;
            }
            
            // Record payment in loan account
            auto account = accountOpt.value();
            account.recordPayment(sdrs::money::Money(amount));
            loanRepo.update(account);
            
            // Also record in payment history
            auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
            sdrs::borrower::PaymentHistory payment(0, accountId, sdrs::money::Money(amount),
                sdrs::constants::PaymentMethod::BankTransfer, today);
            payment.markCompleted();
            paymentRepo.create(payment);
            
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
    server.Post(R"(/loans/(\d+)/missed-payment)", [&loanRepo](const httplib::Request& req, httplib::Response& res) {
        try {
            int accountId = std::stoi(req.matches[1]);
            
            // Find existing loan account
            auto accountOpt = loanRepo.findById(accountId);
            
            if (!accountOpt.has_value()) {
                auto response = Response<void>::notFound("Loan account not found with ID: " + std::to_string(accountId));
                res.status = response.getStatusCode();
                res.set_content(response.toJson(), "application/json");
                return;
            }
            
            auto account = accountOpt.value();
            account.markPaymentMissed();
            loanRepo.update(account);
            
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
    
    // GET /loans - Get all loan accounts
    server.Get("/loans", [&loanRepo](const httplib::Request&, httplib::Response& res) {
        try {
            auto accounts = loanRepo.findAll();
            
            json j = json::array();
            for (const auto& account : accounts) {
                j.push_back(json::parse(account.toJson()));
            }
            
            json response = {
                {"success", true},
                {"message", "Loan accounts retrieved successfully"},
                {"status_code", 200},
                {"data", j}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to retrieve loans: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // POST /loans - Create new loan account
    server.Post("/loans", [&loanRepo, &borrowerRepo](const httplib::Request& req, httplib::Response& res) {
        try {
            sdrs::utils::Logger::Info("[API] POST /loans - Body: " + req.body);
            
            auto j = json::parse(req.body);
            
            // Validate borrower exists
            int borrowerId = j["borrower_id"].get<int>();
            auto borrowerOpt = borrowerRepo.findById(borrowerId);
            
            if (!borrowerOpt.has_value()) {
                auto response = Response<void>::badRequest("Borrower not found with ID: " + std::to_string(borrowerId));
                res.status = response.getStatusCode();
                res.set_content(response.toJson(), "application/json");
                return;
            }
            
            // Parse loan account from JSON
            auto loanAccount = sdrs::borrower::LoanAccount::fromJson(req.body);
            
            // Create in repository
            auto created = loanRepo.create(loanAccount);
            
            json response = {
                {"success", true},
                {"message", "Loan account created successfully"},
                {"status_code", 201},
                {"data", json::parse(created.toJson())}
            };
            
            res.status = 201;
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            sdrs::utils::Logger::Error("[API] POST /loans error: " + std::string(e.what()));
            auto response = Response<void>::error(std::string("Failed to create loan account: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // GET /loans/accounts - Alias for /loans
    server.Get("/loans/accounts", [&loanRepo](const httplib::Request&, httplib::Response& res) {
        try {
            auto accounts = loanRepo.findAll();
            
            json j = json::array();
            for (const auto& account : accounts) {
                j.push_back(json::parse(account.toJson()));
            }
            
            json response = {
                {"success", true},
                {"message", "Loan accounts retrieved successfully"},
                {"status_code", 200},
                {"data", j}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to retrieve loans: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // GET /loans/delinquent - Get delinquent loan accounts
    server.Get("/loans/delinquent", [&loanRepo](const httplib::Request& req, httplib::Response& res) {
        try {
            int minDaysPastDue = 1;
            if (req.has_param("min_days")) {
                minDaysPastDue = std::stoi(req.get_param_value("min_days"));
            }
            
            auto accounts = loanRepo.findDelinquent(minDaysPastDue);
            
            json j = json::array();
            for (const auto& account : accounts) {
                j.push_back(json::parse(account.toJson()));
            }
            
            json response = {
                {"success", true},
                {"message", "Delinquent accounts retrieved successfully"},
                {"status_code", 200},
                {"count", accounts.size()},
                {"data", j}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to retrieve delinquent loans: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // GET /payments - Get all payments
    server.Get("/payments", [&paymentRepo](const httplib::Request&, httplib::Response& res) {
        try {
            auto payments = paymentRepo.findAll();
            
            json j = json::array();
            for (const auto& payment : payments) {
                j.push_back(json::parse(payment.toJson()));
            }
            
            json response = {
                {"success", true},
                {"message", "Payments retrieved successfully"},
                {"status_code", 200},
                {"data", j}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to retrieve payments: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // GET /payments/late - Get late payments
    server.Get("/payments/late", [&paymentRepo](const httplib::Request&, httplib::Response& res) {
        try {
            auto payments = paymentRepo.findLatePayments();
            
            json j = json::array();
            for (const auto& payment : payments) {
                j.push_back(json::parse(payment.toJson()));
            }
            
            json response = {
                {"success", true},
                {"message", "Late payments retrieved successfully"},
                {"status_code", 200},
                {"count", payments.size()},
                {"data", j}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to retrieve late payments: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // GET /payments/borrower/:id - Get all payments for a borrower's loan accounts
    server.Get(R"(/payments/borrower/(\d+))", [&paymentRepo, &loanRepo](const httplib::Request& req, httplib::Response& res) {
        try {
            int borrowerId = std::stoi(req.matches[1]);
            
            // Get all loan accounts for this borrower
            auto loans = loanRepo.findByBorrowerId(borrowerId);
            
            // Collect payments from all loan accounts
            std::vector<PaymentHistory> allPayments;
            for (const auto& loan : loans) {
                auto payments = paymentRepo.findByAccountId(loan.getAccountId());
                allPayments.insert(allPayments.end(), payments.begin(), payments.end());
            }
            
            // Convert to JSON
            json j = json::array();
            for (const auto& payment : allPayments) {
                j.push_back(json::parse(payment.toJson()));
            }
            
            json response = {
                {"success", true},
                {"message", "Borrower payments retrieved successfully"},
                {"status_code", 200},
                {"borrower_id", borrowerId},
                {"count", allPayments.size()},
                {"data", j}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = Response<void>::error(std::string("Failed to retrieve borrower payments: ") + e.what());
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
