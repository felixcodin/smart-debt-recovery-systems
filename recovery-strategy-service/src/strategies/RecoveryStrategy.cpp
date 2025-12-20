#include "../../include/strategies/RecoveryStrategy.h"

namespace sdrs::strategy
{

RecoveryStrategy::RecoveryStrategy(int accountId,
    int borrowerId,
    const sdrs::money::Money& expectedAmount)
    : _strategyId(0),
    _accountId(accountId),
    _borrowerId(borrowerId),
    _status(StrategyStatus::Pending),
    _attemptCount(0),
    _expectedAmount(expectedAmount),
    _actualAmount(0.0)
{
    // Do nothing
}

bool RecoveryStrategy::canExecute() const
{
    return _status == StrategyStatus::Active
    || _status == StrategyStatus::Pending;
}

}