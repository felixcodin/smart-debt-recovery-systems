// BorrowerRepository.cpp - PostgreSQL implementation

#include "../../include/repositories/BorrowerRepository.h"
#include "../../../common/include/utils/Logger.h"
#include "../../../common/include/utils/Constants.h"
#include "../../../common/include/exceptions/DatabaseException.h"

namespace sdrs::borrower
{

BorrowerRepository::BorrowerRepository(bool useMock)
    : _useMock(useMock)
{
}

// ============================================================================
// CRUD Operations - Delegate to mock or real implementation
// ============================================================================

Borrower BorrowerRepository::create(const Borrower& borrower)
{
    if (_useMock) return createMock(borrower);
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> Borrower {
            std::string sql = R"(
                INSERT INTO borrowers (
                    first_name, last_name, email, phone_number,
                    date_of_birth, address,
                    monthly_income, employment_status, is_active
                ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)
                RETURNING borrower_id, created_at, updated_at
            )";
            
            pqxx::result result = txn.exec_params(sql,
                borrower.getFirstName(),
                borrower.getLastName(),
                borrower.getEmail(),
                borrower.getPhoneNumber(),
                borrower.getDateOfBirthString(),
                borrower.getAddress(),
                borrower.getMonthlyIncome(),
                sdrs::constants::employmentStatusToString(borrower.getEmploymentStatus()),
                borrower.isActive()
            );
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException("Failed to insert borrower", sdrs::constants::DatabaseErrorCode::QueryFailed);
            }
            
            int newId = result[0]["borrower_id"].as<int>();
            
            Borrower created(newId, borrower.getFirstName(), borrower.getLastName());
            created.setEmail(borrower.getEmail());
            created.setPhoneNumber(borrower.getPhoneNumber());
            created.setAddress(borrower.getAddress());
            created.setMonthlyIncome(borrower.getMonthlyIncome());
            created.setEmploymentStatus(borrower.getEmploymentStatus());
            if (borrower.isActive()) created.setActive();
            
