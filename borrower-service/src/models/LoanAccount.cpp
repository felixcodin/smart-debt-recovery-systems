#include "../../include/models/LoanAccount.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <algorithm>
#include <chrono>
#include <format>
#include <sstream>

namespace sdrs::borrower
{

std::string LoanAccount::formatTime(time_t t)
{
    if (t == 0) return "";
    using namespace std::chrono;
    sys_seconds tp = sys_seconds{seconds{t}};
    return std::format("{:%Y-%m-%dT%H:%M:%SZ}", tp);
}

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
    auto now = std::chrono::system_clock::now();
    _createdAt = std::chrono::system_clock::to_time_t(now);
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

std::string LoanAccount::getCreatedAt() const 
{
    return formatTime(_createdAt);
}
std::string LoanAccount::getUpdatedAt() const 
{
    return formatTime(_updatedAt);
}

void LoanAccount::updateStatus(AccountStatus newStatus)
{
    if (!canUpdateStatus(_accountStatus, newStatus))
    {
        throw sdrs::exceptions::ValidationException("Invalid status transition", "AccountStatus");
    }
    _accountStatus = newStatus;
    _updatedAt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}
bool LoanAccount::canUpdateStatus(AccountStatus from, AccountStatus to) const
{
    if (from == to) 
    {
        return true;
    }
    if (from == AccountStatus::WrittenOff)
    {
        return false;
    }

    switch (from)
    {
        case AccountStatus::OnTime:
            return to == AccountStatus::Delayed;

        case AccountStatus::Delayed:
            return to == AccountStatus::OnTime ||to == AccountStatus::Partial;
        case AccountStatus::Partial:
            return to == AccountStatus::OnTime ||to == AccountStatus::WrittenOff;
        default:
            return false;
    }
}

void LoanAccount::markPaymentMissed()
{
    _numberOfMissedPayments++;
    _daysPastDue += PAYMENT_CYCLE_DAYS;
    if (_numberOfMissedPayments > 1)
    {
        if (canUpdateStatus(_accountStatus, AccountStatus::Delayed))
            _accountStatus = AccountStatus::Delayed;
    }
    _updatedAt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}
void LoanAccount::recordPayment(double amount)
{
    if (amount < 0)
        throw sdrs::exceptions::ValidationException("Amount cannot be negative", "payment_amount");
    if (amount > _remainingAmount)
        throw sdrs::exceptions::ValidationException("Payment exceeds remaining balance", "payment_amount");
    _remainingAmount -= amount;
    if (_remainingAmount <= 0.0)
    {
        if (canUpdateStatus(_accountStatus, AccountStatus::WrittenOff))
            _accountStatus = AccountStatus::WrittenOff;
        _daysPastDue = 0;
        _numberOfMissedPayments = 0;
    }
    else
    {
        if (canUpdateStatus(_accountStatus, AccountStatus::OnTime))
        {
            _accountStatus = AccountStatus::OnTime;
            _daysPastDue = 0;
            _numberOfMissedPayments = 0;
        }
    }
    _updatedAt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}
void LoanAccount::incrementDaysPastDue(int days)
{
    if (days > 0)
    {
        _daysPastDue += days;
        _updatedAt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
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
        return "Interest rate cannot be under 0% or above 100%";
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
        default:                         
        {
            return "Unknown";
        }
    }
}
AccountStatus LoanAccount::stringToStatus(const std::string& str)
{
    if (str == "On-Time")             
    {
        return AccountStatus::OnTime;
    }
    if (str == "Delayed")             
    {
        return AccountStatus::Delayed;
    }
    if (str == "Partially Recovered") 
    {
        return AccountStatus::Partial;
    }
    if (str == "Written-off")         
    {
        return AccountStatus::WrittenOff;
    }
    throw sdrs::exceptions::ValidationException("Unknown status string", "AccountStatus");
}
} 
