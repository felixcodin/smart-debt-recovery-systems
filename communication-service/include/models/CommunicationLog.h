#pragma once

#include <string>
#include <chrono>
#include <optional>

namespace sdrs::communication
{

enum class ChannelType
{
    Email,
    SMS,
    VoiceCall,
    PushNotification,
    Letter
};

enum class MessageStatus
{
    Pending,
    Sent,
    Delivered,
    Failed,
    Bounced
};

class CommunicationLog
{
private:
    int _communicationId;
    int _accountId;
    int _borrowerId;
    std::optional<int> _strategyId;
    ChannelType _channelType;
    std::string _messageContent;
    MessageStatus _messageStatus;
    std::optional<std::chrono::sys_seconds> _sentAt;
    std::optional<std::chrono::sys_seconds> _deliveredAt;
    std::optional<std::string> _errorMessage;
    std::chrono::sys_seconds _createdAt;
    std::chrono::sys_seconds _updatedAt;

public:
    CommunicationLog() = delete;
    
    // Constructor for creating new log
    CommunicationLog(
        int accountId,
        int borrowerId,
        ChannelType channelType,
        const std::string& messageContent
    );
    
    // Constructor for loading from database
    CommunicationLog(
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
    );

    // Getters
    int getCommunicationId() const;
    int getAccountId() const;
    int getBorrowerId() const;
    std::optional<int> getStrategyId() const;
    ChannelType getChannelType() const;
    std::string getMessageContent() const;
    MessageStatus getMessageStatus() const;
    std::optional<std::chrono::sys_seconds> getSentAt() const;
    std::optional<std::chrono::sys_seconds> getDeliveredAt() const;
    std::optional<std::string> getErrorMessage() const;
    std::chrono::sys_seconds getCreatedAt() const;
    std::chrono::sys_seconds getUpdatedAt() const;

    // Setters
    void setStrategyId(int strategyId);
    void setMessageStatus(MessageStatus status);
    void setSentAt(std::chrono::sys_seconds sentAt);
    void setDeliveredAt(std::chrono::sys_seconds deliveredAt);
    void setErrorMessage(const std::string& errorMessage);

    // Conversion methods
    std::string toJson() const;
    static CommunicationLog fromJson(const std::string& json);
    
    // Enum converters
    static std::string channelTypeToString(ChannelType type);
    static ChannelType stringToChannelType(const std::string& typeStr);
    static std::string messageStatusToString(MessageStatus status);
    static MessageStatus stringToMessageStatus(const std::string& statusStr);

private:
    void touch(); // Update updatedAt timestamp
};

} // namespace sdrs::communication
