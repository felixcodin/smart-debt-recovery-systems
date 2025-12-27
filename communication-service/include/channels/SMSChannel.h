// SMSChannel.h - SMS communication channel implementation

#ifndef SMS_CHANNEL_H
#define SMS_CHANNEL_H

#include "CommunicationChannel.h"

namespace sdrs::communication
{
    
class SMSChannel : public CommunicationChannel
{
public:
    std::string getName() const override;
    bool send(const std::string& recipient,const std::string& message) override;

};

} // namespace sdrs::communication

#endif
