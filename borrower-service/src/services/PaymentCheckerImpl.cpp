#include "../../include/services/PaymentCheckerImpl.h"
#include "../../../common/include/exceptions/ValidationException.h"


namespace sdrs::borrower
{

PaymentCheckerImpl::PaymentCheckerImpl(std::shared_ptr<IBorrowerRepository> repo) : _repository(repo)
{
    if (!repo)
    {
        throw sdrs::exceptions::ValidationException("Repository cannot be null", "PaymentChecker", sdrs::exceptions::ErrorCode::InvalidFormat);
    }
}
 
bool PaymentCheckerImpl::hasPaymentReceived(int accountId) const
{
    auto payments = _repository->getPaymentsByAccount(accountId);
    return !payments.empty();
}

bool PaymentCheckerImpl::hasFullPayment(int accountId, const sdrs::money::Money& expectedAmount) const
{
    auto totalPaid = _repository->getTotalPaidAmount(accountId);
    return totalPaid >= expectedAmount;
}

} // namespace sdrs::borrower
