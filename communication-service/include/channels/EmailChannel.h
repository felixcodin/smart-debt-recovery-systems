#ifndef EMAIL_CHANNEL_H
#define EMAIL_CHANNEL_H

#include "CommunicationChannel.h"

namespace sdrs::communication
{

class EmailChannel : public CommunicationChannel
{
public:
    std::string getName() const override;
    bool send(const std::string& recipient,const std::string& message) override;

};

} // namespace sdrs::communication

#endif
