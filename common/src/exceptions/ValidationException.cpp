#include "../../include/exceptions/ValidationException.h"

namespace sdrs::exceptions
{

ValidationException::ValidationException(const std::string& message,
    const std::string& field,
    ErrorCode code)
    : _field(field),
      _code(code),
      _message("Validation error [" + field + "]: " + message)
{
    // Do nothing
}

const char* ValidationException::what() const noexcept
{
    return _message.c_str();
}

const std::string& ValidationException::getField() const
{
    return _field;
}

ErrorCode ValidationException::getCode() const
{
    return _code;
}

}