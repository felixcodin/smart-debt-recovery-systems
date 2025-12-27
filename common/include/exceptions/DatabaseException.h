// DatabaseException.h - Exception for database operation failures

#ifndef DATABASE_EXCEPTION_H
#define DATABASE_EXCEPTION_H

#include <exception>
#include <string>
#include "../utils/Constants.h"

namespace sdrs::exceptions
{

class DatabaseException : public std::exception
{
private:
    sdrs::constants::DatabaseErrorCode _errorCode;
    std::string _message;

public:
    // Constructor: creates database exception with message and error code
    explicit DatabaseException(
        const std::string& message, 
        sdrs::constants::DatabaseErrorCode code = sdrs::constants::DatabaseErrorCode::Unknown
    );
    
    // Returns basic error message
    const char* what() const noexcept override;
    
    // Returns detailed message including error code name (for debugging/logging)
    std::string getDetail() const;
    
    // Returns the specific database error code
    sdrs::constants::DatabaseErrorCode getErrorCode() const;

};

}

#endif