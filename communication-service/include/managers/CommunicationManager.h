// CommunicationManager.h - Manages multiple communication channels

#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include "../interfaces/ICommunicationService.h"
#include "../channels/CommunicationChannel.h"

#include <memory>
#include <unordered_map>

namespace sdrs::communication
{

// Routes messages to appropriate channel based on account preferences
class CommunicationManager : public ICommunicationService
{
private:
    std::unordered_map<std::string,std::shared_ptr<CommunicationChannel>> channels_;  // name -> channel
    std::string resolveRecipient(int accountId) const;  // looks up contact info
    std::string resolveChannel(int accountId) const;    // determines preferred channel
    
public:
    void registerChannel(std::shared_ptr<CommunicationChannel> channel);  // add channel to registry
    bool sendMessage(int accountId,const std::string& message) const override;  // sends via appropriate channel
};

} // namespace sdrs::communication
#endif
