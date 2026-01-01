// CommunicationLogRepository.cpp - PostgreSQL implementation

#include "../../include/repositories/CommunicationLogRepository.h"
#include "../../../common/include/utils/Logger.h"
#include "../../../common/include/exceptions/DatabaseException.h"
#include <algorithm>
#include <format>

namespace sdrs::communication
{

// Static members for mock mode
int CommunicationLogRepository::_nextMockId = 1;
std::vector<CommunicationLog> CommunicationLogRepository::_mockStorage;

CommunicationLogRepository::CommunicationLogRepository(std::shared_ptr<sdrs::database::DatabaseManager> dbManager)
    : _dbManager(dbManager), _useMockMode(dbManager == nullptr)
{
    if (_useMockMode)
    {
        sdrs::utils::Logger::Info("[CommunicationLogRepo] Running in MOCK mode");
    }
}

// ============================================================================
// CRUD Operations
// ============================================================================

CommunicationLog CommunicationLogRepository::create(const CommunicationLog& log)
{
    if (_useMockMode) return createMock(log);
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> CommunicationLog {
            std::string sql = R"(
                INSERT INTO communication_logs (
                    account_id, borrower_id, strategy_id,
                    channel_type, message_content, message_status
                ) VALUES ($1, $2, $3, $4, $5, $6)
                RETURNING communication_id, created_at, updated_at
            )";
            
            pqxx::result result;
            
            if (log.getStrategyId().has_value())
            {
                result = txn.exec_params(sql,
                    log.getAccountId(),
                    log.getBorrowerId(),
                    log.getStrategyId().value(),
                    CommunicationLog::channelTypeToString(log.getChannelType()),
                    log.getMessageContent(),
                    CommunicationLog::messageStatusToString(log.getMessageStatus())
                );
            }
            else
            {
                // Build a different SQL without strategy_id
                std::string sqlNoStrategy = R"(
                    INSERT INTO communication_logs (
                        account_id, borrower_id,
                        channel_type, message_content, message_status
                    ) VALUES ($1, $2, $3, $4, $5)
                    RETURNING communication_id, created_at, updated_at
                )";
                
                result = txn.exec_params(sqlNoStrategy,
                    log.getAccountId(),
                    log.getBorrowerId(),
                    CommunicationLog::channelTypeToString(log.getChannelType()),
                    log.getMessageContent(),
                    CommunicationLog::messageStatusToString(log.getMessageStatus())
                );
            }
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException(
                    "Failed to insert communication log",
                    sdrs::constants::DatabaseErrorCode::QueryFailed
                );
            }
            
            int newId = result[0]["communication_id"].as<int>();
            
            sdrs::utils::Logger::Info("[CommunicationLogRepo] Created log ID: " + std::to_string(newId));
            
            // Return the created log (ideally we'd fetch it back from DB)
            return log;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[CommunicationLogRepo] SQL error in create: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

