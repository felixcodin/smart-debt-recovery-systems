// PaymentHistoryRepository.cpp - PostgreSQL implementation

#include "../../include/repositories/PaymentHistoryRepository.h"
#include "../../../common/include/utils/Logger.h"
#include "../../../common/include/utils/Constants.h"
#include "../../../common/include/exceptions/DatabaseException.h"

namespace sdrs::borrower
{

PaymentHistoryRepository::PaymentHistoryRepository(bool useMock)
    : _useMock(useMock)
{
}

// ============================================================================
// CRUD Operations
// ============================================================================

PaymentHistory PaymentHistoryRepository::create(const PaymentHistory& payment)
{
    if (_useMock) return createMock(payment);
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> PaymentHistory {
            std::string sql = R"(
                INSERT INTO payment_history (
                    account_id, payment_amount, payment_method, payment_status,
                    payment_date, due_date, is_late, notes
                ) VALUES ($1, $2, $3::payment_method_enum, $4::payment_status_enum, $5, $6, $7, $8)
                RETURNING payment_id, created_at
            )";
            
            // Convert dates to strings
            auto paymentDate = payment.getPaymentDate();
            auto paymentDateStr = std::format("{:%Y-%m-%d}", paymentDate);
            
            std::optional<std::string> dueDateStr;
            if (payment.getDueDate().has_value())
            {
                dueDateStr = std::format("{:%Y-%m-%d}", payment.getDueDate().value());
            }
            
            pqxx::result result;
            if (dueDateStr.has_value())
            {
                result = txn.exec_params(sql,
                    payment.getAccountId(),
                    payment.getPaymentAmount().getAmount(),
                    PaymentHistory::paymentMethodToString(payment.getMethod()),
                    PaymentHistory::paymentStatusToString(payment.getStatus()),
                    paymentDateStr,
                    dueDateStr.value(),
                    payment.isLate(),
                    payment.getNotes()
                );
            }
            else
            {
                result = txn.exec_params(sql,
                    payment.getAccountId(),
                    payment.getPaymentAmount().getAmount(),
                    PaymentHistory::paymentMethodToString(payment.getMethod()),
                    PaymentHistory::paymentStatusToString(payment.getStatus()),
                    paymentDateStr,
                    nullptr,
                    payment.isLate(),
                    payment.getNotes()
                );
            }
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException("Failed to insert payment record", sdrs::constants::DatabaseErrorCode::QueryFailed);
            }
            
            int newId = result[0]["payment_id"].as<int>();
            
            sdrs::utils::Logger::Info("[DB] Created payment ID: " + std::to_string(newId));
            
            // Return payment with new ID
            PaymentHistory created(newId, payment.getAccountId(), payment.getPaymentAmount(),
                                   payment.getMethod(), payment.getPaymentDate(), payment.getDueDate());
            return created;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in create: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return createMock(payment); // Unreachable
}

std::optional<PaymentHistory> PaymentHistoryRepository::findById(int paymentId)
{
    if (_useMock) return findByIdMock(paymentId);
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::optional<PaymentHistory> {
            std::string sql = R"(
                SELECT payment_id, account_id, payment_amount, payment_method, payment_status,
                       payment_date, due_date, is_late, notes, created_at, updated_at
                FROM payment_history
                WHERE payment_id = $1
            )";
            
            pqxx::result result = txn.exec_params(sql, paymentId);
            
            if (result.empty())
            {
                return std::nullopt;
            }
            
            return mapRowToPaymentHistory(result[0]);
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findById: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return std::nullopt; // Unreachable
}

PaymentHistory PaymentHistoryRepository::update(const PaymentHistory& payment)
{
    if (_useMock) return payment;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> PaymentHistory {
            std::string sql = R"(
                UPDATE payment_history SET
                    payment_status = $2::payment_status_enum,
                    notes = $3
                WHERE payment_id = $1
                RETURNING payment_id
            )";
            
            pqxx::result result = txn.exec_params(sql,
                payment.getPaymentId(),
                PaymentHistory::paymentStatusToString(payment.getStatus()),
                payment.getNotes()
            );
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException(
                    "Payment not found with ID: " + std::to_string(payment.getPaymentId()), sdrs::constants::DatabaseErrorCode::QueryFailed);
            }
            
            sdrs::utils::Logger::Info("[DB] Updated payment ID: " + std::to_string(payment.getPaymentId()));
            return payment;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in update: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return payment; // Unreachable
}

