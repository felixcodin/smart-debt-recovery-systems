// PaymentCheckerImpl.h - Implementation of IPaymentChecker using repository

#ifndef PAYMENT_CHECKER_IMPL_H
#define PAYMENT_CHECKER_IMPL_H

#include "../../../recovery-strategy-service/include/interfaces/IPaymentChecker.h"
#include "../interfaces/IBorrowerRepository.h"
#include <memory>

namespace sdrs::borrower
{
    
class PaymentCheckerImpl : public sdrs::strategy::IPaymentChecker
{
private:
    std::shared_ptr<IBorrowerRepository> _repository;

public:
    explicit PaymentCheckerImpl(std::shared_ptr<IBorrowerRepository> repo);
    bool hasPaymentReceived(int accountId) const override;
    bool hasFullPayment(int accountId, const sdrs::money::Money& expectedAmount) const override;

};

} // namespace sdrs::borrower


#endif