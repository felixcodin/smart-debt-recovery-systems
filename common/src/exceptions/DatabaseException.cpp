#include "../../include/exceptions/DatabaseException.h"

namespace sdrs::exceptions
{

DatabaseException::DatabaseException(const std::string& message,
    DatabaseErrorCode code)
    : _message(message),
    _errorCode(code)
{
    // Do nothing
}

const char* DatabaseException::what() const noexcept
{
    return _message.c_str();
}

std::string DatabaseException::getDetail() const
{
    std::string detail = "Database error [";

    switch (_errorCode)
    {
    case DatabaseErrorCode::ConnectionFailed:
        detail += "ConnectionFailed";
        break;
    case DatabaseErrorCode::ConstraintViolation:
        detail += "ConstraintViolation";
        break;
    case DatabaseErrorCode::QueryFailed:
        detail += "QueryFailed";
        break;
    case DatabaseErrorCode::RecordNotFound:
        detail += "RecordNotFound";
        break;
    case DatabaseErrorCode::ForeignKeyViolation:
        detail += "ForeignKeyViolation";
        break;
    case DatabaseErrorCode::UniqueViolation:
        detail += "UniqueViolation";
        break;
    default:
        detail += "Unknown";
        break;
    }

    detail += "]: " + _message;
    return detail;
}

DatabaseErrorCode DatabaseException::getErrorCode() const
{
    return _errorCode;
}

}