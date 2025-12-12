#include "../../include/models/LoanAccount.h"
#include "../../../common/include/exceptions/ValidationException.h"

namespace sdrs::borrower 
{
LoanAccount::LoanAccount(int id,
    int borrower_id,double loan_amt, double rate):
    _accountId(id),
    _borrowerId(borrower_id),
    _loanAmount(loan_amt),
    _remainingAmount(loan_amt),
    _interestRate(rate),
    _accountStatus(STATUS_ON_TIME) 
{
    if (id <= 0) 
    {
        throw sdrs::exceptions::ValidationException("Invalid account ID", "account_id");
    }

    if (borrower_id <= 0) 
    {
        throw sdrs::exceptions::ValidationException("Invalid borrower ID", "borrower_id");
    }

    if (loan_amt <= 0) 
    {
        throw sdrs::exceptions::ValidationException("Loan amount must be greater than zero", "loan_amount");
    }

    if (rate < 0) 
    {
        throw sdrs::exceptions::ValidationException("Interest rate cannot be negative", "interest_rate");
    }

    if (rate > 1.0) 
    {
        throw sdrs::exceptions::ValidationException("Interest rate exceeds allowed maximum", "interest_rate");
    }

    _numberOfMissedPayments = 0;
    _daysPastDue = 0;
}


void LoanAccount::updateStatus(const std::string& new_status)
{
    if (!canUpdateStatus(_accountStatus, new_status)) 
    {
        throw sdrs::exceptions::ValidationException(
            "Invalid status transition", "account_status"
        );
    }
    _accountStatus = new_status;
}

void LoanAccount::markPaymentMissed()
{
    _numberOfMissedPayments++;
    _daysPastDue += 30;

    if (_numberOfMissedPayments > 1) 
    {
        updateStatus(STATUS_DELAYED);
    }
}

void LoanAccount::recordPayment(double amount) 
{
    if (amount < 0) 
    {
        throw sdrs::exceptions::ValidationException("Amount cannot be negative", "payment_amount");
    }
    if (amount > _remainingAmount) 
    {
        throw sdrs::exceptions::ValidationException("Payment exceeds remaining balance", "payment_amount");
    }
    _remainingAmount -= amount;
}

bool LoanAccount::canUpdateStatus(const std::string& from_status,const std::string& to_status) const 
{

    if (from_status == STATUS_ON_TIME) 
    {
        return to_status == STATUS_DELAYED;
    }
    if (from_status == STATUS_DELAYED) {
        return to_status == STATUS_ON_TIME || to_status == STATUS_PARTIAL;
    }
    if (from_status == STATUS_PARTIAL) 
    {
        return to_status == STATUS_ON_TIME || to_status == STATUS_WRITTEN_OFF;
    }
    return false;
}


int LoanAccount::getAccountId() const 
{
    return _accountId;
}

int LoanAccount::getBorrowerId() const 
{
    return _borrowerId;
}

double LoanAccount::getLoanAmount() const 
{
    return _loanAmount;
}

double LoanAccount::getRemainingAmount() const 
{
    return _remainingAmount;
}

double LoanAccount::getInterestRate() const 
{
    return _interestRate;
}

int LoanAccount::getMissedPayments() const 
{
    return _numberOfMissedPayments;
}

int LoanAccount::getDaysPastDue() const 
{
    return _daysPastDue;
}

const std::string& LoanAccount::getStatus() const 
{
    return _accountStatus;
}

}   

