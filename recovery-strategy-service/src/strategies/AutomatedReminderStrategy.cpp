#include "../../include/strategies/AutomatedReminderStrategy.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <iostream>

namespace sdrs::strategy
{

AutomatedReminderStrategy::AutomatedReminderStrategy(int accountId,
    int borrowerId,
    const sdrs::money::Money& expectedAmount,
    std::shared_ptr<IPaymentChecker> paymentChecker,
    std::shared_ptr<sdrs::communication::ICommunicationService> channel,
    int maxReminders,
    int intervalDays)
    : RecoveryStrategy(accountId, borrowerId, expectedAmount),
    _paymentChecker(paymentChecker),
    _channel(channel),
    _maxReminders(maxReminders),
    _intervalDays(intervalDays)
{
    if (accountId < 0)
    {
        throw sdrs::exceptions::ValidationException("Account ID cannot be negative", "AutomatedStrategy", sdrs::exceptions::ErrorCode::InvalidFormat);
    }

    if (borrowerId < 0)
    {
        throw sdrs::exceptions::ValidationException("Borrower ID cannot be negative", "AutomatedStrategy", sdrs::exceptions::ErrorCode::InvalidFormat);
    }
    
    if (maxReminders < 0)
    {
        throw sdrs::exceptions::ValidationException("Max Reminders cannot be negative", "AutomatedStrategy", sdrs::exceptions::ErrorCode::InvalidFormat);
    }

    if (intervalDays < 0)
    {
        throw sdrs::exceptions::ValidationException("Interval Days cannot be negative", "AutomatedStrategy", sdrs::exceptions::ErrorCode::InvalidFormat);
    }

    if (!_paymentChecker)
    {
        throw sdrs::exceptions::ValidationException("Payment checker cannot be null", "PaymentChecker");
    }
}

StrategyType AutomatedReminderStrategy::getType() const
{
    return StrategyType::AutomatedReminder;
}

StrategyStatus AutomatedReminderStrategy::execute()
{
    if (!canExecute())
    {
        return _status;
    }

    _status = StrategyStatus::Active;

    while (_attemptCount < _maxReminders)
    {
        if (checkPaymentReceived())
        {
            _status = StrategyStatus::Completed;
            return _status;
        }

        sendReminder();
        _attemptCount++;
    }
    
    if (checkPaymentReceived())
    {
        _status = StrategyStatus::Completed;
    }
    else
    {
        _status = StrategyStatus::Failed;
    }

    return _status;
}

void AutomatedReminderStrategy::sendReminder()
{
    _channel->sendMessage(_accountId, "Please settle your outstanding balance!");
}

[[nodiscard]]
std::string AutomatedReminderStrategy::statusToString() const
{
    switch (_status)
    {
    case StrategyStatus::Active:
        return "Active";
    case StrategyStatus::Cancelled:
        return "Cancelled";
    case StrategyStatus::Completed:
        return "Completed";
    case StrategyStatus::Failed:
        return "Failed";
    case StrategyStatus::Pending:
        return "Pending";
    }

    throw sdrs::exceptions::ValidationException("Invalid strategy status", "AutomatedReminderStrategy");
}

std::string AutomatedReminderStrategy::toJson() const
{
    return "{\"type\":\"AutomatedReminder\",\"accountId\":" 
           + std::to_string(_accountId) 
           + ",\"status\":\"" + statusToString() + "\"}";
}

bool AutomatedReminderStrategy::checkPaymentReceived() const
{
    if (!_paymentChecker)
    {
        return false;
    }
    return _paymentChecker->hasPaymentReceived(_accountId);
}

} // namespace sdrs::strategy
