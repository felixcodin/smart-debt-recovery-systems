// RiskAssessmentRepository.cpp - PostgreSQL implementation

#include "../../include/repositories/RiskAssessmentRepository.h"
#include "../../../common/include/utils/Logger.h"
#include "../../../common/include/exceptions/DatabaseException.h"
#include <nlohmann/json.hpp>
#include <algorithm>

namespace sdrs::risk
{

// Static members for mock mode
int RiskAssessmentRepository::_nextMockId = 1;
std::vector<RiskAssessment> RiskAssessmentRepository::_mockStorage;

RiskAssessmentRepository::RiskAssessmentRepository(std::shared_ptr<sdrs::database::DatabaseManager> dbManager)
    : _dbManager(dbManager), _useMockMode(dbManager == nullptr)
{
    if (_useMockMode)
    {
        sdrs::utils::Logger::Info("[RiskAssessmentRepo] Running in MOCK mode");
    }
}

// ============================================================================
// CRUD Operations
// ============================================================================

RiskAssessment RiskAssessmentRepository::create(const RiskAssessment& assessment)
{
    if (_useMockMode) return createMock(assessment);
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> RiskAssessment {
            std::string sql = R"(
                INSERT INTO risk_assessments (
                    account_id, borrower_id, risk_score, risk_level,
                    algorithm_used, risk_factors, assessment_date
                ) VALUES ($1, $2, $3, $4, $5, $6, $7)
                RETURNING assessment_id, created_at
            )";
            
            // Convert risk_factors map to JSONB string
            nlohmann::json factorsJson = assessment.getRiskFactors();
            std::string factorsStr = factorsJson.dump();
            
            // Convert algorithm enum to string
            std::string algorithmStr = (assessment.getAlgorithmUsed() == AlgorithmUsed::RandomForest) 
                ? "RandomForest" : "RuleBase";
            
            // Convert timestamp to string
            auto assessTime = std::chrono::system_clock::to_time_t(assessment.getAssessmentDate());
            std::tm* tm = std::localtime(&assessTime);
            char timeStr[20];
            std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm);
            
            pqxx::result result = txn.exec_params(sql,
                assessment.getAccountId(),
                assessment.getBorrowerId(),
                assessment.getRiskScore(),
                RiskAssessment::riskLevelToString(assessment.getRiskLevel()),
                algorithmStr,
                factorsStr,
                timeStr
            );
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException(
                    "Failed to insert risk assessment",
                    sdrs::constants::DatabaseErrorCode::QueryFailed
                );
            }
            
            int newId = result[0]["assessment_id"].as<int>();
            
            // Create new assessment with DB-generated ID
            RiskAssessment created = assessment;
            // Note: Cannot modify assessment_id after construction, 
            // so we return the original with assumption DB assigned correct ID
            
            sdrs::utils::Logger::Info("[RiskAssessmentRepo] Created assessment ID: " + std::to_string(newId));
            return created;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[RiskAssessmentRepo] SQL error in create: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

