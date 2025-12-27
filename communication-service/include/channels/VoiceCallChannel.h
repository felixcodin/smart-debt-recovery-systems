// VoiceCallChannel.h - Voice call communication channel implementation

#ifndef VOICE_CALL_CHANNEL_H
#define VOICE_CALL_CHANNEL_H

#include "CommunicationChannel.h"

namespace sdrs::communication
{

class VoiceCallChannel : public CommunicationChannel
{
public:
    std::string getName() const override;
    bool send(const std::string& recipient,const std::string& message) override;

};

} // namespace sdrs::communication
#endif
