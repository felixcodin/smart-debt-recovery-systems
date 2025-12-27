// ValidationException.h - Exception for input validation failures

#ifndef VALIDATION_EXCEPTION_H
#define VALIDATION_EXCEPTION_H

#include <stdexcept>
#include <string>
#include "../utils/Constants.h"

namespace sdrs::exceptions
{

class ValidationException : public std::exception
{
private:
    std::string _field;
    sdrs::constants::ValidationErrorCode _code;
    std::string _message;


public:
    // Constructor: creates validation exception with message, field name, and error code
    ValidationException(
        const std::string& message, 
        const std::string& field = "", 
        sdrs::constants::ValidationErrorCode code = sdrs::constants::ValidationErrorCode::ValidationFailed
    );

public:
    // Returns error message for display/logging
    const char* what() const noexcept override;
    
    // Returns the field name that failed validation
    const std::string& getField() const;
    
    // Returns the specific error code for this validation failure
    sdrs::constants::ValidationErrorCode getCode() const;

};

}

#endif