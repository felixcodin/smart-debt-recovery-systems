#pragma once

#include "../models/RiskScorer.h"
#include "../../../common/include/database/DatabaseManager.h"
#include <vector>
#include <memory>

namespace sdrs::risk
{

class RiskAssessmentRepository
{
private:
    std::shared_ptr<sdrs::database::DatabaseManager> _dbManager;
    bool _useMockMode;

public:
    explicit RiskAssessmentRepository(std::shared_ptr<sdrs::database::DatabaseManager> dbManager = nullptr);

    // CRUD operations
    RiskAssessment create(const RiskAssessment& assessment);
    RiskAssessment getById(int assessmentId);
    std::vector<RiskAssessment> getByAccountId(int accountId);
    std::vector<RiskAssessment> getByBorrowerId(int borrowerId);
    std::vector<RiskAssessment> getAll();
    
    // Query by criteria
    std::vector<RiskAssessment> getByRiskLevel(sdrs::constants::RiskLevel level);
    std::vector<RiskAssessment> getRecentAssessments(int limit = 10);
    
    // Delete operations
    bool deleteById(int assessmentId);

private:
    // Mock mode implementations
    RiskAssessment createMock(const RiskAssessment& assessment);
    RiskAssessment getByIdMock(int assessmentId);
    std::vector<RiskAssessment> getAllMock();
    std::vector<RiskAssessment> getByAccountIdMock(int accountId);
    std::vector<RiskAssessment> getByBorrowerIdMock(int borrowerId);
    
    // Helper methods
    RiskAssessment mapRowToRiskAssessment(const pqxx::row& row);
    static int _nextMockId;
    static std::vector<RiskAssessment> _mockStorage;
};

} // namespace sdrs::risk
