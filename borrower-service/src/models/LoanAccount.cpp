#include "../../include/models/LoanAccount.h"
#include "../../../common/include/exceptions/ValidationException.h"

namespace sdrs::borrower {

LoanAccount::LoanAccount(int id, int borrower_id,
                         double loan_amt, double rate)
    : account_id(id), borrower_id(borrower_id),
      loan_amount(loan_amt), remaining_amount(loan_amt),
      interest_rate(rate), account_status(STATUS_ON_TIME) {
}

void LoanAccount::updateStatus(const std::string& new_status) {
    if (!canUpdateStatus(account_status, new_status)) {
        throw sdrs::exceptions::ValidationException("Invalid status transition", "account_status");
    }
    account_status = new_status;
}

void LoanAccount::markPaymentMissed() {
    number_of_missed_payments++;
    days_past_due += 30;

    // business rule: more than 1 missed -> delayed
    if (number_of_missed_payments > 1) {
        updateStatus(STATUS_DELAYED);
    }
}

void LoanAccount::recordPayment(double amount) {
    if (amount < 0) {
        throw sdrs::exceptions::ValidationException("Amount cannot be negative", "payment_amount");
    }
    if (amount > remaining_amount) {
        throw sdrs::exceptions::ValidationException("Payment exceeds remaining balance", "payment_amount");
    }
    remaining_amount -= amount;
}

bool LoanAccount::canUpdateStatus(const std::string& from_status,
                                  const std::string& to_status) const {

    if (from_status == STATUS_ON_TIME) {
        return to_status == STATUS_DELAYED;
    }
    if (from_status == STATUS_DELAYED) {
        return to_status == STATUS_ON_TIME || to_status == STATUS_PARTIAL;
    }
    if (from_status == STATUS_PARTIAL) {
        return to_status == STATUS_ON_TIME || to_status == STATUS_WRITTEN_OFF;
    }

    return false;
}

} // namespace sdrs::borrower
