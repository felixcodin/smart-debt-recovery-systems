#pragma once

#include "../models/CommunicationLog.h"
#include "../../../common/include/database/DatabaseManager.h"
#include <vector>
#include <memory>
#include <chrono>

namespace sdrs::communication
{

class CommunicationLogRepository
{
private:
    std::shared_ptr<sdrs::database::DatabaseManager> _dbManager;
    bool _useMockMode;

public:
    explicit CommunicationLogRepository(std::shared_ptr<sdrs::database::DatabaseManager> dbManager = nullptr);

    // CRUD operations
    CommunicationLog create(const CommunicationLog& log);
    CommunicationLog getById(int communicationId);
    std::vector<CommunicationLog> getByAccountId(int accountId);
    std::vector<CommunicationLog> getByBorrowerId(int borrowerId);
    std::vector<CommunicationLog> getAll();
    
    // Query by criteria
    std::vector<CommunicationLog> getByChannelType(ChannelType channelType);
    std::vector<CommunicationLog> getByStatus(MessageStatus status);
    std::vector<CommunicationLog> getByDateRange(
        std::chrono::sys_seconds from,
        std::chrono::sys_seconds to
    );
    
    // Update operations
    CommunicationLog update(const CommunicationLog& log);
    bool deleteById(int communicationId);

private:
    // Mock mode implementations
    CommunicationLog createMock(const CommunicationLog& log);
    CommunicationLog getByIdMock(int communicationId);
    std::vector<CommunicationLog> getAllMock();
    std::vector<CommunicationLog> getByAccountIdMock(int accountId);
    std::vector<CommunicationLog> getByBorrowerIdMock(int borrowerId);
    CommunicationLog updateMock(const CommunicationLog& log);
    
    // Helper methods
    CommunicationLog mapRowToCommunicationLog(const pqxx::row& row);
    
    static int _nextMockId;
    static std::vector<CommunicationLog> _mockStorage;
};

} // namespace sdrs::communication
