// LoanAccount.h - Domain entity representing a loan account with payment tracking

#ifndef SDRS_BORROWER_LOANACCOUNT_H
#define SDRS_BORROWER_LOANACCOUNT_H

#include "../../../common/include/models/Money.h"
#include <string>
#include <chrono>

namespace sdrs::borrower
{

class LoanAccount
{
private:
    int _accountId;
    int _borrowerId;
    sdrs::money::Money _loanAmount;
    sdrs::money::Money _initialAmount;
    sdrs::money::Money _remainingAmount;
    double _interestRate;
    sdrs::money::Money _monthlyPaymentAmount;
    sdrs::money::Money _totalPaidAmount;
    sdrs::money::Money _lateFees;
    int _loanTermMonths;
    std::chrono::sys_seconds _lastPaymentDate{};
    std::chrono::sys_seconds _nextPaymentDueDate{};
    sdrs::constants::AccountStatus _accountStatus{sdrs::constants::AccountStatus::Current};
    int _daysPastDue{0};
    int _numberOfMissedPayments{0};
    std::chrono::sys_seconds _loanStartDate;
    std::chrono::sys_seconds _loanEndDate;
    std::chrono::sys_seconds _createdAt;
    std::chrono::sys_seconds _updatedAt;
    static constexpr int PAYMENT_CYCLE_DAYS = 30;

private:
    void validateConstructorArgs(int accountId,int borrowerId,double loanAmount,double interestRate,int loanTermMonths) const;
    void touch();
    void recalculateMonthlyPayment();
    void updateNextPaymentDueDate();

public:
    LoanAccount() = delete;
    LoanAccount(int accountId,int borrowerId,double loanAmount,double interestRate,int loanTermMonths);
    int getAccountId() const;
    int getBorrowerId() const;
    sdrs::money::Money getLoanAmount() const;
    sdrs::money::Money getInitialAmount() const;
    sdrs::money::Money getRemainingAmount() const;
    sdrs::money::Money getMonthlyPaymentAmount() const;
    sdrs::money::Money getTotalPaidAmount() const;
    sdrs::money::Money getLateFees() const;
    double getInterestRate() const;
    int getLoanTermMonths() const;
    sdrs::constants::AccountStatus getStatus() const;
    int getDaysPastDue() const;
    int getMissedPayments() const;
    std::chrono::sys_seconds getLoanStartDate() const;
    std::chrono::sys_seconds getLoanEndDate() const;
    std::chrono::sys_seconds getCreatedAt() const;
    std::chrono::sys_seconds getUpdatedAt() const;
    std::chrono::sys_seconds getLastPaymentDate() const;
    std::chrono::sys_seconds getNextPaymentDueDate() const;

    // Status and payment operations
public:
    void updateStatus(sdrs::constants::AccountStatus newStatus);  // validates transition
    bool canUpdateStatus(sdrs::constants::AccountStatus fromStatus,sdrs::constants::AccountStatus toStatus) const;
    void markPaymentMissed();      // increments DPD, adds late fees
    void recordPayment(const sdrs::money::Money& amount);  // reduces remaining balance
    void incrementDaysPastDue(int days);
    bool isTerminalStatus() const; // PaidOff, ChargedOff, Settled
    bool isFullyPaid() const;      // remaining == 0

public:
    std::string toJson() const;
    static std::string statusToString(sdrs::constants::AccountStatus status);
    static sdrs::constants::AccountStatus stringToStatus(const std::string& statusStr);
};
} 
#endif
