// CommunicationChannel.h - Abstract base class for communication channels (Polymorphism)

#ifndef COMMUNICATION_CHANNEL_H
#define COMMUNICATION_CHANNEL_H

#include <string>
#include "../interfaces/ICommunicationService.h"

namespace sdrs::communication
{

// Base class for all channels (Email, SMS, Voice) - subclasses implement send()
class CommunicationChannel : public ICommunicationService
{
public:
    virtual ~CommunicationChannel() = default;
    virtual std::string getName() const = 0;  // "email", "sms", "voice"
    virtual bool send(const std::string& recipient, const std::string& message) = 0;
    
    // Adapter: converts accountId to recipient string
    bool sendMessage(int accountId, const std::string& message) const override
    {
        std::string recipient = std::to_string(accountId);
        return const_cast<CommunicationChannel*>(this)->send(recipient, message);
    }
};

} // namespace sdrs::communication

#endif
