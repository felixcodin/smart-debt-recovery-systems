#include "../../include/models/LoanAccount.h"
#include "../../../common/include/exceptions/ValidationException.h"
#include "../../../common/include/models/Money.h"

#include <sstream>
#include <chrono>

namespace sdrs::borrower
{
void LoanAccount::validateConstructorArgs(int accountId,int borrowerId,double loanAmount,double interestRate,int loanTermMonths) const
{
    if (accountId <= 0)
    {
        throw sdrs::exceptions::ValidationException("Invalid account id", "accountId");
    }
    if (borrowerId <= 0)
    {
        throw sdrs::exceptions::ValidationException("Invalid borrower id", "borrowerId");
    }
    if (loanAmount <= 0)
    {
        throw sdrs::exceptions::ValidationException("Loan amount must be positive", "loanAmount");
    }
    if (interestRate < 0 || interestRate > 1)
    {
        throw sdrs::exceptions::ValidationException("Interest rate must be between 0 and 1", "interestRate");
    }
    if (loanTermMonths <= 0)
    {
        throw sdrs::exceptions::ValidationException("Loan term must be positive", "loanTermMonths");
    }
}

void LoanAccount::touch()
{
    _updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

void LoanAccount::recalculateMonthlyPayment()
{
    sdrs::money::Money totalInterest = _loanAmount * _interestRate;
    sdrs::money::Money totalPayable  = _loanAmount + totalInterest;
    _monthlyPaymentAmount = totalPayable / static_cast<double>(_loanTermMonths);
}

void LoanAccount::updateNextPaymentDueDate()
{
    if (_lastPaymentDate.time_since_epoch().count() == 0)
    {
        _nextPaymentDueDate = _loanStartDate + std::chrono::days{PAYMENT_CYCLE_DAYS};
    }
    else
    {
        _nextPaymentDueDate = _lastPaymentDate + std::chrono::days{PAYMENT_CYCLE_DAYS};
    }
}

LoanAccount::LoanAccount(int accountId,
                         int borrowerId,
                         double loanAmount,
                         double interestRate,
                         int loanTermMonths):
    _accountId(accountId),
    _borrowerId(borrowerId),
    _loanAmount(loanAmount),
    _initialAmount(loanAmount),
    _remainingAmount(loanAmount),
    _interestRate(interestRate),
    _monthlyPaymentAmount(0),
    _totalPaidAmount(0),
    _lateFees(0),
    _loanTermMonths(loanTermMonths)
{
    validateConstructorArgs(accountId, borrowerId, loanAmount, interestRate, loanTermMonths);
    _loanStartDate = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    _loanEndDate = _loanStartDate + std::chrono::months{loanTermMonths};
    _createdAt = _loanStartDate;
    _updatedAt = _createdAt;
    recalculateMonthlyPayment();
    updateNextPaymentDueDate();
}

int LoanAccount::getAccountId() const
{
    return _accountId;
}

int LoanAccount::getBorrowerId() const
{
    return _borrowerId;
}

sdrs::money::Money LoanAccount::getLoanAmount() const
{
    return _loanAmount;
}

sdrs::money::Money LoanAccount::getInitialAmount() const
{
    return _initialAmount;
}

sdrs::money::Money LoanAccount::getRemainingAmount() const
{
    return _remainingAmount;
}

sdrs::money::Money LoanAccount::getMonthlyPaymentAmount() const
{
    return _monthlyPaymentAmount;
}

sdrs::money::Money LoanAccount::getTotalPaidAmount() const
{
    return _totalPaidAmount;
}

sdrs::money::Money LoanAccount::getLateFees() const
{
    return _lateFees;
}

double LoanAccount::getInterestRate() const
{
    return _interestRate;
}

int LoanAccount::getLoanTermMonths() const
{
    return _loanTermMonths;
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

std::chrono::sys_seconds LoanAccount::getLoanStartDate() const
{
    return _loanStartDate;
}

std::chrono::sys_seconds LoanAccount::getLoanEndDate() const
{
    return _loanEndDate;
}

std::chrono::sys_seconds LoanAccount::getCreatedAt() const
{
    return _createdAt;
}

std::chrono::sys_seconds LoanAccount::getUpdatedAt() const
{
    return _updatedAt;
}

std::chrono::sys_seconds LoanAccount::getLastPaymentDate() const
{
    return _lastPaymentDate;
}

std::chrono::sys_seconds LoanAccount::getNextPaymentDueDate() const
{
    return _nextPaymentDueDate;
}

bool LoanAccount::canUpdateStatus(AccountStatus fromStatus,AccountStatus toStatus) const
{
    if (fromStatus == toStatus)
    {
        return true;
    }
    switch (fromStatus)
    {
        case AccountStatus::Current:
        {
            return (toStatus == AccountStatus::Delinquent ||toStatus == AccountStatus::Partial ||toStatus == AccountStatus::PaidOff);
        }
        case AccountStatus::Delinquent:
        {
            return (toStatus == AccountStatus::Current ||toStatus == AccountStatus::Partial ||toStatus == AccountStatus::Default);
        }
        case AccountStatus::Partial:
        {
            return (toStatus == AccountStatus::Current ||toStatus == AccountStatus::Delinquent);
        }
        case AccountStatus::Default:
        {
            return (toStatus == AccountStatus::ChargedOff ||toStatus == AccountStatus::Settled);
        }
        default:
        {
            return false;
        }
    }
}

void LoanAccount::updateStatus(AccountStatus newStatus)
{
    if (!canUpdateStatus(_accountStatus, newStatus))
    {
        throw sdrs::exceptions::ValidationException("Invalid account status transition","accountStatus");
    }
    _accountStatus = newStatus;
    touch();
}

void LoanAccount::markPaymentMissed()
{
    _numberOfMissedPayments++;
    _daysPastDue += PAYMENT_CYCLE_DAYS;
    _lateFees += (_monthlyPaymentAmount * 0.02);
    if (_daysPastDue >= 90)
    {
        updateStatus(AccountStatus::Default);
    }
    else
    {
        updateStatus(AccountStatus::Delinquent);
    }
    updateNextPaymentDueDate();
    touch();
}

void LoanAccount::recordPayment(const sdrs::money::Money& amount)
{
    if (amount <= sdrs::money::Money(0))
    {
        throw sdrs::exceptions::ValidationException("Invalid payment amount","paymentAmount");
    }

    if (amount > _remainingAmount)
    {
        throw sdrs::exceptions::ValidationException("Payment exceeds remaining amount","paymentAmount");
    }
    _remainingAmount -= amount;
    _totalPaidAmount += amount;
    _lastPaymentDate = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    updateNextPaymentDueDate();

    if (_remainingAmount == sdrs::money::Money(0))
    {
        _daysPastDue = 0;
        _numberOfMissedPayments = 0;
        updateStatus(AccountStatus::PaidOff);
    }
    else if (_accountStatus == AccountStatus::Delinquent || _accountStatus == AccountStatus::Default)
    {
        updateStatus(AccountStatus::Partial);
    }
    touch();
}

void LoanAccount::incrementDaysPastDue(int days)
{
    if (days <= 0)
    {
        return;
    }
    _daysPastDue += days;
    if (_daysPastDue >= 90)
    {
        updateStatus(AccountStatus::Default);
    }
    else if (_daysPastDue >= 30)
    {
        updateStatus(AccountStatus::Delinquent);
    }
}

bool LoanAccount::isTerminalStatus() const
{
    return (_accountStatus == AccountStatus::PaidOff || _accountStatus == AccountStatus::ChargedOff ||_accountStatus == AccountStatus::Settled);
}

bool LoanAccount::isFullyPaid() const
{
    return (_remainingAmount == sdrs::money::Money(0));
}

std::string LoanAccount::statusToString(AccountStatus status)
{
    switch (status)
    {
        case AccountStatus::Current:
        {
            return "Current";
        }
        case AccountStatus::Delinquent:
        {
            return "Delinquent";
        }
        case AccountStatus::Partial:
        {
            return "Partial";
        }
        case AccountStatus::Default:
        {
            return "Default";
        }
        case AccountStatus::PaidOff:
        {
            return "PaidOff";
        }
        case AccountStatus::ChargedOff:
        {
            return "ChargedOff";
        }
        case AccountStatus::Settled:
        {
            return "Settled";
        }
        default:
        {
            return "Unknown";
        }
    }
}

AccountStatus LoanAccount::stringToStatus(const std::string& statusStr)
{
    if (statusStr == "Current")
    {
        return AccountStatus::Current;
    }
    if (statusStr == "Delinquent")
    {
        return AccountStatus::Delinquent;
    }
    if (statusStr == "Partial")
    {
        return AccountStatus::Partial;
    }
    if (statusStr == "Default")
    {
        return AccountStatus::Default;
    }
    if (statusStr == "PaidOff")
    {
        return AccountStatus::PaidOff;
    }
    if (statusStr == "ChargedOff")
    {
        return AccountStatus::ChargedOff;
    }
    if (statusStr == "Settled")
    {
        return AccountStatus::Settled;
    }
    throw sdrs::exceptions::ValidationException("Unknown account status string","accountStatus");
}

std::string LoanAccount::toJson() const
{
    std::ostringstream oss;
    oss << "{";
    oss << "\"accountId\":" << _accountId << ",";
    oss << "\"borrowerId\":" << _borrowerId << ",";
    oss << "\"remainingAmount\":" << _remainingAmount.getAmount() << ",";
    oss << "\"monthlyPaymentAmount\":" << _monthlyPaymentAmount.getAmount() << ",";
    oss << "\"totalPaidAmount\":" << _totalPaidAmount.getAmount() << ",";
    oss << "\"lateFees\":" << _lateFees.getAmount() << ",";
    oss << "\"loanTermMonths\":" << _loanTermMonths << ",";
    oss << "\"daysPastDue\":" << _daysPastDue << ",";
    oss << "\"missedPayments\":" << _numberOfMissedPayments << ",";
    oss << "\"status\":\"" << statusToString(_accountStatus) << "\"";
    oss << "}";
    return oss.str();
}

} 