            sdrs::utils::Logger::Info("[DB] Created borrower ID: " + std::to_string(newId));
            return created;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in create: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::optional<Borrower> BorrowerRepository::findById(int id)
{
    if (_useMock) return findByIdMock(id);
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::optional<Borrower> {
            std::string sql = R"(
                SELECT borrower_id, first_name, last_name, email, phone_number,
                       date_of_birth, address,
                       monthly_income, employment_status, is_active, inactive_reason,
                       created_at, updated_at
                FROM borrowers
                WHERE borrower_id = $1
            )";
            
            pqxx::result result = txn.exec_params(sql, id);
            
            if (result.empty())
            {
                return std::nullopt;
            }
            
            return mapRowToBorrower(result[0]);
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findById: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

Borrower BorrowerRepository::update(const Borrower& borrower)
{
    if (_useMock) return updateMock(borrower);
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> Borrower {
            std::string sql = R"(
                UPDATE borrowers SET
                    first_name = $2,
                    last_name = $3,
                    email = $4,
                    phone_number = $5,
                    address = $6,
                    monthly_income = $7,
                    employment_status = $8::employment_status_enum,
                    is_active = $9
                WHERE borrower_id = $1
                RETURNING borrower_id
            )";
            
            pqxx::result result = txn.exec_params(sql,
                borrower.getId(),
                borrower.getFirstName(),
                borrower.getLastName(),
                borrower.getEmail(),
                borrower.getPhoneNumber(),
                borrower.getAddress(),
                borrower.getMonthlyIncome(),
                sdrs::constants::employmentStatusToString(borrower.getEmploymentStatus()),
                borrower.isActive()
            );
            
            if (result.empty())
            {
                throw sdrs::exceptions::DatabaseException(
                    "Borrower not found with ID: " + std::to_string(borrower.getId()), sdrs::constants::DatabaseErrorCode::QueryFailed);
            }
            
            sdrs::utils::Logger::Info("[DB] Updated borrower ID: " + std::to_string(borrower.getId()));
            return borrower;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in update: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

bool BorrowerRepository::deleteById(int id)
{
    if (_useMock) return deleteByIdMock(id);
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> bool {
            std::string sql = "DELETE FROM borrowers WHERE borrower_id = $1";
            pqxx::result result = txn.exec_params(sql, id);
            
            bool deleted = result.affected_rows() > 0;
            if (deleted)
            {
                sdrs::utils::Logger::Info("[DB] Deleted borrower ID: " + std::to_string(id));
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

std::vector<Borrower> BorrowerRepository::findAll()
{
    if (_useMock) return findAllMock();
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<Borrower> {
            std::string sql = R"(
                SELECT borrower_id, first_name, last_name, email, phone_number,
                       date_of_birth, address,
                       monthly_income, employment_status, is_active, inactive_reason,
                       created_at, updated_at
                FROM borrowers
                ORDER BY created_at DESC
                LIMIT 100
            )";
            
            pqxx::result result = txn.exec(sql);
            
            std::vector<Borrower> borrowers;
            borrowers.reserve(result.size());
            
            for (const auto& row : result)
            {
                borrowers.push_back(mapRowToBorrower(row));
            }
            
            sdrs::utils::Logger::Info("[DB] Found " + std::to_string(borrowers.size()) + " borrowers");
            return borrowers;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findAll: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

// ============================================================================
// Additional Queries
// ============================================================================

std::optional<Borrower> BorrowerRepository::findByEmail(const std::string& email)
{
    if (_useMock) return std::nullopt;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::optional<Borrower> {
            std::string sql = R"(
                SELECT borrower_id, first_name, last_name, email, phone_number,
                       date_of_birth, address,
                       monthly_income, employment_status, is_active, inactive_reason,
                       created_at, updated_at
                FROM borrowers
                WHERE email = $1
            )";
            
            pqxx::result result = txn.exec_params(sql, email);
            
            if (result.empty())
            {
                return std::nullopt;
            }
            
            return mapRowToBorrower(result[0]);
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findByEmail: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

std::vector<Borrower> BorrowerRepository::findByActiveStatus(bool isActive)
{
    if (_useMock) return findAllMock();
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> std::vector<Borrower> {
            std::string sql = R"(
                SELECT borrower_id, first_name, last_name, email, phone_number,
                       date_of_birth, address,
                       monthly_income, employment_status, is_active, inactive_reason,
                       created_at, updated_at
                FROM borrowers
                WHERE is_active = $1
                ORDER BY created_at DESC
            )";
            
            pqxx::result result = txn.exec_params(sql, isActive);
            
            std::vector<Borrower> borrowers;
            borrowers.reserve(result.size());
            
            for (const auto& row : result)
            {
                borrowers.push_back(mapRowToBorrower(row));
            }
            
            return borrowers;
        });
    }
    catch (const pqxx::sql_error& e)
    {
        sdrs::utils::Logger::Error("[DB] SQL error in findByActiveStatus: " + std::string(e.what()));
        throw sdrs::exceptions::DatabaseException(e.what(), sdrs::constants::DatabaseErrorCode::QueryFailed);
    }
}

int BorrowerRepository::count()
{
    if (_useMock) return 2;
    
    try
    {
        auto& db = sdrs::database::DatabaseManager::getInstance();
        
        return db.executeQuery([&](pqxx::work& txn) -> int {
            pqxx::result result = txn.exec("SELECT COUNT(*) FROM borrowers");
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
// Helper: Map database row to Borrower object
// ============================================================================

Borrower BorrowerRepository::mapRowToBorrower(const pqxx::row& row)
{
    int id = row["borrower_id"].as<int>();
    std::string firstName = row["first_name"].as<std::string>();
    std::string lastName = row["last_name"].as<std::string>();
    
    Borrower borrower(id, firstName, lastName);
    
    if (!row["email"].is_null())
        borrower.setEmail(row["email"].as<std::string>());
    
    if (!row["phone_number"].is_null())
        borrower.setPhoneNumber(row["phone_number"].as<std::string>());
    
    if (!row["address"].is_null())
        borrower.setAddress(row["address"].as<std::string>());
    
    if (!row["monthly_income"].is_null())
        borrower.setMonthlyIncome(row["monthly_income"].as<double>());
    
    if (!row["employment_status"].is_null())
    {
        std::string status = row["employment_status"].as<std::string>();
        borrower.setEmploymentStatus(sdrs::constants::stringToEmploymentStatus(status));
    }
    
    if (row["is_active"].as<bool>())
        borrower.setActive();
    else
        borrower.setInactive(sdrs::constants::InactiveReason::AccountClosed);
    
    return borrower;
}

// ============================================================================
// Mock implementations for testing without database
// ============================================================================

Borrower BorrowerRepository::createMock(const Borrower& borrower)
{
    sdrs::utils::Logger::Info("[MOCK] Creating borrower: " + borrower.getFirstName() + " " + borrower.getLastName());
    
    Borrower created(999, borrower.getFirstName(), borrower.getLastName());
    created.setEmail(borrower.getEmail());
    created.setPhoneNumber(borrower.getPhoneNumber());
    created.setAddress(borrower.getAddress());
    created.setActive();
    
    return created;
}

std::optional<Borrower> BorrowerRepository::findByIdMock(int id)
{
    sdrs::utils::Logger::Info("[MOCK] Finding borrower by ID: " + std::to_string(id));
    
    if (id <= 0)
    {
        return std::nullopt;
    }
    
    Borrower borrower(id, "John", "Doe");
    borrower.setEmail("john.doe@example.com");
    borrower.setPhoneNumber("0123456789");
    borrower.setAddress("123 Main St, Hanoi");
    borrower.setMonthlyIncome(15000000.0);
    borrower.setActive();
    
    return borrower;
}

Borrower BorrowerRepository::updateMock(const Borrower& borrower)
{
    sdrs::utils::Logger::Info("[MOCK] Updating borrower ID: " + std::to_string(borrower.getId()));
    return borrower;
}

bool BorrowerRepository::deleteByIdMock(int id)
{
    sdrs::utils::Logger::Info("[MOCK] Deleting borrower ID: " + std::to_string(id));
    return id > 0;
}

std::vector<Borrower> BorrowerRepository::findAllMock()
{
    sdrs::utils::Logger::Info("[MOCK] Finding all borrowers");
    
    std::vector<Borrower> borrowers;
    
    Borrower b1(1, "Alice", "Smith");
    b1.setEmail("alice.smith@example.com");
    b1.setActive();
    borrowers.push_back(b1);
    
    Borrower b2(2, "Bob", "Johnson");
    b2.setEmail("bob.johnson@example.com");
    b2.setActive();
    borrowers.push_back(b2);
    
    return borrowers;
}

} // namespace sdrs::borrower
