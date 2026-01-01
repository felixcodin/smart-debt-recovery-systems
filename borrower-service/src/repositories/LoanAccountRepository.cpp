// LoanAccountRepository.cpp - PostgreSQL implementation

#include "../../include/repositories/LoanAccountRepository.h"
#include "../../../common/include/utils/Logger.h"
#include "../../../common/include/utils/Constants.h"
#include "../../../common/include/exceptions/DatabaseException.h"

namespace sdrs::borrower
{

LoanAccountRepository::LoanAccountRepository(bool useMock)
    : _useMock(useMock)
{
}

// ============================================================================
// CRUD Operations
// ============================================================================

LoanAccount LoanAccountRepository::create(const LoanAccount& account)
{
    if (_useMock) return createMock(account);
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> LoanAccount {
            std::string sql = R"(
                INSERT INTO loan_accounts (
                    borrower_id, loan_amount, initial_amount, interest_rate,
                    remaining_amount, loan_start_date, loan_end_date,
                    account_status, days_past_due, number_of_missed_payments
                ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8::account_status_enum, $9, $10)
                RETURNING account_id, created_at, updated_at
            )";
            
            pqxx::result result = txn.exec_params(sql,
                account.getBorrowerId(),
                account.getLoanAmount().getAmount(),
                account.getInitialAmount().getAmount(),
                account.getInterestRate(),
                account.getRemainingAmount().getAmount(),
                account.getLoanStartDateString(),
                account.getLoanEndDateString(),
                LoanAccount::statusToString(account.getStatus()),
                account.getDaysPastDue(),
                account.getMissedPayments()
            );
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException("Failed to insert loan account", sdrs::constants::DatabaseErrorCode::QueryFailed);
            }
            
            int newId = result[0]["account_id"].as<int>();
            
            LoanAccount created(newId, account.getBorrowerId(),
                               account.getLoanAmount().getAmount(),
                               account.getInterestRate(),
                               account.getLoanTermMonths());
            
