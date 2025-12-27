// EmailChannel.cpp - Implementation

#include "../../include/channels/EmailChannel.h"
#include <iostream>

namespace sdrs::communication
{

std::string EmailChannel::getName() const
{
    return "email";
}
bool EmailChannel::send(const std::string& recipient,const std::string& message)
{
    std::cout << "[EMAIL] To: " << recipient << " | Message: " << message << std::endl;
    return true;
}
}