#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../common/include/utils/Logger.h"
#include "../../common/include/utils/Constants.h"
#include "../../common/include/models/Response.h"
#include "../include/models/Borrower.h"
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
    
    const int port = sdrs::constants::ports::BORROWER_SERVICE_PORT;
    std::cout << "Borrower Service listening on port " << port << std::endl;
    sdrs::utils::Logger::Info("Borrower Service started on port " + std::to_string(port));
    
    server.listen("0.0.0.0", port);
    
    return 0;
}