            sdrs::utils::Logger::Info("[DB] Created loan account ID: " + std::to_string(newId));
            return created;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in create: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::optional<LoanAccount> LoanAccountRepository::findById(int accountId)
{
    if (_useMock) return findByIdMock(accountId);
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::optional<LoanAccount> {
            std::string sql = R"(
                SELECT account_id, borrower_id, loan_amount, initial_amount,
                       interest_rate, remaining_amount, loan_start_date, loan_end_date,
                       account_status, days_past_due, number_of_missed_payments,
                       created_at, updated_at
                FROM loan_accounts
                WHERE account_id = $1
            )";
            
            pqxx::result result = txn.exec_params(sql, accountId);
            
            if (result.empty())
            {
                return std::nullopt;
            }
            
            return mapRowToLoanAccount(result[0]);
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findById: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

LoanAccount LoanAccountRepository::update(const LoanAccount& account)
{
    if (_useMock) return account;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> LoanAccount {
            std::string sql = R"(
                UPDATE loan_accounts SET
                    remaining_amount = $2,
                    account_status = $3::account_status_enum,
                    days_past_due = $4,
                    number_of_missed_payments = $5
                WHERE account_id = $1
                RETURNING account_id
            )";
            
            pqxx::result result = txn.exec_params(sql,
                account.getAccountId(),
                account.getRemainingAmount().getAmount(),
                LoanAccount::statusToString(account.getStatus()),
                account.getDaysPastDue(),
                account.getMissedPayments()
            );
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException(
                    "Loan account not found with ID: " + std::to_string(account.getAccountId()), sdrs::constants::DatabaseErrorCode::QueryFailed);
            }
            
            sdrs::utils::Logger::Info("[DB] Updated loan account ID: " + std::to_string(account.getAccountId()));
            return account;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in update: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

bool LoanAccountRepository::deleteById(int accountId)
{
    if (_useMock) return accountId > 0;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> bool {
            std::string sql = "DELETE FROM loan_accounts WHERE account_id = $1";
            pqxx::result result = txn.exec_params(sql, accountId);
            
            bool deleted = result.affected_rows() > 0;
            if (deleted)
            {
                sdrs::utils::Logger::Info("[DB] Deleted loan account ID: " + std::to_string(accountId));
            }
            return deleted;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in deleteById: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

// ============================================================================
// Query Operations
// ============================================================================

std::vector<LoanAccount> LoanAccountRepository::findByBorrowerId(int borrowerId)
{
    if (_useMock) return findByBorrowerIdMock(borrowerId);
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<LoanAccount> {
            std::string sql = R"(
                SELECT account_id, borrower_id, loan_amount, initial_amount,
                       interest_rate, remaining_amount, loan_start_date, loan_end_date,
                       account_status, days_past_due, number_of_missed_payments,
                       created_at, updated_at
                FROM loan_accounts
                WHERE borrower_id = $1
                ORDER BY created_at DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, borrowerId);
            
            std::vector<LoanAccount> accounts;
            accounts.reserve(result.size());
            
            for (const auto& row : result)
            {
                accounts.push_back(mapRowToLoanAccount(row));
            }
            
            return accounts;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findByBorrowerId: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<LoanAccount> LoanAccountRepository::findByStatus(sdrs::constants::AccountStatus status)
{
    if (_useMock) return findAllMock();
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<LoanAccount> {
            std::string sql = R"(
                SELECT account_id, borrower_id, loan_amount, initial_amount,
                       interest_rate, remaining_amount, loan_start_date, loan_end_date,
                       account_status, days_past_due, number_of_missed_payments,
                       created_at, updated_at
                FROM loan_accounts
                WHERE account_status = $1::account_status_enum
                ORDER BY days_past_due DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, LoanAccount::statusToString(status));
            
            std::vector<LoanAccount> accounts;
            accounts.reserve(result.size());
            
            for (const auto& row : result)
            {
                accounts.push_back(mapRowToLoanAccount(row));
            }
            
            return accounts;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findByStatus: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<LoanAccount> LoanAccountRepository::findDelinquent(int minDaysPastDue)
{
    if (_useMock) return findAllMock();
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<LoanAccount> {
            std::string sql = R"(
                SELECT account_id, borrower_id, loan_amount, initial_amount,
                       interest_rate, remaining_amount, loan_start_date, loan_end_date,
                       account_status, days_past_due, number_of_missed_payments,
                       created_at, updated_at
                FROM loan_accounts
                WHERE days_past_due >= $1
                  AND account_status NOT IN ('PaidOff', 'ChargedOff', 'Settled')
                ORDER BY days_past_due DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, minDaysPastDue);
            
            std::vector<LoanAccount> accounts;
            accounts.reserve(result.size());
            
            for (const auto& row : result)
            {
                accounts.push_back(mapRowToLoanAccount(row));
            }
            
            sdrs::utils::Logger::Info("[DB] Found " + std::to_string(accounts.size()) + " delinquent accounts");
            return accounts;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findDelinquent: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<LoanAccount> LoanAccountRepository::findAll()
{
    if (_useMock) return findAllMock();
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<LoanAccount> {
            std::string sql = R"(
                SELECT account_id, borrower_id, loan_amount, initial_amount,
                       interest_rate, remaining_amount, loan_start_date, loan_end_date,
                       account_status, days_past_due, number_of_missed_payments,
                       created_at, updated_at
                FROM loan_accounts
                ORDER BY created_at DESC
                LIMIT 100
            )";
            
            pqxx::result result = txn.exec(sql);
            
            std::vector<LoanAccount> accounts;
            accounts.reserve(result.size());
            
            for (const auto& row : result)
            {
                accounts.push_back(mapRowToLoanAccount(row));
            }
            
            return accounts;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findAll: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

int LoanAccountRepository::count()
{
    if (_useMock) return 2;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> int {
            pqxx::result result = txn.exec("SELECT COUNT(*) FROM loan_accounts");
            return result[0][0].as<int>();
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in count: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

// ============================================================================
// Update specific fields
// ============================================================================

bool LoanAccountRepository::updateStatus(int accountId, sdrs::constants::AccountStatus status)
{
    if (_useMock) return true;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> bool {
            std::string sql = R"(
                UPDATE loan_accounts 
                SET account_status = $2::account_status_enum
                WHERE account_id = $1
            )";
            
            pqxx::result result = txn.exec_params(sql, accountId, LoanAccount::statusToString(status));
            return result.affected_rows() > 0;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in updateStatus: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

bool LoanAccountRepository::updateDaysPastDue(int accountId, int daysPastDue)
{
    if (_useMock) return true;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> bool {
            std::string sql = R"(
                UPDATE loan_accounts 
                SET days_past_due = $2
                WHERE account_id = $1
            )";
            
            pqxx::result result = txn.exec_params(sql, accountId, daysPastDue);
            return result.affected_rows() > 0;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in updateDaysPastDue: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

bool LoanAccountRepository::recordPayment(int accountId, double amount)
{
    if (_useMock) return true;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> bool {
            std::string sql = R"(
                UPDATE loan_accounts 
                SET remaining_amount = remaining_amount - $2,
                    account_status = CASE 
                        WHEN remaining_amount - $2 <= 0 THEN 'PaidOff'::account_status_enum
                        ELSE account_status
                    END
                WHERE account_id = $1 AND remaining_amount >= $2
            )";
            
            pqxx::result result = txn.exec_params(sql, accountId, amount);
            return result.affected_rows() > 0;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in recordPayment: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

// ============================================================================
// Helper: Map database row to LoanAccount object
// ============================================================================

LoanAccount LoanAccountRepository::mapRowToLoanAccount(const pqxx::row& row)
{
    int accountId = row["account_id"].as<int>();
    int borrowerId = row["borrower_id"].as<int>();
    double loanAmount = row["loan_amount"].as<double>();
    double interestRate = row["interest_rate"].as<double>();
    
    // Calculate term from dates (simplified)
    int termMonths = 12; // Default
    
    LoanAccount account(accountId, borrowerId, loanAmount, interestRate, termMonths);
    
    // Set remaining amount and other fields
    if (!row["remaining_amount"].is_null())
    {
        // Note: Need to use internal setter or reconstruct with correct amount
    }
    
    if (!row["account_status"].is_null())
    {
        std::string status = row["account_status"].as<std::string>();
        account.updateStatus(LoanAccount::stringToStatus(status));
    }
    
    if (!row["days_past_due"].is_null())
    {
        int dpd = row["days_past_due"].as<int>();
        for (int i = 0; i < dpd; ++i)
        {
            account.incrementDaysPastDue(1);
        }
    }
    
    return account;
}

// ============================================================================
// Mock implementations
// ============================================================================

LoanAccount LoanAccountRepository::createMock(const LoanAccount& account)
{
    sdrs::utils::Logger::Info("[MOCK] Creating loan account for borrower: " + std::to_string(account.getBorrowerId()));
    return LoanAccount(999, account.getBorrowerId(), 
                       account.getLoanAmount().getAmount(),
                       account.getInterestRate(), 12);
}

std::optional<LoanAccount> LoanAccountRepository::findByIdMock(int accountId)
{
    sdrs::utils::Logger::Info("[MOCK] Finding loan account by ID: " + std::to_string(accountId));
    
    if (accountId <= 0) return std::nullopt;
    
    return LoanAccount(accountId, 1, 10000000.0, 0.12, 12);
}

std::vector<LoanAccount> LoanAccountRepository::findByBorrowerIdMock(int borrowerId)
{
    sdrs::utils::Logger::Info("[MOCK] Finding loan accounts for borrower: " + std::to_string(borrowerId));
    
    std::vector<LoanAccount> accounts;
    accounts.emplace_back(1, borrowerId, 10000000.0, 0.12, 12);
    accounts.emplace_back(2, borrowerId, 5000000.0, 0.10, 6);
    return accounts;
}

std::vector<LoanAccount> LoanAccountRepository::findAllMock()
{
    sdrs::utils::Logger::Info("[MOCK] Finding all loan accounts");
    
    std::vector<LoanAccount> accounts;
    accounts.emplace_back(1, 1, 10000000.0, 0.12, 12);
    accounts.emplace_back(2, 2, 20000000.0, 0.15, 24);
    return accounts;
}

} // namespace sdrs::borrower
