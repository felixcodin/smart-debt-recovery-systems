#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include "../interfaces/ICommunicationService.h"
#include "../channels/CommunicationChannel.h"

#include <memory>
#include <unordered_map>

namespace sdrs::communication
{

class CommunicationManager : public ICommunicationService
{
private:
    std::unordered_map<std::string,std::shared_ptr<CommunicationChannel>> channels_;
    std::string resolveRecipient(int accountId) const;
    std::string resolveChannel(int accountId) const;
public:
    void registerChannel(std::shared_ptr<CommunicationChannel> channel);
    bool sendMessage(int accountId,const std::string& message) const override;


};

} // namespace sdrs::communication
#endif
