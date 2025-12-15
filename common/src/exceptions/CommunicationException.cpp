#include "../../include/exceptions/CommunicationException.h"

namespace sdrs::exceptions
{

CommunicationException::CommunicationException(const std::string& message,
    ChannelType channel,
    CommunicationErrorCode code)
    : _message(message),
    _channel(channel),
    _errorCode(code)
{
    // Do nothing
}

const char* CommunicationException::what() const noexcept
{
    return _message.c_str();
}

ChannelType CommunicationException::getChannel() const
{
    return _channel;
}

CommunicationErrorCode CommunicationException::getErrorCode() const
{
    return _errorCode;
}

std::string CommunicationException::getDetail() const
{
    std::string detail = "Communication error [";

    switch (_errorCode)
    {
    case CommunicationErrorCode::SendFailed:
        detail += "SendFailed";
        break;
    case CommunicationErrorCode::InvalidRecipient:
        detail += "InvalidRecipient";
        break;
    case CommunicationErrorCode::ChannelUnavailable:
        detail += "ChannelUnavailable";
        break;
    case CommunicationErrorCode::RateLimitExceeded:
        detail += "RateLimitExceeded";
        break;
    default:
        detail += "Unknown";
        break;
    }

    detail += "] on channel [";

    switch (_channel)
    {
    case ChannelType::SMS:
        detail += "SMS";
        break;
    case ChannelType::Email:
        detail += "Email";
        break;
    case ChannelType::VoiceCall:
        detail += "VoiceCall";
        break;
    default:
        break;
    }

    detail += "]: " + _message;

    return detail;
}

}