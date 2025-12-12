#include "../../include/models/LoanAccount.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <algorithm>

namespace sdrs::borrower
{

LoanAccount::LoanAccount(int id, int borrower_id,
                         double loan_amt, double rate)
    : _accountId(id),
      _borrowerId(borrower_id),
      _loanAmount(loan_amt),
      _remainingAmount(loan_amt),
      _interestRate(rate),
      _accountStatus(AccountStatus::OnTime),
      _daysPastDue(0),
      _numberOfMissedPayments(0)
{
    _createdAt = std::time(nullptr);
    _updatedAt = _createdAt;
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
AccountStatus LoanAccount::getStatus() const 
{ 
    return _accountStatus; 
}
int LoanAccount::getDaysPastDue() const 
{ 
    return _daysPastDue; 
}
int LoanAccount::getMissedPayments() const 
{ 
    return _numberOfMissedPayments; 
}

void LoanAccount::updateStatus(AccountStatus newStatus)
{
    if (!canUpdateStatus(_accountStatus, newStatus))
    {
        throw sdrs::exceptions::ValidationException("Invalid status transition", "account_status");
    }
    _accountStatus = newStatus;
    _updatedAt = std::time(nullptr);
}

bool LoanAccount::canUpdateStatus(AccountStatus from, AccountStatus to) const
{
    if (from == AccountStatus::OnTime)
        return to == AccountStatus::Delayed;
    if (from == AccountStatus::Delayed)
        return to == AccountStatus::OnTime || to == AccountStatus::Partial;
    if (from == AccountStatus::Partial)
        return to == AccountStatus::OnTime || to == AccountStatus::WrittenOff;
    return false;
}

void LoanAccount::markPaymentMissed()
{
    _numberOfMissedPayments++;
    _daysPastDue += 30;
    if (_numberOfMissedPayments > 1)
    {
        updateStatus(AccountStatus::Delayed);
    }
    _updatedAt = std::time(nullptr);
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
    _updatedAt = std::time(nullptr);
}

void LoanAccount::incrementDaysPastDue(int days)
{
    if (days > 0)
    {
        _daysPastDue += days;
        _updatedAt = std::time(nullptr);
    }
}

bool LoanAccount::isValid() const
{
    return validate().empty();
}

std::string LoanAccount::validate() const
{
    if (_accountId <= 0) 
    {
        return "Invalid account ID";
    }
    if (_borrowerId <= 0) 
    {
        return "Invalid borrower ID";
    }
    if (_loanAmount <= 0) 
    {
        return "Loan amount must be positive";
    }   
    if (_interestRate < 0 || _interestRate > 1)
    {       
        return "Interest rate can not be under 0 or above 1";
    }
    return "";
}

std::string LoanAccount::statusToString(AccountStatus status)
{
    switch (status)
    {
    case AccountStatus::OnTime:      
    {
        return "On-Time";
    }
    case AccountStatus::Delayed:
    {
        return "Delayed";
    }      
    case AccountStatus::Partial:     
    {
        return "Partially Recovered";
    }
    case AccountStatus::WrittenOff:  
    {
        return "Written-off";
    }
    }
    return "Unknown";
}

AccountStatus LoanAccount::stringToStatus(const std::string& str)
{
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "on-time") 
    {
        return AccountStatus::OnTime;
    }
    if (lower == "delayed") 
    {
        return AccountStatus::Delayed;  
    }
    if (lower == "partially recovered") 
    {
        return AccountStatus::Partial;
    }
    if (lower == "written-off") 
    {
        return AccountStatus::WrittenOff;
    }   
    throw sdrs::exceptions::ValidationException("Unknown status string", "account_status");
}
}
