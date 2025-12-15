#ifndef COMMUNICATION_EXCEPTION_H
#define COMMUNICATION_EXCEPTION_H

#include <exception>
#include <string>

namespace sdrs::exceptions
{

// Error codes for communication operations
enum class CommunicationErrorCode
{
    SendFailed,          // Failed to send message (network/service error)
    InvalidRecipient,    // Invalid email/phone format
    ChannelUnavailable,  // Communication channel temporarily unavailable
    RateLimitExceeded,   // Too many messages sent in short period
    Unknown              // Unclassified communication error
};

// Communication channels available in the system
enum class ChannelType
{
    Email,      // Email channel (SMTP)
    SMS,        // SMS channel (SMS gateway)
    VoiceCall   // Voice call channel (maps to "Phone" in database)
};

class CommunicationException : public std::exception
{
private:
    std::string _message;
    ChannelType _channel;
    CommunicationErrorCode _errorCode;

public:
    // Constructor: creates communication exception with message, channel, and error code
    CommunicationException(const std::string& message, ChannelType channel, CommunicationErrorCode code = CommunicationErrorCode::Unknown);
    
    // Returns basic error message
    const char* what() const noexcept override;
    
    // Returns the communication channel where error occurred
    ChannelType getChannel() const;
    
    // Returns the specific communication error code
    CommunicationErrorCode getErrorCode() const;
    
    // Returns detailed message including error code and channel (for debugging/logging)
    std::string getDetail() const;

};

}

#endif