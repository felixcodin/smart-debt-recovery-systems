#include "../../include/strategies/RecoveryStrategy.h"

using namespace sdrs::constants;
using namespace sdrs::money;

namespace sdrs::strategy
{

RecoveryStrategy::RecoveryStrategy(int accountId,
    int borrowerId,
    const Money& expectedAmount)
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