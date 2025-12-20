#include "../../include/channels/VoiceCallChannel.h"

#include <iostream>

namespace sdrs::communication
{

std::string VoiceCallChannel::getName() const
{
    return "voice";
}

bool VoiceCallChannel::send(const std::string& recipient,const std::string& message)
{
    std::cout << "[VOICE] Calling: " << recipient << " | Script: " << message << std::endl;
    return true;
}

}
