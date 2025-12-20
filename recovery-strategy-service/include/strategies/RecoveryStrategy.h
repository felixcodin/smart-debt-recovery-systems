#ifndef RECOVERY_STRATEGY_H
#define RECOVERY_STRATEGY_H

#include "../../../common/include/models/Money.h"
#include <string>

namespace sdrs::strategy
{

enum class StrategyType
{
    AutomatedReminder,
    SettlementOffer,
    LegalAction
};

enum class StrategyStatus
{
    Pending,
    Active,
    Completed,
    Failed,
    Cancelled
};

class RecoveryStrategy
{
protected:
    int _strategyId;
    int _accountId;
    int _borrowerId;
    StrategyStatus _status;
    int _attemptCount;
    sdrs::money::Money _expectedAmount;
    sdrs::money::Money _actualAmount;

public:
    RecoveryStrategy(int accountId,
        int borrowerId,
        const sdrs::money::Money& expectedAmount);

public:
    virtual StrategyStatus execute() = 0;
    virtual StrategyType getType() const = 0;
    virtual std::string toJson() const = 0;
    virtual ~RecoveryStrategy() = default;

public:
    bool canExecute() const;

};

} // namespace sdrs::strategy


#endif