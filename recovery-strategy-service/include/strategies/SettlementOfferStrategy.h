#ifndef SETTLEMENT_OFFER_STRATEGY_H
#define SETTLEMENT_OFFER_STRATEGY_H

#include "RecoveryStrategy.h"
#include "../../../common/include/models/Money.h"
#include "../interfaces/IPaymentChecker.h"
#include "../../../communication-service/include/interfaces/ICommunicationService.h"
#include <chrono>
#include <memory>

namespace sdrs::strategy
{
    

class SettlementOfferStrategy : public RecoveryStrategy
{
private:
    double _discountRate;
    int _offerValidDays;
    sdrs::money::Money _minimumAcceptableAmount;

    std::shared_ptr<IPaymentChecker> _paymentChecker;
    std::shared_ptr<sdrs::communication::ICommunicationService> _channel;

public:
    SettlementOfferStrategy(
        int accountId,
        int borrowerId,
        const sdrs::money::Money& expectedAmount,
        std::shared_ptr<IPaymentChecker> paymentChecker,
        std::shared_ptr<sdrs::communication::ICommunicationService> channel,
        double discountRate = 0.15,
        int offerValidDays = 30
    );

public:
    StrategyStatus execute() override;
    StrategyType getType() const override;
    std::string toJson() const override;

private:
    sdrs::money::Money calculateSettlementAmount() const;
    void sendOffer();
    bool checkPaymentReceived() const;

};

} // namespace sdrs::strategy

#endif