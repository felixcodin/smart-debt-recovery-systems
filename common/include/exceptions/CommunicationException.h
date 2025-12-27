// CommunicationException.h - Exception for communication channel failures

#ifndef COMMUNICATION_EXCEPTION_H
#define COMMUNICATION_EXCEPTION_H

#include <exception>
#include <string>
#include "../utils/Constants.h"

namespace sdrs::exceptions
{

class CommunicationException : public std::exception
{
private:
    std::string _message;
    sdrs::constants::ChannelType _channel;
    sdrs::constants::CommunicationErrorCode _errorCode;

public:
    // Constructor: creates communication exception with message, channel, and error code
    CommunicationException(
        const std::string& message,
        sdrs::constants::ChannelType channel,
        sdrs::constants::CommunicationErrorCode code = sdrs::constants::CommunicationErrorCode::Unknown
    );
    
    // Returns basic error message
    const char* what() const noexcept override;
    
    // Returns the communication channel where error occurred
    sdrs::constants::ChannelType getChannel() const;
    
    // Returns the specific communication error code
    sdrs::constants::CommunicationErrorCode getErrorCode() const;
    
    // Returns detailed message including error code and channel (for debugging/logging)
    std::string getDetail() const;

};

}

#endif