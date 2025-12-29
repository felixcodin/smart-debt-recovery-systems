#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Logger.h"
#include "../../common/include/utils/Constants.h"
#include "../../common/include/models/Response.h"
#include "../include/models/RiskScorer.h"

using json = nlohmann::json;
using namespace sdrs::risk;
using namespace sdrs::borrower;
using namespace sdrs::money;

int main() {
    std::cout << "Starting Risk Assessment Service..." << std::endl;
    
    httplib::Server server;
    RiskScorer scorer;
    
    // Health check endpoint
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "healthy"},
            {"service", "risk-assessment-service"}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // POST /assess-risk - Assess risk for a borrower
    server.Post("/assess-risk", [&scorer](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            
            RiskFeatures features;
            features.accountId = j["account_id"].get<int>();
            features.borrowerId = j["borrower_id"].get<int>();
            features.daysPastDue = j["days_past_due"].get<int>();
            features.numberOfMissedPayments = j["missed_payments"].get<int>();
            features.loanAmount = Money(j["loan_amount"].get<double>());
            features.remainingAmount = Money(j["remaining_amount"].get<double>());
            features.interestRate = j["interest_rate"].get<double>();
            features.monthlyIncome = Money(j["monthly_income"].get<double>());
            features.accountAgeMonths = j["account_age_months"].get<int>();
            
            auto assessment = scorer.assessRisk(features);
            
            json response = {
                {"success", true},
                {"message", "Risk assessed successfully"},
                {"status_code", 200},
                {"data", json::parse(assessment.toJson())}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = sdrs::models::Response<void>::error(std::string("Risk assessment failed: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    const int port = sdrs::constants::ports::RISK_SERVICE_PORT;
    std::cout << "Risk Assessment Service listening on port " << port << std::endl;
    sdrs::utils::Logger::Info("Risk Assessment Service started on port " + std::to_string(port));
    
    server.listen("0.0.0.0", port);
    
    return 0;
}
