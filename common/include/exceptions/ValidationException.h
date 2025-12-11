#ifndef VALIDATION_EXCEPTION_H
#define VALIDATION_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace sdrs::exceptions
{

enum class ErrorCode
{
    ValidationFailed,
    MissingField,
    InvalidFormat
};

class ValidationException : public std::exception
{
private:
    std::string _field;
    ErrorCode _code;
    std::string _message;


public:
    ValidationException(const std::string& message, const std::string& field = "", ErrorCode code = ErrorCode::ValidationFailed);

public:
    const char* what() const noexcept override;
    const std::string& getField() const;
    ErrorCode getCode() const;

};

}

#endif