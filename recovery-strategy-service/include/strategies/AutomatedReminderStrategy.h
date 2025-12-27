// AutomatedReminderStrategy.h - Strategy for sending automated payment reminders

#ifndef AUTOMATED_REMINDER_STRATEGY_H
#define AUTOMATED_REMINDER_STRATEGY_H

#include "RecoveryStrategy.h"
#include "../interfaces/IPaymentChecker.h"
#include "../../../communication-service/include/interfaces/ICommunicationService.h"
#include "../../../common/include/utils/Constants.h"
#include <memory>

namespace sdrs::strategy
{

class AutomatedReminderStrategy : public RecoveryStrategy
{
private:
    int _maxReminders = 0;
    int _intervalDays = 0;
    
    std::shared_ptr<IPaymentChecker> _paymentChecker;
    std::shared_ptr<sdrs::communication::ICommunicationService> _channel;

public:
    AutomatedReminderStrategy(int accountId,
        int borrowerId,
        const sdrs::money::Money& expectedAmount,
        std::shared_ptr<IPaymentChecker> paymentChecker,
        std::shared_ptr<sdrs::communication::ICommunicationService> channel,
        int maxReminders = sdrs::constants::recovery::MAX_REMINDER_ATTEMPTS,
        int intervalDays = sdrs::constants::recovery::INTERVAL_DAYS
    );

public:
    sdrs::constants::StrategyStatus execute() override;  // sends reminders until paid or max attempts
    sdrs::constants::StrategyType getType() const override;
    std::string toJson() const override;
    std::string statusToString() const;

private:
    bool checkPaymentReceived() const;  // queries payment checker
    void sendReminder();                // sends via communication channel

};

} // namespace sdrs::strategy

#endif