CommunicationLog CommunicationLogRepository::getById(int communicationId)
{
    if (_useMockMode) return getByIdMock(communicationId);
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> CommunicationLog {
            std::string sql = R"(
                SELECT communication_id, account_id, borrower_id, strategy_id,
                       channel_type, message_content, message_status,
                       sent_at, delivered_at, error_message,
                       created_at, updated_at
                FROM communication_logs
                WHERE communication_id = $1
            )";
            
            pqxx::result result = txn.exec_params(sql, communicationId);
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException(
                    "Communication log not found: " + std::to_string(communicationId),
                    sdrs::constants::DatabaseErrorCode::RecordNotFound
                );
            }
            
            return mapRowToCommunicationLog(result[0]);
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[CommunicationLogRepo] SQL error in getById: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<CommunicationLog> CommunicationLogRepository::getByAccountId(int accountId)
{
    if (_useMockMode) return getByAccountIdMock(accountId);
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<CommunicationLog> {
            std::string sql = R"(
                SELECT communication_id, account_id, borrower_id, strategy_id,
                       channel_type, message_content, message_status,
                       sent_at, delivered_at, error_message,
                       created_at, updated_at
                FROM communication_logs
                WHERE account_id = $1
                ORDER BY created_at DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, accountId);
            
            std::vector<CommunicationLog> logs;
            for (const auto& row : result)
            {
                logs.push_back(mapRowToCommunicationLog(row));
            }
            
            return logs;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[CommunicationLogRepo] SQL error in getByAccountId: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<CommunicationLog> CommunicationLogRepository::getByBorrowerId(int borrowerId)
{
    if (_useMockMode) return getByBorrowerIdMock(borrowerId);
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<CommunicationLog> {
            std::string sql = R"(
                SELECT communication_id, account_id, borrower_id, strategy_id,
                       channel_type, message_content, message_status,
                       sent_at, delivered_at, error_message,
                       created_at, updated_at
                FROM communication_logs
                WHERE borrower_id = $1
                ORDER BY created_at DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, borrowerId);
            
            std::vector<CommunicationLog> logs;
            for (const auto& row : result)
            {
                logs.push_back(mapRowToCommunicationLog(row));
            }
            
            return logs;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[CommunicationLogRepo] SQL error in getByBorrowerId: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<CommunicationLog> CommunicationLogRepository::getAll()
{
    if (_useMockMode) return getAllMock();
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<CommunicationLog> {
            std::string sql = R"(
                SELECT communication_id, account_id, borrower_id, strategy_id,
                       channel_type, message_content, message_status,
                       sent_at, delivered_at, error_message,
                       created_at, updated_at
                FROM communication_logs
                ORDER BY created_at DESC
            )";
            
            pqxx::result result = txn.exec_params(sql);
            
            std::vector<CommunicationLog> logs;
            for (const auto& row : result)
            {
                logs.push_back(mapRowToCommunicationLog(row));
            }
            
            return logs;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[CommunicationLogRepo] SQL error in getAll: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<CommunicationLog> CommunicationLogRepository::getByChannelType(ChannelType channelType)
{
    if (_useMockMode)
    {
        std::vector<CommunicationLog> filtered;
        for (const auto& log : _mockStorage)
        {
            if (log.getChannelType() == channelType)
            {
                filtered.push_back(log);
            }
        }
        return filtered;
    }
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<CommunicationLog> {
            std::string sql = R"(
                SELECT communication_id, account_id, borrower_id, strategy_id,
                       channel_type, message_content, message_status,
                       sent_at, delivered_at, error_message,
                       created_at, updated_at
                FROM communication_logs
                WHERE channel_type = $1
                ORDER BY created_at DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, CommunicationLog::channelTypeToString(channelType));
            
            std::vector<CommunicationLog> logs;
            for (const auto& row : result)
            {
                logs.push_back(mapRowToCommunicationLog(row));
            }
            
            return logs;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[CommunicationLogRepo] SQL error in getByChannelType: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<CommunicationLog> CommunicationLogRepository::getByStatus(MessageStatus status)
{
    if (_useMockMode)
    {
        std::vector<CommunicationLog> filtered;
        for (const auto& log : _mockStorage)
        {
            if (log.getMessageStatus() == status)
            {
                filtered.push_back(log);
            }
        }
        return filtered;
    }
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<CommunicationLog> {
            std::string sql = R"(
                SELECT communication_id, account_id, borrower_id, strategy_id,
                       channel_type, message_content, message_status,
                       sent_at, delivered_at, error_message,
                       created_at, updated_at
                FROM communication_logs
                WHERE message_status = $1
                ORDER BY created_at DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, CommunicationLog::messageStatusToString(status));
            
            std::vector<CommunicationLog> logs;
            for (const auto& row : result)
            {
                logs.push_back(mapRowToCommunicationLog(row));
            }
            
            return logs;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[CommunicationLogRepo] SQL error in getByStatus: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<CommunicationLog> CommunicationLogRepository::getByDateRange(
    std::chrono::sys_seconds from,
    std::chrono::sys_seconds to
)
{
    if (_useMockMode)
    {
        std::vector<CommunicationLog> filtered;
        for (const auto& log : _mockStorage)
        {
            if (log.getCreatedAt() >= from && log.getCreatedAt() <= to)
            {
                filtered.push_back(log);
            }
        }
        return filtered;
    }
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> std::vector<CommunicationLog> {
            std::string sql = R"(
                SELECT communication_id, account_id, borrower_id, strategy_id,
                       channel_type, message_content, message_status,
                       sent_at, delivered_at, error_message,
                       created_at, updated_at
                FROM communication_logs
                WHERE created_at BETWEEN $1 AND $2
                ORDER BY created_at DESC
            )";
            
            auto fromTime = std::chrono::system_clock::to_time_t(from);
            auto toTime = std::chrono::system_clock::to_time_t(to);
            
            char fromStr[20], toStr[20];
            std::strftime(fromStr, sizeof(fromStr), "%Y-%m-%d %H:%M:%S", std::localtime(&fromTime));
            std::strftime(toStr, sizeof(toStr), "%Y-%m-%d %H:%M:%S", std::localtime(&toTime));
            
            pqxx::result result = txn.exec_params(sql, fromStr, toStr);
            
            std::vector<CommunicationLog> logs;
            for (const auto& row : result)
            {
                logs.push_back(mapRowToCommunicationLog(row));
            }
            
            return logs;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[CommunicationLogRepo] SQL error in getByDateRange: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

CommunicationLog CommunicationLogRepository::update(const CommunicationLog& log)
{
    if (_useMockMode) return updateMock(log);
    
    try
    {
        return _dbManager->executeQuery([&](pqxx::work& txn) -> CommunicationLog {
            // Convert optional timestamps to strings
            std::optional<std::string> sentAtStr, deliveredAtStr, errorMsgStr;
            
            if (log.getSentAt().has_value())
            {
                auto t = std::chrono::system_clock::to_time_t(log.getSentAt().value());
                char buffer[20];
                std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
                sentAtStr = buffer;
            }
            
            if (log.getDeliveredAt().has_value())
            {
                auto t = std::chrono::system_clock::to_time_t(log.getDeliveredAt().value());
                char buffer[20];
                std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
                deliveredAtStr = buffer;
            }
            
            if (log.getErrorMessage().has_value())
            {
                errorMsgStr = log.getErrorMessage().value();
            }
            
            // Simple update - just status for now
            std::string sql = R"(
                UPDATE communication_logs
                SET message_status = $1,
                    updated_at = CURRENT_TIMESTAMP
                WHERE communication_id = $2
                RETURNING updated_at
            )";
            
            pqxx::result result = txn.exec_params(sql,
                CommunicationLog::messageStatusToString(log.getMessageStatus()),
                log.getCommunicationId()
            );
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException(
                    "Communication log not found for update: " + std::to_string(log.getCommunicationId()),
                    sdrs::constants::DatabaseErrorCode::RecordNotFound
                );
            }
            
            sdrs::utils::Logger::Info("[CommunicationLogRepo] Updated log ID: " + std::to_string(log.getCommunicationId()));
            return log;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[CommunicationLogRepo] SQL error in update: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

bool CommunicationLogRepository::deleteById(int communicationId)
{
    if (_useMockMode)
    {
        auto it = std::find_if(_mockStorage.begin(), _mockStorage.end(),
            [communicationId](const CommunicationLog& log) { return log.getCommunicationId() == communicationId; });
        
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
            std::string sql = "DELETE FROM communication_logs WHERE communication_id = $1";
            pqxx::result result = txn.exec_params(sql, communicationId);
            return result.affected_rows() > 0;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[CommunicationLogRepo] SQL error in deleteById: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

// ============================================================================
// Mock Mode Implementations
// ============================================================================

CommunicationLog CommunicationLogRepository::createMock(const CommunicationLog& log)
{
    _mockStorage.push_back(log);
    _nextMockId++;
    
    sdrs::utils::Logger::Info("[CommunicationLogRepo-Mock] Created log");
    return log;
}

CommunicationLog CommunicationLogRepository::getByIdMock(int communicationId)
{
    for (const auto& log : _mockStorage)
    {
        if (log.getCommunicationId() == communicationId)
        {
            return log;
        }
    }
    
    throw sdrs::exceptions::DatabaseException(
        "Communication log not found: " + std::to_string(communicationId),
        sdrs::constants::DatabaseErrorCode::RecordNotFound
    );
}

std::vector<CommunicationLog> CommunicationLogRepository::getAllMock()
{
    return _mockStorage;
}

std::vector<CommunicationLog> CommunicationLogRepository::getByAccountIdMock(int accountId)
{
    std::vector<CommunicationLog> filtered;
    for (const auto& log : _mockStorage)
    {
        if (log.getAccountId() == accountId)
        {
            filtered.push_back(log);
        }
    }
    return filtered;
}

std::vector<CommunicationLog> CommunicationLogRepository::getByBorrowerIdMock(int borrowerId)
{
    std::vector<CommunicationLog> filtered;
    for (const auto& log : _mockStorage)
    {
        if (log.getBorrowerId() == borrowerId)
        {
            filtered.push_back(log);
        }
    }
    return filtered;
}

CommunicationLog CommunicationLogRepository::updateMock(const CommunicationLog& log)
{
    for (auto& existing : _mockStorage)
    {
        if (existing.getCommunicationId() == log.getCommunicationId())
        {
            existing = log;
            sdrs::utils::Logger::Info("[CommunicationLogRepo-Mock] Updated log ID: " + std::to_string(log.getCommunicationId()));
            return log;
        }
    }
    
    throw sdrs::exceptions::DatabaseException(
        "Communication log not found for update: " + std::to_string(log.getCommunicationId()),
        sdrs::constants::DatabaseErrorCode::RecordNotFound
    );
}

// ============================================================================
// Helper Methods
// ============================================================================

CommunicationLog CommunicationLogRepository::mapRowToCommunicationLog(const pqxx::row& row)
{
    int communicationId = row["communication_id"].as<int>();
    int accountId = row["account_id"].as<int>();
    int borrowerId = row["borrower_id"].as<int>();
    
    std::optional<int> strategyId;
    if (!row["strategy_id"].is_null())
        strategyId = row["strategy_id"].as<int>();
    
    ChannelType channelType = CommunicationLog::stringToChannelType(row["channel_type"].as<std::string>());
    std::string messageContent = row["message_content"].as<std::string>();
    MessageStatus messageStatus = CommunicationLog::stringToMessageStatus(row["message_status"].as<std::string>());
    
    std::optional<std::chrono::sys_seconds> sentAt, deliveredAt;
    if (!row["sent_at"].is_null())
    {
        // TODO: Parse timestamp string to chrono properly
        sentAt = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    }
    
    if (!row["delivered_at"].is_null())
    {
        // TODO: Parse timestamp string to chrono properly
        deliveredAt = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    }
    
    std::optional<std::string> errorMessage;
    if (!row["error_message"].is_null())
        errorMessage = row["error_message"].as<std::string>();
    
    // Parse created_at and updated_at (placeholder - need proper parsing)
    auto createdAt = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    auto updatedAt = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    
    return CommunicationLog(
        communicationId,
        accountId,
        borrowerId,
        strategyId,
        channelType,
        messageContent,
        messageStatus,
        sentAt,
        deliveredAt,
        errorMessage,
        createdAt,
        updatedAt
    );
}

} // namespace sdrs::communication
