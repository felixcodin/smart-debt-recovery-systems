// SettlementOfferStrategy.h - Strategy for offering discounted settlement amounts

#ifndef SETTLEMENT_OFFER_STRATEGY_H
#define SETTLEMENT_OFFER_STRATEGY_H

#include "RecoveryStrategy.h"
#include "../../../common/include/models/Money.h"
#include "../interfaces/IPaymentChecker.h"
#include "../../../communication-service/include/interfaces/ICommunicationService.h"
#include "../../../common/include/utils/Constants.h"
#include <chrono>
#include <memory>

namespace sdrs::strategy
{
    

class SettlementOfferStrategy : public RecoveryStrategy
{
private:
    std::shared_ptr<IPaymentChecker> _paymentChecker;
    std::shared_ptr<sdrs::communication::ICommunicationService> _channel;

    double _discountRate;
    int _offerValidDays;
    sdrs::money::Money _minimumAcceptableAmount;

    int _maxSettlementAttempt = 0;

public:
    SettlementOfferStrategy(
        int accountId,
        int borrowerId,
        const sdrs::money::Money& expectedAmount,
        std::shared_ptr<IPaymentChecker> paymentChecker,
        std::shared_ptr<sdrs::communication::ICommunicationService> channel,
        double discountRate = 0.15,
        int offerValidDays = 30,
        int maxSettlementAttempt = sdrs::constants::recovery::MAX_SETTLEMENT_ATTEMPTS
    );

public:
    sdrs::constants::StrategyStatus execute() override;  // sends discount offer, checks for acceptance
    sdrs::constants::StrategyType getType() const override;
    std::string toJson() const override;

private:
    sdrs::money::Money calculateSettlementAmount() const;  // expectedAmount * (1 - discountRate)
    void sendOffer();              // sends offer via communication channel
    bool checkPaymentReceived() const;

};

} // namespace sdrs::strategy

#endif