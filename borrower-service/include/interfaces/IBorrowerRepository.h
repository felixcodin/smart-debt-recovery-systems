#ifndef INTERFACE_BORROWER_REPOSITORY_H
#define INTERFACE_BORROWER_REPOSITORY_H

#include "../models/PaymentHistory.h"
#include "../../../common/include/models/Money.h"
#include <vector>

namespace sdrs::borrower
{

class IBorrowerRepository
{
public:
    virtual ~IBorrowerRepository() = default;

    virtual std::vector<PaymentHistory> getPaymentsByAccount(int accountId) const = 0;
    virtual sdrs::money::Money getTotalPaidAmount(int accountId) const = 0;
};

} // namespace sdrs::borrower

#endif