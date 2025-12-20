#ifndef AUTOMATED_REMINDER_STRATEGY_H
#define AUTOMATED_REMINDER_STRATEGY_H

#include "RecoveryStrategy.h"
#include "../interfaces/IPaymentChecker.h"
#include "../../../communication-service/include/interfaces/ICommunicationService.h"
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
        int maxReminders = 3, int intervalDays = 7);

public:
    StrategyStatus execute() override;
    StrategyType getType() const override;
    std::string toJson() const override;
    std::string statusToString() const;

private:
    bool checkPaymentReceived() const;
    void sendReminder();

};

} // namespace sdrs::strategy

#endif