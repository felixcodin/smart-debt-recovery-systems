// SettlementOfferStrategy.cpp - Implementation

#include "../../include/strategies/SettlementOfferStrategy.h"
#include "../../../common/include/exceptions/ValidationException.h"
#include <format>

using namespace sdrs::exceptions;
using namespace sdrs::constants;
using namespace sdrs::money;
using namespace sdrs::communication;

namespace sdrs::strategy
{

SettlementOfferStrategy::SettlementOfferStrategy(
    int accountId,
    int borrowerId,
    const Money& expectedAmount,
    std::shared_ptr<IPaymentChecker> paymentChecker,
    std::shared_ptr<ICommunicationService> channel,
    double discountRate,
    int offerValidDays,
    int maxSettlementAttempt
) : RecoveryStrategy(accountId, borrowerId, expectedAmount),
    _paymentChecker(paymentChecker),
    _channel(channel),
    _discountRate(discountRate),
    _offerValidDays(offerValidDays),
    _maxSettlementAttempt(maxSettlementAttempt)
{
    if (discountRate < 0 || discountRate > 1)
    {
        throw ValidationException("Discount Rate must be between 0 and 1", "SettlementOfferStrategy");
    }

    if (offerValidDays <= 0)
    {
        throw ValidationException("Offer Valid Days must be positive", "SettlementOfferStrategy");
    }

    _minimumAcceptableAmount = calculateSettlementAmount();
}

StrategyType SettlementOfferStrategy::getType() const
{
    return StrategyType::SettlementOffer;
}

Money SettlementOfferStrategy::calculateSettlementAmount() const
{
    double discountedAmount = _expectedAmount.getAmount() * (1.0 - _discountRate);
    return Money(discountedAmount);
}

void SettlementOfferStrategy::sendOffer()
{
    _attemptCount++;

    Money offerAmount = calculateSettlementAmount();

    std::string message = std::format(
        "SETTLEMENT OFFER\n Original amount: {}\n Discount: {}%\n Settlement amount: {}\n Valid for: {}",
        _expectedAmount.format(), _discountRate*100, offerAmount.format(), _offerValidDays
    );

    _channel->sendMessage(_accountId, message);
}

bool SettlementOfferStrategy::checkPaymentReceived() const
{
    if (!_paymentChecker)
    {
        return false;
    }
    return _paymentChecker->hasFullPayment(_accountId, _expectedAmount);
}

StrategyStatus SettlementOfferStrategy::execute()
{
    if (!canExecute())
    {
        return _status;
    }

    _status = StrategyStatus::Active;
    while (_attemptCount < _maxSettlementAttempt)
    {
        if (checkPaymentReceived())
        {
            _status = StrategyStatus::Completed;
            _actualAmount = _minimumAcceptableAmount;
            return _status;
        }
        sendOffer();
        _attemptCount++;
    }
    if (checkPaymentReceived())
    {
        _actualAmount = _minimumAcceptableAmount;
        _status = StrategyStatus::Completed;
    }
    else
    {
        _status = StrategyStatus::Failed;
    }

    return _status;
}

std::string SettlementOfferStrategy::toJson() const
{
    return "{\"type\":\"SettlementOffer\",\"accountId\":" 
           + std::to_string(_accountId) 
           + ",\"discountRate\":" + std::to_string(_discountRate)
           + ",\"settlementAmount\":" + std::to_string(_minimumAcceptableAmount.getAmount())
           + "}";
}

} // namespace sdrs::strategy