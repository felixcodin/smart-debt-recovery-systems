#ifndef INTERFACE_PAYMENT_CHECKER_H
#define INTERFACE_PAYMENT_CHECKER_H

#include "../../../common/include/models/Money.h"

namespace sdrs::strategy
{
    
class IPaymentChecker
{
public:
    virtual ~IPaymentChecker() = default;
    virtual bool hasPaymentReceived(int accountId) const = 0;
    virtual bool hasFullPayment(int accountId, const sdrs::money::Money& expectedAmount) const = 0;

};

} // namespace sdrs::strategy



#endif