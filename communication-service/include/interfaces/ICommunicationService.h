#ifndef INTERFACE_COMMUNICATION_SERVICE_H
#define INTERFACE_COMMUNICATION_SERVICE_H

#include <string>

namespace sdrs::communication
{
    
class ICommunicationService
{
public:
    virtual ~ICommunicationService() = default;
    virtual bool sendMessage(int accountId, const std::string& message) const = 0;

};

} // namespace sdrs::communication


#endif