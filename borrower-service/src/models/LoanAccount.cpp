#include "../../include/models/LoanAccount.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <sstream>
#include <cmath>
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
    if (loanAmount <= 0 || std::isnan(loanAmount))
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
    double totalInterest = _loanAmount * _interestRate;
    double totalPayable = _loanAmount + totalInterest;
    _monthlyPaymentAmount = totalPayable / _loanTermMonths;
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

LoanAccount::LoanAccount(int accountId,int borrowerId,double loanAmount,double interestRate,int loanTermMonths):
      _accountId(accountId),
      _borrowerId(borrowerId),
      _loanAmount(loanAmount),
      _initialAmount(loanAmount),
      _remainingAmount(loanAmount),
      _interestRate(interestRate),
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

bool LoanAccount::canUpdateStatus(AccountStatus from, AccountStatus to) const
{
    if (from == to)
    {
        return true;
    }
    switch (from)
    {
        case AccountStatus::Current:
        {
            return to == AccountStatus::Delinquent || to == AccountStatus::Partial || to == AccountStatus::PaidOff;
        }
        case AccountStatus::Delinquent:
        {
            return to == AccountStatus::Current || to == AccountStatus::Partial || to == AccountStatus::Default;
        }
        case AccountStatus::Partial:
        {
            return to == AccountStatus::Current || to == AccountStatus::Delinquent;
        }
        case AccountStatus::Default:
        {
            return to == AccountStatus::ChargedOff || to == AccountStatus::Settled;
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
        throw sdrs::exceptions::ValidationException("Invalid account status transition", "accountStatus");
    }
    _accountStatus = newStatus;
    touch();
}

void LoanAccount::markPaymentMissed()
{
    _numberOfMissedPayments++;
    _daysPastDue += PAYMENT_CYCLE_DAYS;
    _lateFees += _monthlyPaymentAmount * 0.02;
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

void LoanAccount::recordPayment(double amount)
{
    if (amount <= 0)
    {
        throw sdrs::exceptions::ValidationException("Invalid payment amount", "paymentAmount");
    }
    _remainingAmount -= amount;
    _totalPaidAmount += amount;
    _lastPaymentDate = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    updateNextPaymentDueDate();
    if (_remainingAmount < -0.0001)
    {
        _validAmounts = false;
        throw sdrs::exceptions::ValidationException("Payment exceeds remaining amount", "paymentAmount");
    }
    if (std::abs(_remainingAmount) < 0.0001)
    {
        _remainingAmount = 0;
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
    return _accountStatus == AccountStatus::PaidOff || _accountStatus == AccountStatus::ChargedOff || _accountStatus == AccountStatus::Settled;
}

bool LoanAccount::isFullyPaid() const
{
    return _remainingAmount == 0;
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
    throw sdrs::exceptions::ValidationException("Unknown account status string", "accountStatus");
}

std::string LoanAccount::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"accountId\":" << _accountId << ","
        << "\"borrowerId\":" << _borrowerId << ","
        << "\"remainingAmount\":" << _remainingAmount << ","
        << "\"monthlyPaymentAmount\":" << _monthlyPaymentAmount << ","
        << "\"totalPaidAmount\":" << _totalPaidAmount << ","
        << "\"lateFees\":" << _lateFees << ","
        << "\"loanTermMonths\":" << _loanTermMonths << ","
        << "\"daysPastDue\":" << _daysPastDue << ","
        << "\"missedPayments\":" << _numberOfMissedPayments << ","
        << "\"status\":\"" << statusToString(_accountStatus) << "\""
        << "}";
    return oss.str();
}
}
