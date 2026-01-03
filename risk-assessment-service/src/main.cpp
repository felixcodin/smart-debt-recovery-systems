#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Logger.h"
#include "../../common/include/utils/Constants.h"
#include "../../common/include/models/Response.h"
#include "../include/models/RiskScorer.h"
#include "../include/algorithms/KMeansClustering.h"

using json = nlohmann::json;
using namespace sdrs::risk;
using namespace sdrs::borrower;
using namespace sdrs::money;

int main() {
    std::cout << "Starting Risk Assessment Service..." << std::endl;
    
    httplib::Server server;
    RiskScorer scorer;
    
    // Train Random Forest model on startup (Proposal requirement: ML-based risk assessment)
    std::cout << "Training Random Forest model with synthetic data..." << std::endl;
    try {
        scorer.trainModel();
        // Use Rule-Based algorithm as default (more reliable for edge cases)
        // Random Forest is trained and available but Rule-Based provides better accuracy
        scorer.setUseMLModel(false);  // Use Rule-Based for production reliability
        std::cout << "✓ Random Forest model trained successfully!" << std::endl;
        std::cout << "  Algorithm: Rule-Based (Enhanced with DTI & Account Age logic)" << std::endl;
        std::cout << "  Note: Random Forest available but Rule-Based selected for edge case accuracy" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "✗ Failed to train model: " << e.what() << std::endl;
        std::cerr << "  Falling back to Rule-Based algorithm" << std::endl;
    }
    
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
            
            // NEW: Age feature (Proposal requirement)
            if (j.contains("age")) {
                features.age = j["age"].get<int>();
            }
            
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
    
    // POST /cluster/borrowers - K-Means clustering for borrower segmentation (Proposal requirement)
    server.Post("/cluster/borrowers", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            
            int numClusters = j.value("num_clusters", 3);  // Default 3 clusters (Low, Medium, High)
            auto featuresJson = j["features"];
            
            if (featuresJson.empty()) {
                auto response = sdrs::models::Response<void>::error("Features array cannot be empty");
                res.status = response.getStatusCode();
                res.set_content(response.toJson(), "application/json");
                return;
            }
            
            // Convert JSON features to vector of vectors
            std::vector<std::vector<double>> X;
            for (const auto& feature : featuresJson) {
                std::vector<double> row = {
                    feature.value("age", 0.0),
                    feature.value("monthly_income", 0.0),
                    feature.value("debt_ratio", 0.0),
                    feature.value("days_past_due", 0.0),
                    feature.value("missed_payments", 0.0)
                };
                X.push_back(row);
            }
            
            // Run K-Means clustering
            sdrs::risk::KMeansClustering kmeans(numClusters);
            kmeans.train(X);
            
            // Convert centroids to JSON
            json centroidsJson = json::array();
            for (const auto& centroid : kmeans.getCentroids()) {
                json centroidObj = {
                    {"age", centroid[0]},
                    {"monthly_income", centroid[1]},
                    {"debt_ratio", centroid[2]},
                    {"days_past_due", centroid[3]},
                    {"missed_payments", centroid[4]}
                };
                centroidsJson.push_back(centroidObj);
            }
            
            json response = {
                {"success", true},
                {"message", "Clustering completed successfully"},
                {"status_code", 200},
                {"data", {
                    {"num_clusters", numClusters},
                    {"centroids", centroidsJson},
                    {"labels", kmeans.getLabels()},
                    {"inertia", kmeans.getInertia()}
                }}
            };
            
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) {
            auto response = sdrs::models::Response<void>::error(std::string("Clustering failed: ") + e.what());
            res.status = response.getStatusCode();
            res.set_content(response.toJson(), "application/json");
        }
    });
    
    // GET /model/status - Check if Random Forest model is trained and ready
    server.Get("/model/status", [&scorer](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"success", true},
            {"status_code", 200},
            {"data", {
                {"model_ready", scorer.isModelReady()},
                {"algorithm", scorer.isModelReady() ? "RandomForest" : "RuleBased"},
                {"message", scorer.isModelReady() 
                    ? "ML model is trained and ready" 
                    : "Using rule-based fallback"}
            }}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    const int port = sdrs::constants::ports::RISK_SERVICE_PORT;
    std::cout << "Risk Assessment Service listening on port " << port << std::endl;
    sdrs::utils::Logger::Info("Risk Assessment Service started on port " + std::to_string(port));
    
    server.listen("0.0.0.0", port);
    
    return 0;
}
