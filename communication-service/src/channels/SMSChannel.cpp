#include "../../include/channels/SMSChannel.h"

#include <iostream>

namespace sdrs::communication
{

std::string SMSChannel::getName() const
{
    return "sms";
}
bool SMSChannel::send(const std::string& recipient,const std::string& message)
{
    std::cout << "[SMS] To: " << recipient << " | Message: " << message << std::endl;
    return true;
}

} 
