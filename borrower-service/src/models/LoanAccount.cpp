#include "../../include/models/LoanAccount.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <sstream>
#include <cmath>
#include <chrono>
#include <string>

namespace sdrs::borrower
{

void LoanAccount::validateConstructorArgs(int accountId,int borrowerId,double loanAmount,double interestRate) const
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
}

void LoanAccount::touch()
{
    _updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

LoanAccount::LoanAccount(int accountId,
                         int borrowerId,
                         double loanAmount,
                         double interestRate):
    _accountId(accountId),
    _borrowerId(borrowerId),
    _loanAmount(loanAmount),
    _initialAmount(loanAmount),
    _remainingAmount(loanAmount),
    _interestRate(interestRate)
{
    validateConstructorArgs(accountId, borrowerId, loanAmount, interestRate);
    _loanStartDate = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    _loanEndDate = _loanStartDate + std::chrono::years{1};
    _createdAt = _loanStartDate;
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

double LoanAccount::getInitialAmount() const 
{ 
    return _initialAmount; 
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

bool LoanAccount::isValidAmounts() const 
{ 
    return _validAmounts; 
}

bool LoanAccount::isValidInterest() const 
{ 
    return _validInterest; 
}

void LoanAccount::setRemainingAmount(double amount)
{
    if (amount < 0)
    {
        throw sdrs::exceptions::ValidationException("Remaining amount cannot be negative", "remainingAmount");
    }
    _remainingAmount = amount;
    touch();
}

void LoanAccount::setInterestRate(double rate)
{
    if (rate < 0 || rate > 1)
    {
        _validInterest = false;
        throw sdrs::exceptions::ValidationException("Invalid interest rate", "interestRate");
    }
    _interestRate = rate;
    _validInterest = true;
    touch();
}

void LoanAccount::setDaysPastDue(int days)
{
    if (days < 0) 
    {
        return;
    }
    _daysPastDue = days;
    touch();
}

void LoanAccount::setMissedPayments(int missedPayments)
{
    if (missedPayments < 0) 
    {
        return;
    }
    _numberOfMissedPayments = missedPayments;
    touch();
}

void LoanAccount::setLoanEndDate(std::chrono::sys_seconds endDate)
{
    if (endDate < _loanStartDate)
    {
        throw sdrs::exceptions::ValidationException("Loan end date cannot be before start date", "loanEndDate");
    }
    _loanEndDate = endDate;
    touch();
}

void LoanAccount::setUpdatedAt(std::chrono::sys_seconds updatedAt)
{
    _updatedAt = updatedAt;
}

void LoanAccount::setValidAmounts(bool valid) 
{ 
    _validAmounts = valid; 
}

void LoanAccount::setValidInterest(bool valid) 
{ 
    _validInterest = valid; 
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
            return toStatus == AccountStatus::Delinquent || toStatus == AccountStatus::Partial || toStatus == AccountStatus::PaidOff;
        }
        case AccountStatus::Delinquent:
        {
            return toStatus == AccountStatus::Current || toStatus == AccountStatus::Partial || toStatus == AccountStatus::Default;
        }
        case AccountStatus::Default:
        {
            return toStatus == AccountStatus::ChargedOff || toStatus == AccountStatus::Settled;
        }
        case AccountStatus::Partial:
        {
            return toStatus == AccountStatus::Current ||toStatus == AccountStatus::Delinquent;
        }
        case AccountStatus::PaidOff:
        case AccountStatus::ChargedOff:
        case AccountStatus::Settled:
        default:
            return false;
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
    if (_daysPastDue >= 90)
    {
        updateStatus(AccountStatus::Default);
    }
    else
    {
        updateStatus(AccountStatus::Delinquent);
    }
}

void LoanAccount::recordPayment(double amount)
{
    if (amount <= 0)
    {
        throw sdrs::exceptions::ValidationException("Invalid payment amount", "paymentAmount");
    }

    _remainingAmount -= amount;

    if (_remainingAmount < -0.0001)
    {
        _validAmounts = false;
        throw sdrs::exceptions::ValidationException("Payment exceeds remaining amount", "paymentAmount");
    }

    if (std::abs(_remainingAmount) < 0.0001)
    {
        _remainingAmount = 0;
    }

    if (_remainingAmount == 0)
    {
        _daysPastDue = 0;
        _numberOfMissedPayments = 0;
        updateStatus(AccountStatus::PaidOff);
    }
    else if (_accountStatus == AccountStatus::Delinquent || _accountStatus == AccountStatus::Default)
    {
        updateStatus(AccountStatus::Partial);
    }
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
        << "\"daysPastDue\":" << _daysPastDue << ","
        << "\"missedPayments\":" << _numberOfMissedPayments << ","
        << "\"status\":\"" << statusToString(_accountStatus) << "\""
        << "}";
    return oss.str();
}
}