bool PaymentHistoryRepository::deleteById(int paymentId)
{
    if (_useMock) return paymentId > 0;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> bool {
            std::string sql = "DELETE FROM payment_history WHERE payment_id = $1";
            pqxx::result result = txn.exec_params(sql, paymentId);
            
            bool deleted = result.affected_rows() > 0;
            if (deleted)
            {
                sdrs::utils::Logger::Info("[DB] Deleted payment ID: " + std::to_string(paymentId));
            }
            return deleted;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in deleteById: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return false; // Unreachable
}

// ============================================================================
// Query Operations
// ============================================================================

std::vector<PaymentHistory> PaymentHistoryRepository::findByAccountId(int accountId)
{
    if (_useMock) return findByAccountIdMock(accountId);
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<PaymentHistory> {
            std::string sql = R"(
                SELECT payment_id, account_id, payment_amount, payment_method, payment_status,
                       payment_date, due_date, is_late, notes, created_at, updated_at
                FROM payment_history
                WHERE account_id = $1
                ORDER BY payment_date DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, accountId);
            
            std::vector<PaymentHistory> payments;
            payments.reserve(result.size());
            
            for (const auto& row : result)
            {
                payments.push_back(mapRowToPaymentHistory(row));
            }
            
            return payments;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findByAccountId: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return {}; // Unreachable
}

std::vector<PaymentHistory> PaymentHistoryRepository::findByStatus(sdrs::constants::PaymentStatus status)
{
    if (_useMock) return findAllMock();
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<PaymentHistory> {
            std::string sql = R"(
                SELECT payment_id, account_id, payment_amount, payment_method, payment_status,
                       payment_date, due_date, is_late, notes, created_at, updated_at
                FROM payment_history
                WHERE payment_status = $1::payment_status_enum
                ORDER BY payment_date DESC
            )";
            
            // Convert enum to string
            std::string statusStr;
            switch (status)
            {
                case sdrs::constants::PaymentStatus::Pending: statusStr = "Pending"; break;
                case sdrs::constants::PaymentStatus::Completed: statusStr = "Completed"; break;
                case sdrs::constants::PaymentStatus::Failed: statusStr = "Failed"; break;
                case sdrs::constants::PaymentStatus::Cancelled: statusStr = "Cancelled"; break;
            }
            
            pqxx::result result = txn.exec_params(sql, statusStr);
            
            std::vector<PaymentHistory> payments;
            payments.reserve(result.size());
            
            for (const auto& row : result)
            {
                payments.push_back(mapRowToPaymentHistory(row));
            }
            
            return payments;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findByStatus: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return {}; // Unreachable
}

std::vector<PaymentHistory> PaymentHistoryRepository::findLatePayments()
{
    if (_useMock) return findAllMock();
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<PaymentHistory> {
            std::string sql = R"(
                SELECT payment_id, account_id, payment_amount, payment_method, payment_status,
                       payment_date, due_date, is_late, notes, created_at, updated_at
                FROM payment_history
                WHERE is_late = true
                ORDER BY payment_date DESC
            )";
            
            pqxx::result result = txn.exec(sql);
            
            std::vector<PaymentHistory> payments;
            payments.reserve(result.size());
            
            for (const auto& row : result)
            {
                payments.push_back(mapRowToPaymentHistory(row));
            }
            
            sdrs::utils::Logger::Info("[DB] Found " + std::to_string(payments.size()) + " late payments");
            return payments;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findLatePayments: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return {}; // Unreachable
}

std::vector<PaymentHistory> PaymentHistoryRepository::findByDateRange(const std::string& startDate, const std::string& endDate)
{
    if (_useMock) return findAllMock();
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<PaymentHistory> {
            std::string sql = R"(
                SELECT payment_id, account_id, payment_amount, payment_method, payment_status,
                       payment_date, due_date, is_late, notes, created_at, updated_at
                FROM payment_history
                WHERE payment_date >= $1::date AND payment_date <= $2::date
                ORDER BY payment_date DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, startDate, endDate);
            
            std::vector<PaymentHistory> payments;
            payments.reserve(result.size());
            
            for (const auto& row : result)
            {
                payments.push_back(mapRowToPaymentHistory(row));
            }
            
            return payments;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findByDateRange: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return {}; // Unreachable
}

std::vector<PaymentHistory> PaymentHistoryRepository::findPendingPayments()
{
    return findByStatus(sdrs::constants::PaymentStatus::Pending);
}

std::vector<PaymentHistory> PaymentHistoryRepository::findAll()
{
    if (_useMock) return findAllMock();
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<PaymentHistory> {
            std::string sql = R"(
                SELECT payment_id, account_id, payment_amount, payment_method, payment_status,
                       payment_date, due_date, is_late, notes, created_at, updated_at
                FROM payment_history
                ORDER BY created_at DESC
                LIMIT 100
            )";
            
            pqxx::result result = txn.exec(sql);
            
            std::vector<PaymentHistory> payments;
            payments.reserve(result.size());
            
            for (const auto& row : result)
            {
                payments.push_back(mapRowToPaymentHistory(row));
            }
            
            return payments;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findAll: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return {}; // Unreachable
}

int PaymentHistoryRepository::count()
{
    if (_useMock) return 3;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> int {
            pqxx::result result = txn.exec("SELECT COUNT(*) FROM payment_history");
            return result[0][0].as<int>();
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in count: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return 0; // Unreachable
}

double PaymentHistoryRepository::sumPaymentsForAccount(int accountId)
{
    if (_useMock) return 5000000.0;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> double {
            std::string sql = R"(
                SELECT COALESCE(SUM(payment_amount), 0) 
                FROM payment_history 
                WHERE account_id = $1 AND payment_status = 'Completed'
            )";
            
            pqxx::result result = txn.exec_params(sql, accountId);
            return result[0][0].as<double>();
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in sumPaymentsForAccount: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return 0.0; // Unreachable
}

// ============================================================================
// Update Operations
// ============================================================================

bool PaymentHistoryRepository::updateStatus(int paymentId, sdrs::constants::PaymentStatus status)
{
    if (_useMock) return true;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> bool {
            std::string sql = R"(
                UPDATE payment_history 
                SET payment_status = $2::payment_status_enum
                WHERE payment_id = $1
            )";
            
            std::string statusStr;
            switch (status)
            {
                case sdrs::constants::PaymentStatus::Pending: statusStr = "Pending"; break;
                case sdrs::constants::PaymentStatus::Completed: statusStr = "Completed"; break;
                case sdrs::constants::PaymentStatus::Failed: statusStr = "Failed"; break;
                case sdrs::constants::PaymentStatus::Cancelled: statusStr = "Cancelled"; break;
            }
            
            pqxx::result result = txn.exec_params(sql, paymentId, statusStr);
            return result.affected_rows() > 0;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in updateStatus: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return false; // Unreachable
}

bool PaymentHistoryRepository::markAsLate(int paymentId, bool isLate)
{
    if (_useMock) return true;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> bool {
            std::string sql = R"(
                UPDATE payment_history 
                SET is_late = $2
                WHERE payment_id = $1
            )";
            
            pqxx::result result = txn.exec_params(sql, paymentId, isLate);
            return result.affected_rows() > 0;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in markAsLate: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
    return false; // Unreachable
}

// ============================================================================
// Helper: Map database row to PaymentHistory object
// ============================================================================

PaymentHistory PaymentHistoryRepository::mapRowToPaymentHistory(const pqxx::row& row)
{
    int paymentId = row["payment_id"].as<int>();
    int accountId = row["account_id"].as<int>();
    double amount = row["payment_amount"].as<double>();
    
    // Parse method
    std::string methodStr = row["payment_method"].as<std::string>();
    auto method = PaymentHistory::stringToPaymentMethod(methodStr);
    
    // Parse payment date
    std::string paymentDateStr = row["payment_date"].as<std::string>();
    
    // Parse to sys_days (simplified - assume YYYY-MM-DD format)
    int year, month, day;
    std::sscanf(paymentDateStr.c_str(), "%d-%d-%d", &year, &month, &day);
    std::chrono::sys_days paymentDate = std::chrono::year{year}/month/day;
    
    // Parse due date (optional)
    std::optional<std::chrono::sys_days> dueDate;
    if (!row["due_date"].is_null())
    {
        std::string dueDateStr = row["due_date"].as<std::string>();
        std::sscanf(dueDateStr.c_str(), "%d-%d-%d", &year, &month, &day);
        dueDate = std::chrono::year{year}/month/day;
    }
    
    sdrs::money::Money paymentAmount(amount);
    PaymentHistory payment(paymentId, accountId, paymentAmount, method, paymentDate, dueDate);
    
    // Set status
    if (!row["payment_status"].is_null())
    {
        std::string statusStr = row["payment_status"].as<std::string>();
        auto status = PaymentHistory::stringToPaymentStatus(statusStr);
        if (status == sdrs::constants::PaymentStatus::Completed)
        {
            payment.markCompleted();
        }
        else if (status == sdrs::constants::PaymentStatus::Failed)
        {
            payment.markFailed("From database");
        }
        else if (status == sdrs::constants::PaymentStatus::Cancelled)
        {
            payment.cancel("From database");
        }
    }
    
    // Set notes
    if (!row["notes"].is_null())
    {
        payment.setNotes(row["notes"].as<std::string>());
    }
    
    return payment;
}

// ============================================================================
// Mock implementations
// ============================================================================

PaymentHistory PaymentHistoryRepository::createMock(const PaymentHistory& payment)
{
    sdrs::utils::Logger::Info("[MOCK] Creating payment for account: " + std::to_string(payment.getAccountId()));
    return PaymentHistory(999, payment.getAccountId(), payment.getPaymentAmount(),
                          payment.getMethod(), payment.getPaymentDate(), payment.getDueDate());
}

std::optional<PaymentHistory> PaymentHistoryRepository::findByIdMock(int paymentId)
{
    sdrs::utils::Logger::Info("[MOCK] Finding payment by ID: " + std::to_string(paymentId));
    
    if (paymentId <= 0) return std::nullopt;
    
    auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    sdrs::money::Money amount(1000000.0);
    return PaymentHistory(paymentId, 1, amount, sdrs::constants::PaymentMethod::BankTransfer, today);
}

std::vector<PaymentHistory> PaymentHistoryRepository::findByAccountIdMock(int accountId)
{
    sdrs::utils::Logger::Info("[MOCK] Finding payments for account: " + std::to_string(accountId));
    
    auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    
    std::vector<PaymentHistory> payments;
    payments.emplace_back(1, accountId, sdrs::money::Money(1000000.0), 
                          sdrs::constants::PaymentMethod::BankTransfer, today);
    payments.emplace_back(2, accountId, sdrs::money::Money(1000000.0), 
                          sdrs::constants::PaymentMethod::Cash, today - std::chrono::days(30));
    return payments;
}

std::vector<PaymentHistory> PaymentHistoryRepository::findAllMock()
{
    sdrs::utils::Logger::Info("[MOCK] Finding all payments");
    
    auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    
    std::vector<PaymentHistory> payments;
    payments.emplace_back(1, 1, sdrs::money::Money(1000000.0), 
                          sdrs::constants::PaymentMethod::BankTransfer, today);
    payments.emplace_back(2, 1, sdrs::money::Money(500000.0), 
                          sdrs::constants::PaymentMethod::Card, today - std::chrono::days(15));
    payments.emplace_back(3, 2, sdrs::money::Money(2000000.0), 
                          sdrs::constants::PaymentMethod::Cash, today - std::chrono::days(30));
    return payments;
}

} // namespace sdrs::borrower
