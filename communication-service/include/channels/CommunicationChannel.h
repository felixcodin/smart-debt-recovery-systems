#ifndef COMMUNICATION_CHANNEL_H
#define COMMUNICATION_CHANNEL_H

#include <string>

namespace sdrs::communication
{ 

class CommunicationChannel
{
public:
    virtual ~CommunicationChannel() = default;
    virtual std::string getName() const = 0;
    virtual bool send(const std::string& recipient, const std::string& message) = 0;

};

} // namespace sdrs::communication

#endif // COMMUNICATION_CHANNEL_H
