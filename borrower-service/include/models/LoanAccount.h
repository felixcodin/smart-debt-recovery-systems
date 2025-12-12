#ifndef SDRS_BORROWER_LOANACCOUNT_H
#define SDRS_BORROWER_LOANACCOUNT_H

#include <string>

namespace sdrs::borrower 
{
class LoanAccount
{
private:
    int _accountId;
    int _borrowerId;
    double _loanAmount;
    double _remainingAmount;
    double _interestRate;
    int _numberOfMissedPayments = 0;
    int _daysPastDue = 0;
    std::string _accountStatus = STATUS_ON_TIME;

public:
    static inline const std::string STATUS_ON_TIME     = "ON_TIME";
    static inline const std::string STATUS_DELAYED     = "DELAYED";
    static inline const std::string STATUS_PARTIAL     = "PARTIAL";
    static inline const std::string STATUS_WRITTEN_OFF = "WRITTEN_OFF";
    LoanAccount(int id, int borrower_id, double loan_amt, double rate);

public:
    void updateStatus(const std::string& new_status);
    void markPaymentMissed();
    void recordPayment(double amount);

public:
    int getAccountId() const;
    int getBorrowerId() const; 
    double getLoanAmount() const;
    double getRemainingAmount() const;
    double getInterestRate() const;
    int getMissedPayments() const;
    int getDaysPastDue() const;
    const std::string& getStatus() const;

private:
    bool canUpdateStatus(const std::string& from_status, const std::string& to_status) const;
};

}

#endif
