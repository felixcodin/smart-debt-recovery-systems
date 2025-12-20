#include "../../include/strategies/SettlementOfferStrategy.h"
#include "../../../common/include/exceptions/ValidationException.h"
#include <format>

namespace sdrs::strategy
{

SettlementOfferStrategy::SettlementOfferStrategy(
    int accountId,
    int borrowerId,
    const sdrs::money::Money& expectedAmount,
    std::shared_ptr<IPaymentChecker> paymentChecker,
    std::shared_ptr<sdrs::communication::ICommunicationService> channel,
    double discountRate,
    int offerValidDays
) : RecoveryStrategy(accountId, borrowerId, expectedAmount),
    _paymentChecker(paymentChecker),
    _channel(channel),
    _discountRate(discountRate),
    _offerValidDays(offerValidDays)
{
    if (discountRate < 0 || discountRate > 1)
    {
        throw sdrs::exceptions::ValidationException("Discount Rate must be between 0 and 1", "SettlementOfferStrategy");
    }

    if (offerValidDays <= 0)
    {
        throw sdrs::exceptions::ValidationException("Offer Valid Days must be positive", "SettlementOfferStrategy");
    }

    _minimumAcceptableAmount = calculateSettlementAmount();
}

StrategyType SettlementOfferStrategy::getType() const
{
    return StrategyType::SettlementOffer;
}

sdrs::money::Money SettlementOfferStrategy::calculateSettlementAmount() const
{
    double discountedAmount = _expectedAmount.getAmount() * (1.0 -_discountRate);
    return sdrs::money::Money(discountedAmount);
}

void SettlementOfferStrategy::sendOffer()
{
    _attemptCount++;

    sdrs::money::Money offerAmount = calculateSettlementAmount();

    std::string message = std::format(
        "SETTLEMENT OFFER\n Original amount: {}\n Discount: {}%\n Settlement amount: {}\n Valid for: {}",
        _expectedAmount.formatUSD(), _discountRate*100, offerAmount.formatUSD(), _offerValidDays
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

    sendOffer();

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