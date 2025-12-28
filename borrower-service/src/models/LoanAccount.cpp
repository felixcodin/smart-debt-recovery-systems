// LoanAccount.cpp - Implementation

#include "../../include/models/LoanAccount.h"
#include "../../../common/include/exceptions/ValidationException.h"
#include "../../../common/include/models/Money.h"
#include "../../../common/include/utils/Constants.h"

#include <sstream>
#include <chrono>
#include <format>
#include <nlohmann/json.hpp>

using namespace sdrs::constants;
using namespace sdrs::exceptions;

namespace sdrs::borrower
{
void LoanAccount::validateConstructorArgs(int accountId,int borrowerId,double loanAmount,double interestRate,int loanTermMonths) const
{
    if (accountId <= 0)
    {
        throw ValidationException("Invalid account id", "accountId");
    }
    if (borrowerId <= 0)
    {
        throw ValidationException("Invalid borrower id", "borrowerId");
    }
    if (loanAmount <= 0)
    {
        throw ValidationException("Loan amount must be positive", "loanAmount");
    }
    if (interestRate < 0 || interestRate > 1)
    {
        throw ValidationException("Interest rate must be between 0 and 1", "interestRate");
    }
    if (loanTermMonths <= 0)
    {
        throw ValidationException("Loan term must be positive", "loanTermMonths");
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
    int loanTermMonths)
    :_accountId(accountId),
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
        throw ValidationException("Invalid account status transition","accountStatus");
    }
    _accountStatus = newStatus;
    touch();
}

void LoanAccount::markPaymentMissed()
{
    _numberOfMissedPayments++;
    _daysPastDue += PAYMENT_CYCLE_DAYS;
    _lateFees += (_monthlyPaymentAmount * recovery::LATE_FEE_RATE);
    if (_daysPastDue >= risk::DPD_HIGH_THRESHOLD)
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
        throw ValidationException("Invalid payment amount","paymentAmount");
    }

    if (amount > _remainingAmount)
    {
        throw ValidationException("Payment exceeds remaining amount","paymentAmount");
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
    if (_daysPastDue >= risk::DPD_HIGH_THRESHOLD)
    {
        updateStatus(AccountStatus::Default);
    }
    else
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
    throw ValidationException("Unknown account status string","accountStatus");
}

std::string LoanAccount::toJson() const
{
    std::ostringstream oss;
    oss << "{";
    oss << "\"account_id\":" << _accountId << ",";
    oss << "\"borrower_id\":" << _borrowerId << ",";
    oss << "\"loan_amount\":" << _loanAmount.getAmount() << ",";
    oss << "\"initial_amount\":" << _initialAmount.getAmount() << ",";
    oss << "\"interest_rate\":" << _interestRate << ",";
    oss << "\"remaining_amount\":" << _remainingAmount.getAmount() << ",";
    oss << "\"loan_start_date\":\"" << std::format("{:%Y-%m-%d %H:%M:%S}", _loanStartDate) << "\",";
    oss << "\"loan_end_date\":\"" << std::format("{:%Y-%m-%d %H:%M:%S}", _loanEndDate) << "\",";
    oss << "\"account_status\":\"" << sdrs::constants::accountStatusToString(_accountStatus) << "\",";
    oss << "\"days_past_due\":" << _daysPastDue << ",";
    oss << "\"number_of_missed_payments\":" << _numberOfMissedPayments << ",";
    oss << "\"created_at\":\"" << std::format("{:%Y-%m-%d %H:%M:%S}", _createdAt) << "\",";
    oss << "\"updated_at\":\"" << std::format("{:%Y-%m-%d %H:%M:%S}", _updatedAt) << "\"";
    oss << "}";
    return oss.str();
}

LoanAccount LoanAccount::fromJson(const std::string& json)
{
    auto j = nlohmann::json::parse(json);
    
    int accountId = j.value("account_id", j.value("accountId", 0));
    int borrowerId = j.value("borrower_id", j.value("borrowerId", 0));
    double loanAmount = j["loan_amount"].get<double>();
    double interestRate = j["interest_rate"].get<double>();
    
    // Calculate loan term from start/end dates if provided, otherwise default to 12 months
    int loanTermMonths = 12;
    if (j.contains("loan_start_date") && j.contains("loan_end_date"))
    {
        // For MVP, simplified calculation
        loanTermMonths = 12; // Can be enhanced later
    }
    
    LoanAccount account(accountId, borrowerId, loanAmount, interestRate, loanTermMonths);
    
    // Update optional fields from schema
    if (j.contains("remaining_amount") && !j["remaining_amount"].is_null())
    {
        double remaining = j["remaining_amount"].get<double>();
        account._remainingAmount = sdrs::money::Money(remaining);
    }
    
    if (j.contains("days_past_due") && !j["days_past_due"].is_null())
    {
        account._daysPastDue = j["days_past_due"].get<int>();
    }
    
    if (j.contains("number_of_missed_payments") && !j["number_of_missed_payments"].is_null())
    {
        account._numberOfMissedPayments = j["number_of_missed_payments"].get<int>();
    }
    
    if (j.contains("account_status") && !j["account_status"].is_null())
    {
        std::string status = j["account_status"].get<std::string>();
        account._accountStatus = sdrs::constants::stringToAccountStatus(status);
    }
    
    return account;
}

} 
