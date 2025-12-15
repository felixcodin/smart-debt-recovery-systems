#ifndef VALIDATION_EXCEPTION_H
#define VALIDATION_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace sdrs::exceptions
{

// Error codes for validation failures
enum class ErrorCode
{
    ValidationFailed,  // General validation error
    MissingField,      // Required field is missing
    InvalidFormat      // Field format is incorrect (e.g., email, phone)
};

class ValidationException : public std::exception
{
private:
    std::string _field;
    ErrorCode _code;
    std::string _message;


public:
    // Constructor: creates validation exception with message, field name, and error code
    ValidationException(const std::string& message, const std::string& field = "", ErrorCode code = ErrorCode::ValidationFailed);

public:
    // Returns error message for display/logging
    const char* what() const noexcept override;
    
    // Returns the field name that failed validation
    const std::string& getField() const;
    
    // Returns the specific error code for this validation failure
    ErrorCode getCode() const;

};

}

#endif