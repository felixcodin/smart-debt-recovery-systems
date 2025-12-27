// CommunicationManager.cpp - Implementation

#include "../../include/managers/CommunicationManager.h"
#include "../../../common/include/exceptions/ValidationException.h"

using namespace sdrs::exceptions;

namespace sdrs::communication
{

void CommunicationManager::registerChannel(std::shared_ptr<CommunicationChannel> channel)
{
    if (!channel)
    {
        throw ValidationException("Communication channel must not be null", "channel");
    }

    const std::string name = channel->getName();
    if (name.empty())
    {
        throw ValidationException("Communication channel name must not be empty", "channelName");
    }

    channels_[name] = channel;
}

bool CommunicationManager::sendMessage(int accountId,const std::string& message) const
{
    if (message.empty())
    {
        throw ValidationException("Message must not be empty","message");
    }
    const std::string recipient = resolveRecipient(accountId);
    const std::string channelName = resolveChannel(accountId);
    auto it = channels_.find(channelName);
    if (it == channels_.end())
    {
        throw ValidationException("Communication channel not found", "channelName");
    }
    return it->second->send(recipient, message);
}

std::string CommunicationManager::resolveRecipient(int accountId) const
{
    if (accountId <= 0)
    {
        throw ValidationException("Invalid accountId","accountId");
    }
    return "user_" + std::to_string(accountId);
}

std::string CommunicationManager::resolveChannel(int accountId) const
{
    return (accountId % 2 == 0) ? "email" : "sms";
}

}
