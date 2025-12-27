// RecoveryStrategy.h - Abstract base class for debt recovery strategies (Strategy Pattern)

#ifndef RECOVERY_STRATEGY_H
#define RECOVERY_STRATEGY_H

#include <string>
#include "../../../common/include/models/Money.h"
#include "../../../common/include/utils/Constants.h"

namespace sdrs::strategy
{

class RecoveryStrategy
{
protected:
    int _strategyId;
    int _accountId;
    int _borrowerId;
    sdrs::constants::StrategyStatus _status;
    int _attemptCount;
    sdrs::money::Money _expectedAmount;
    sdrs::money::Money _actualAmount;

public:
    RecoveryStrategy(int accountId,
        int borrowerId,
        const sdrs::money::Money& expectedAmount);

    // Pure virtual methods - must be implemented by subclasses
public:
    virtual sdrs::constants::StrategyStatus execute() = 0;  // run the strategy, returns final status
    virtual sdrs::constants::StrategyType getType() const = 0;
    virtual std::string toJson() const = 0;
    virtual ~RecoveryStrategy() = default;

public:
    bool canExecute() const;  // check if strategy can still run (not completed/failed)

};

} // namespace sdrs::strategy


#endif