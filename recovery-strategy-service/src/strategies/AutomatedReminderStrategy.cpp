// AutomatedReminderStrategy.cpp - Implementation

#include "../../include/strategies/AutomatedReminderStrategy.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <iostream>

using namespace sdrs::constants;
using namespace sdrs::exceptions;
using namespace sdrs::money;
using namespace sdrs::communication;

namespace sdrs::strategy
{

AutomatedReminderStrategy::AutomatedReminderStrategy(int accountId,
    int borrowerId,
    const Money& expectedAmount,
    std::shared_ptr<IPaymentChecker> paymentChecker,
    std::shared_ptr<ICommunicationService> channel,
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
        throw ValidationException("Account ID cannot be negative", "AutomatedStrategy", ValidationErrorCode::InvalidFormat);
    }

    if (borrowerId < 0)
    {
        throw ValidationException("Borrower ID cannot be negative", "AutomatedStrategy", ValidationErrorCode::InvalidFormat);
    }
    
    if (maxReminders < 0)
    {
        throw ValidationException("Max Reminders cannot be negative", "AutomatedStrategy", ValidationErrorCode::InvalidFormat);
    }

    if (intervalDays < 0)
    {
        throw ValidationException("Interval Days cannot be negative", "AutomatedStrategy", ValidationErrorCode::InvalidFormat);
    }

    if (!_paymentChecker)
    {
        throw ValidationException("Payment checker cannot be null", "PaymentChecker");
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

    throw ValidationException("Invalid strategy status", "AutomatedReminderStrategy");
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
