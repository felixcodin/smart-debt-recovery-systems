#ifndef SDRS_BORROWER_LOANACCOUNT_H
#define SDRS_BORROWER_LOANACCOUNT_H

#include <string>
#include <ctime>

namespace sdrs::borrower
{

enum class AccountStatus
{
    OnTime,
    Delayed,
    Partial,
    WrittenOff
};

class LoanAccount
{
private:
    int _accountId = 0;
    int _borrowerId = 0;
    double _loanAmount = 0;
    double _remainingAmount = 0;
    double _interestRate = 0;
    AccountStatus _accountStatus = AccountStatus::OnTime;
    int _daysPastDue = 0;
    int _numberOfMissedPayments = 0;
    time_t _createdAt = 0;
    time_t _updatedAt = 0;
    static constexpr int PAYMENT_CYCLE_DAYS = 30;

private:
    static std::string formatTime(time_t t);

public:
    LoanAccount(int id = 0, int borrower_id = 0,double loan_amt = 0, double rate = 0);

public:
    int getAccountId() const;
    int getBorrowerId() const;
    double getLoanAmount() const;
    double getRemainingAmount() const;
    double getInterestRate() const;
    AccountStatus getStatus() const;
    int getDaysPastDue() const;
    int getMissedPayments() const;
    std::string getCreatedAt() const;
    std::string getUpdatedAt() const;

public:
    void updateStatus(AccountStatus newStatus);
    bool canUpdateStatus(AccountStatus fromStatus, AccountStatus toStatus) const;
    void markPaymentMissed();
    void recordPayment(double amount);
    void incrementDaysPastDue(int days);

public:
    bool isValid() const;
    std::string validate() const;
    static std::string statusToString(AccountStatus status);
    static AccountStatus stringToStatus(const std::string& str);

};
}

#endif
