#ifndef DATABASE_EXCEPTION_H
#define DATABASE_EXCEPTION_H

#include <exception>
#include <string>

namespace sdrs::exceptions
{

// Error codes for database operations
enum class DatabaseErrorCode
{
    ConnectionFailed,       // Cannot connect to database
    QueryFailed,           // SQL query execution failed
    RecordNotFound,        // SELECT returned no results
    ConstraintViolation,   // Generic database constraint violated
    UniqueViolation,       // UNIQUE constraint violated (e.g., duplicate email)
    ForeignKeyViolation,   // Foreign key constraint violated
    Unknown                // Unclassified database error
};

class DatabaseException : public std::exception
{
private:
    DatabaseErrorCode _errorCode;
    std::string _message;

public:
    // Constructor: creates database exception with message and error code
    explicit DatabaseException(const std::string& message, DatabaseErrorCode code = DatabaseErrorCode::Unknown);
    
    // Returns basic error message
    const char* what() const noexcept override;
    
    // Returns detailed message including error code name (for debugging/logging)
    std::string getDetail() const;
    
    // Returns the specific database error code
    DatabaseErrorCode getErrorCode() const;

};

}

#endif