#include "../../include/models/LoanAccount.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <sstream>
#include <format>
#include <cmath>

namespace sdrs::borrower
{
void LoanAccount::validateConstructorArgs(int accountId,int borrowerId,double loanAmount,double interestRate) const
{
    if (accountId <= 0)
    {
        throw sdrs::exceptions::ValidationException("Invalid account id","accountId");
    }
    if (borrowerId <= 0)
    {
        throw sdrs::exceptions::ValidationException("Invalid borrower id","borrowerId");
    }
    if (loanAmount <= 0 || std::isnan(loanAmount))
    {
        throw sdrs::exceptions::ValidationException("Loan amount must be positive","loanAmount");
    }

    if (interestRate < 0 || interestRate > 1)
    {
        throw sdrs::exceptions::ValidationException("Interest rate must be between 0 and 1","interestRate");
    }
}

LoanAccount::LoanAccount(int accountId,int borrowerId, double loanAmount,double interestRate): 
    _accountId(accountId),
    _borrowerId(borrowerId),
    _loanAmount(loanAmount),
    _remainingAmount(loanAmount),
    _interestRate(interestRate)
{
    validateConstructorArgs(accountId, borrowerId, loanAmount, interestRate);
    _createdAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
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
std::chrono::sys_seconds LoanAccount::getCreatedAt() const
{
    return _createdAt;
}
std::chrono::sys_seconds LoanAccount::getUpdatedAt() const
{
    return _updatedAt;
}

bool LoanAccount::canUpdateStatus(AccountStatus fromStatus,AccountStatus toStatus) const
{
    if (fromStatus == toStatus)
    {
        return true;
    }
    if (fromStatus == AccountStatus::WrittenOff)
    {
        return false;
    }
    switch (fromStatus)
    {
        case AccountStatus::OnTime:
            return toStatus == AccountStatus::Delayed;
        case AccountStatus::Delayed:
            return toStatus == AccountStatus::OnTime || toStatus == AccountStatus::Partial;
        case AccountStatus::Partial:
            return toStatus == AccountStatus::OnTime || toStatus == AccountStatus::WrittenOff;
        default:
            return false;
    }
}

void LoanAccount::updateStatus(AccountStatus newStatus)
{
    if (!canUpdateStatus(_accountStatus, newStatus))
    {
        throw sdrs::exceptions::ValidationException("Invalid account status transition","accountStatus");
    }
    _accountStatus = newStatus;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

void LoanAccount::markPaymentMissed()
{
    _numberOfMissedPayments++;
    _daysPastDue += PAYMENT_CYCLE_DAYS;
    updateStatus(AccountStatus::Delayed);
}

void LoanAccount::recordPayment(double amount)
{
    if (amount <= 0)
    {
        throw sdrs::exceptions::ValidationException("Payment amount must be positive","paymentAmount");
    }
    if (amount > _remainingAmount)
    {
        throw sdrs::exceptions::ValidationException("Payment exceeds remaining amount","paymentAmount");
    }
    _remainingAmount -= amount;
    if (_remainingAmount == 0)
    {
        _daysPastDue = 0;
        _numberOfMissedPayments = 0;
        updateStatus(AccountStatus::OnTime);
    }
    else
    {
        updateStatus(AccountStatus::Partial);
    }
    _updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

void LoanAccount::incrementDaysPastDue(int days)
{
    if (days <= 0)
    {
        return;
    }
    _daysPastDue += days;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

std::string LoanAccount::toJson() const
{
    auto formatTime = [](const std::chrono::sys_seconds& time) -> std::string
    {
        return std::format("{:%Y-%m-%dT%H:%M:%SZ}", time);
    };
    std::ostringstream oss;
    oss << "{"
        << "\"accountId\":" << _accountId << ","
        << "\"borrowerId\":" << _borrowerId << ","
        << "\"loanAmount\":" << _loanAmount << ","
        << "\"remainingAmount\":" << _remainingAmount << ","
        << "\"interestRate\":" << _interestRate << ","
        << "\"status\":\"" << statusToString(_accountStatus) << "\","
        << "\"daysPastDue\":" << _daysPastDue << ","
        << "\"missedPayments\":" << _numberOfMissedPayments << ","
        << "\"createdAt\":\"" << formatTime(_createdAt) << "\","
        << "\"updatedAt\":\"" << formatTime(_updatedAt) << "\""
        << "}";
    return oss.str();
}

std::string LoanAccount::statusToString(AccountStatus status)
{
    switch (status)
    {
        case AccountStatus::OnTime:
            return "On-Time";
        case AccountStatus::Delayed:
            return "Delayed";
        case AccountStatus::Partial:
            return "Partially Recovered";
        case AccountStatus::WrittenOff:
            return "Written-off";
        default:
            return "Unknown";
    }
}

AccountStatus LoanAccount::stringToStatus(const std::string& statusStr)
{
    if (statusStr == "On-Time")
        return AccountStatus::OnTime;
    if (statusStr == "Delayed")
        return AccountStatus::Delayed;
    if (statusStr == "Partially Recovered")
        return AccountStatus::Partial;
    if (statusStr == "Written-off")
        return AccountStatus::WrittenOff;
    throw sdrs::exceptions::ValidationException("Unknown account status string","accountStatus");
}
}