RiskAssessment RiskAssessmentRepository::getById(int assessmentId)
{
    if (_useMockMode) return getByIdMock(assessmentId);
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> RiskAssessment {
            std::string sql = R"(
                SELECT assessment_id, account_id, borrower_id, risk_score, risk_level,
                       algorithm_used, risk_factors, assessment_date, created_at
                FROM risk_assessments
                WHERE assessment_id = $1
            )";
            
            pqxx::result result = txn.exec_params(sql, assessmentId);
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException(
                    "Risk assessment not found: " + std::to_string(assessmentId),
                    sdrs::constants::DatabaseErrorCode::RecordNotFound
                );
            }
            
            return mapRowToRiskAssessment(result[0]);
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[RiskAssessmentRepo] SQL error in getById: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<RiskAssessment> RiskAssessmentRepository::getByAccountId(int accountId)
{
    if (_useMockMode) return getByAccountIdMock(accountId);
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<RiskAssessment> {
            std::string sql = R"(
                SELECT assessment_id, account_id, borrower_id, risk_score, risk_level,
                       algorithm_used, risk_factors, assessment_date, created_at
                FROM risk_assessments
                WHERE account_id = $1
                ORDER BY assessment_date DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, accountId);
            
            std::vector<RiskAssessment> assessments;
            for (const auto& row : result)
            {
                assessments.push_back(mapRowToRiskAssessment(row));
            }
            
            return assessments;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[RiskAssessmentRepo] SQL error in getByAccountId: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<RiskAssessment> RiskAssessmentRepository::getByBorrowerId(int borrowerId)
{
    if (_useMockMode) return getByBorrowerIdMock(borrowerId);
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<RiskAssessment> {
            std::string sql = R"(
                SELECT assessment_id, account_id, borrower_id, risk_score, risk_level,
                       algorithm_used, risk_factors, assessment_date, created_at
                FROM risk_assessments
                WHERE borrower_id = $1
                ORDER BY assessment_date DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, borrowerId);
            
            std::vector<RiskAssessment> assessments;
            for (const auto& row : result)
            {
                assessments.push_back(mapRowToRiskAssessment(row));
            }
            
            return assessments;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[RiskAssessmentRepo] SQL error in getByBorrowerId: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<RiskAssessment> RiskAssessmentRepository::getAll()
{
    if (_useMockMode) return getAllMock();
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<RiskAssessment> {
            std::string sql = R"(
                SELECT assessment_id, account_id, borrower_id, risk_score, risk_level,
                       algorithm_used, risk_factors, assessment_date, created_at
                FROM risk_assessments
                ORDER BY assessment_date DESC
            )";
            
            pqxx::result result = txn.exec_params(sql);
            
            std::vector<RiskAssessment> assessments;
            for (const auto& row : result)
            {
                assessments.push_back(mapRowToRiskAssessment(row));
            }
            
            return assessments;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[RiskAssessmentRepo] SQL error in getAll: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<RiskAssessment> RiskAssessmentRepository::getByRiskLevel(sdrs::constants::RiskLevel level)
{
    if (_useMockMode)
    {
        std::vector<RiskAssessment> filtered;
        for (const auto& assessment : _mockStorage)
        {
            if (assessment.getRiskLevel() == level)
            {
                filtered.push_back(assessment);
            }
        }
        return filtered;
    }
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<RiskAssessment> {
            std::string sql = R"(
                SELECT assessment_id, account_id, borrower_id, risk_score, risk_level,
                       algorithm_used, risk_factors, assessment_date, created_at
                FROM risk_assessments
                WHERE risk_level = $1
                ORDER BY assessment_date DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, RiskAssessment::riskLevelToString(level));
            
            std::vector<RiskAssessment> assessments;
            for (const auto& row : result)
            {
                assessments.push_back(mapRowToRiskAssessment(row));
            }
            
            return assessments;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[RiskAssessmentRepo] SQL error in getByRiskLevel: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<RiskAssessment> RiskAssessmentRepository::getRecentAssessments(int limit)
{
    if (_useMockMode)
    {
        std::vector<RiskAssessment> recent;
        int count = std::min(static_cast<int>(_mockStorage.size()), limit);
        for (int i = 0; i < count; i++)
        {
            recent.push_back(_mockStorage[i]);
        }
        return recent;
    }
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<RiskAssessment> {
            std::string sql = R"(
                SELECT assessment_id, account_id, borrower_id, risk_score, risk_level,
                       algorithm_used, risk_factors, assessment_date, created_at
                FROM risk_assessments
                ORDER BY assessment_date DESC
                LIMIT $1
            )";
            
            pqxx::result result = txn.exec_params(sql, limit);
            
            std::vector<RiskAssessment> assessments;
            for (const auto& row : result)
            {
                assessments.push_back(mapRowToRiskAssessment(row));
            }
            
            return assessments;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[RiskAssessmentRepo] SQL error in getRecentAssessments: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

bool RiskAssessmentRepository::deleteById(int assessmentId)
{
    if (_useMockMode)
    {
        auto it = std::find_if(_mockStorage.begin(), _mockStorage.end(),
            [assessmentId](const RiskAssessment& a) { return a.getAssessmentId() == assessmentId; });
        
        if (it != _mockStorage.end())
        {
            _mockStorage.erase(it);
            return true;
        }
        return false;
    }
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> bool {
            std::string sql = "DELETE FROM risk_assessments WHERE assessment_id = $1";
            pqxx::result result = txn.exec_params(sql, assessmentId);
            return result.affected_rows() > 0;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[RiskAssessmentRepo] SQL error in deleteById: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

// ============================================================================
// Mock Mode Implementations
// ============================================================================

RiskAssessment RiskAssessmentRepository::createMock(const RiskAssessment& assessment)
{
    // Since RiskAssessment has no setter for ID, we return the same assessment
    // In real scenario, we'd need to modify RiskAssessment to allow ID setting
    _mockStorage.push_back(assessment);
    _nextMockId++;
    
    sdrs::utils::Logger::Info("[RiskAssessmentRepo-Mock] Created assessment");
    return assessment;
}

RiskAssessment RiskAssessmentRepository::getByIdMock(int assessmentId)
{
    for (const auto& assessment : _mockStorage)
    {
        if (assessment.getAssessmentId() == assessmentId)
        {
            return assessment;
        }
    }
    
    throw sdrs::exceptions::DatabaseException(
        "Risk assessment not found: " + std::to_string(assessmentId),
        sdrs::constants::DatabaseErrorCode::RecordNotFound
    );
    
    // Unreachable, but needed to suppress warning
    return RiskAssessment(0, 0, 0.0, AlgorithmUsed::RuleBase);
}

std::vector<RiskAssessment> RiskAssessmentRepository::getAllMock()
{
    return _mockStorage;
}

std::vector<RiskAssessment> RiskAssessmentRepository::getByAccountIdMock(int accountId)
{
    std::vector<RiskAssessment> filtered;
    for (const auto& assessment : _mockStorage)
    {
        if (assessment.getAccountId() == accountId)
        {
            filtered.push_back(assessment);
        }
    }
    return filtered;
}

std::vector<RiskAssessment> RiskAssessmentRepository::getByBorrowerIdMock(int borrowerId)
{
    std::vector<RiskAssessment> filtered;
    for (const auto& assessment : _mockStorage)
    {
        if (assessment.getBorrowerId() == borrowerId)
        {
            filtered.push_back(assessment);
        }
    }
    return filtered;
}

// ============================================================================
// Helper Methods
// ============================================================================

RiskAssessment RiskAssessmentRepository::mapRowToRiskAssessment(const pqxx::row& row)
{
    int accountId = row["account_id"].as<int>();
    int borrowerId = row["borrower_id"].as<int>();
    double riskScore = row["risk_score"].as<double>();
    
    std::string algorithmStr = row["algorithm_used"].as<std::string>();
    AlgorithmUsed algorithm = (algorithmStr == "RandomForest") 
        ? AlgorithmUsed::RandomForest 
        : AlgorithmUsed::RuleBase;
    
    RiskAssessment assessment(accountId, borrowerId, riskScore, algorithm);
    
    // Parse risk_factors JSONB
    if (!row["risk_factors"].is_null())
    {
        std::string factorsStr = row["risk_factors"].as<std::string>();
        try
        {
            nlohmann::json factorsJson = nlohmann::json::parse(factorsStr);
            for (auto& [key, value] : factorsJson.items())
            {
                if (value.is_number())
                {
                    assessment.addRiskFactor(key, value.get<double>());
                }
            }
        }
        catch (const nlohmann::json::exception& e)
        {
            sdrs::utils::Logger::Warn("[RiskAssessmentRepo] Failed to parse risk_factors JSON: " + std::string(e.what()));
        }
    }
    
    return assessment;
}

} // namespace sdrs::risk
