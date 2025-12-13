#ifndef SDRS_BORROWER_LOANACCOUNT_H
#define SDRS_BORROWER_LOANACCOUNT_H

#include <string>
#include <chrono>

namespace sdrs::borrower
{
enum class AccountStatus
{
    Current,
    Delinquent,
    Partial,
    Default,
    PaidOff,
    ChargedOff,
    Settled
};

class LoanAccount
{
private:
    int _accountId;
    int _borrowerId;
    double _loanAmount;
    double _initialAmount;
    double _remainingAmount;
    double _interestRate;
    AccountStatus _accountStatus = AccountStatus::Current;
    int _daysPastDue = 0;
    int _numberOfMissedPayments = 0;
    std::chrono::sys_seconds _loanStartDate;
    std::chrono::sys_seconds _loanEndDate;
    std::chrono::sys_seconds _createdAt;
    std::chrono::sys_seconds _updatedAt;
    bool _validAmounts = true;
    bool _validInterest = true;
    static constexpr int PAYMENT_CYCLE_DAYS = 30;

private:
    void validateConstructorArgs(int accountId,int borrowerId,double loanAmount,double interestRate) const;
    void touch();

public:
    LoanAccount() = delete;
    LoanAccount(int accountId,int borrowerId,double loanAmount,double interestRate);

public:
    int getAccountId() const;
    int getBorrowerId() const;
    double getLoanAmount() const;
    double getInitialAmount() const;
    double getRemainingAmount() const;
    double getInterestRate() const;
    AccountStatus getStatus() const;
    int getDaysPastDue() const;
    int getMissedPayments() const;
    std::chrono::sys_seconds getLoanStartDate() const;
    std::chrono::sys_seconds getLoanEndDate() const;
    std::chrono::sys_seconds getCreatedAt() const;
    std::chrono::sys_seconds getUpdatedAt() const;
    bool isValidAmounts() const;
    bool isValidInterest() const;

public:
    void setRemainingAmount(double amount);
    void setInterestRate(double rate);
    void setDaysPastDue(int days);
    void setMissedPayments(int missedPayments);
    void setLoanEndDate(std::chrono::sys_seconds endDate);
    void setUpdatedAt(std::chrono::sys_seconds updatedAt);
    void setValidAmounts(bool valid);
    void setValidInterest(bool valid);

public:
    void updateStatus(AccountStatus newStatus);
    bool canUpdateStatus(AccountStatus fromStatus,AccountStatus toStatus) const;
    void markPaymentMissed();
    void recordPayment(double amount);
    void incrementDaysPastDue(int days);
    bool isTerminalStatus() const;
    bool isFullyPaid() const;

public:
    std::string toJson() const;
    static std::string statusToString(AccountStatus status);
    static AccountStatus stringToStatus(const std::string& statusStr);
};
}
#endif
