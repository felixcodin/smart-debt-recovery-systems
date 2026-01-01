#include "../../include/models/CommunicationLog.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <ctime>

using json = nlohmann::json;

namespace sdrs::communication
{

// Constructor for creating new log
CommunicationLog::CommunicationLog(
    int accountId,
    int borrowerId,
    ChannelType channelType,
    const std::string& messageContent
)
    : _communicationId(0),
      _accountId(accountId),
      _borrowerId(borrowerId),
      _strategyId(std::nullopt),
      _channelType(channelType),
      _messageContent(messageContent),
      _messageStatus(MessageStatus::Pending),
      _sentAt(std::nullopt),
      _deliveredAt(std::nullopt),
      _errorMessage(std::nullopt),
      _createdAt(std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now())),
      _updatedAt(std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()))
{
}

// Constructor for loading from database
CommunicationLog::CommunicationLog(
    int communicationId,
    int accountId,
    int borrowerId,
    std::optional<int> strategyId,
    ChannelType channelType,
    const std::string& messageContent,
    MessageStatus messageStatus,
    std::optional<std::chrono::sys_seconds> sentAt,
    std::optional<std::chrono::sys_seconds> deliveredAt,
    std::optional<std::string> errorMessage,
    std::chrono::sys_seconds createdAt,
    std::chrono::sys_seconds updatedAt
)
    : _communicationId(communicationId),
      _accountId(accountId),
      _borrowerId(borrowerId),
      _strategyId(strategyId),
      _channelType(channelType),
      _messageContent(messageContent),
      _messageStatus(messageStatus),
      _sentAt(sentAt),
      _deliveredAt(deliveredAt),
      _errorMessage(errorMessage),
      _createdAt(createdAt),
      _updatedAt(updatedAt)
{
}

// Getters
int CommunicationLog::getCommunicationId() const { return _communicationId; }
int CommunicationLog::getAccountId() const { return _accountId; }
int CommunicationLog::getBorrowerId() const { return _borrowerId; }
std::optional<int> CommunicationLog::getStrategyId() const { return _strategyId; }
ChannelType CommunicationLog::getChannelType() const { return _channelType; }
std::string CommunicationLog::getMessageContent() const { return _messageContent; }
MessageStatus CommunicationLog::getMessageStatus() const { return _messageStatus; }
std::optional<std::chrono::sys_seconds> CommunicationLog::getSentAt() const { return _sentAt; }
std::optional<std::chrono::sys_seconds> CommunicationLog::getDeliveredAt() const { return _deliveredAt; }
std::optional<std::string> CommunicationLog::getErrorMessage() const { return _errorMessage; }
std::chrono::sys_seconds CommunicationLog::getCreatedAt() const { return _createdAt; }
std::chrono::sys_seconds CommunicationLog::getUpdatedAt() const { return _updatedAt; }

// Setters
void CommunicationLog::setStrategyId(int strategyId)
{
    _strategyId = strategyId;
    touch();
}

void CommunicationLog::setMessageStatus(MessageStatus status)
{
    _messageStatus = status;
    touch();
}

void CommunicationLog::setSentAt(std::chrono::sys_seconds sentAt)
{
    _sentAt = sentAt;
    touch();
}

void CommunicationLog::setDeliveredAt(std::chrono::sys_seconds deliveredAt)
{
    _deliveredAt = deliveredAt;
    touch();
}

void CommunicationLog::setErrorMessage(const std::string& errorMessage)
{
    _errorMessage = errorMessage;
    touch();
}

void CommunicationLog::touch()
{
    _updatedAt = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
}

// Enum converters
std::string CommunicationLog::channelTypeToString(ChannelType type)
{
    switch (type)
    {
        case ChannelType::Email: return "Email";
        case ChannelType::SMS: return "SMS";
        case ChannelType::VoiceCall: return "Phone";
        case ChannelType::PushNotification: return "PushNotification";
        case ChannelType::Letter: return "Letter";
        default: throw std::invalid_argument("Unknown ChannelType");
    }
}

ChannelType CommunicationLog::stringToChannelType(const std::string& typeStr)
{
    if (typeStr == "Email") return ChannelType::Email;
    if (typeStr == "SMS") return ChannelType::SMS;
    if (typeStr == "Phone" || typeStr == "VoiceCall") return ChannelType::VoiceCall;
    if (typeStr == "PushNotification") return ChannelType::PushNotification;
    if (typeStr == "Letter") return ChannelType::Letter;
    throw std::invalid_argument("Invalid ChannelType string: " + typeStr);
}

std::string CommunicationLog::messageStatusToString(MessageStatus status)
{
    switch (status)
    {
        case MessageStatus::Pending: return "Pending";
        case MessageStatus::Sent: return "Sent";
        case MessageStatus::Delivered: return "Delivered";
        case MessageStatus::Failed: return "Failed";
        case MessageStatus::Bounced: return "Bounced";
        default: throw std::invalid_argument("Unknown MessageStatus");
    }
}

MessageStatus CommunicationLog::stringToMessageStatus(const std::string& statusStr)
{
    if (statusStr == "Pending") return MessageStatus::Pending;
    if (statusStr == "Sent") return MessageStatus::Sent;
    if (statusStr == "Delivered") return MessageStatus::Delivered;
    if (statusStr == "Failed") return MessageStatus::Failed;
    if (statusStr == "Bounced" || statusStr == "Opened") return MessageStatus::Bounced;
    throw std::invalid_argument("Invalid MessageStatus string: " + statusStr);
}

// JSON conversion
std::string CommunicationLog::toJson() const
{
    json j;
    j["communication_id"] = _communicationId;
    j["account_id"] = _accountId;
    j["borrower_id"] = _borrowerId;
    
    if (_strategyId.has_value())
        j["strategy_id"] = _strategyId.value();
    else
        j["strategy_id"] = nullptr;
    
    j["channel_type"] = channelTypeToString(_channelType);
    j["message_content"] = _messageContent;
    j["message_status"] = messageStatusToString(_messageStatus);
    
    if (_sentAt.has_value()) {
        auto tp = std::chrono::system_clock::to_time_t(_sentAt.value());
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&tp));
        j["sent_at"] = std::string(buffer);
    } else {
        j["sent_at"] = nullptr;
    }
    
    if (_deliveredAt.has_value()) {
        auto tp = std::chrono::system_clock::to_time_t(_deliveredAt.value());
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&tp));
        j["delivered_at"] = std::string(buffer);
    } else {
        j["delivered_at"] = nullptr;
    }
    
    if (_errorMessage.has_value())
        j["error_message"] = _errorMessage.value();
    else
        j["error_message"] = nullptr;
    
    auto createdTp = std::chrono::system_clock::to_time_t(_createdAt);
    char createdBuffer[32];
    std::strftime(createdBuffer, sizeof(createdBuffer), "%Y-%m-%d %H:%M:%S", std::localtime(&createdTp));
    j["created_at"] = std::string(createdBuffer);
    
    auto updatedTp = std::chrono::system_clock::to_time_t(_updatedAt);
    char updatedBuffer[32];
    std::strftime(updatedBuffer, sizeof(updatedBuffer), "%Y-%m-%d %H:%M:%S", std::localtime(&updatedTp));
    j["updated_at"] = std::string(updatedBuffer);
    
    return j.dump();
}

CommunicationLog CommunicationLog::fromJson(const std::string& json_str)
{
    json j = json::parse(json_str);
    
    int accountId = j.at("account_id").get<int>();
    int borrowerId = j.at("borrower_id").get<int>();
    ChannelType channelType = stringToChannelType(j.at("channel_type").get<std::string>());
    std::string messageContent = j.at("message_content").get<std::string>();
    
    CommunicationLog log(accountId, borrowerId, channelType, messageContent);
    
    if (j.contains("strategy_id") && !j["strategy_id"].is_null()) {
        log.setStrategyId(j["strategy_id"].get<int>());
    }
    
    if (j.contains("message_status")) {
        log.setMessageStatus(stringToMessageStatus(j["message_status"].get<std::string>()));
    }
    
    return log;
}

} // namespace sdrs::communication
