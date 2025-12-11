#ifndef SDRS_BORROWER_LOANACCOUNT_H
#define SDRS_BORROWER_LOANACCOUNT_H

#include <string>
#include "exceptions/ValidationException.h"

namespace sdrs::borrower {

class LoanAccount {
public:
    // ----- Status constants -----
    static inline const std::string STATUS_ON_TIME      = "ON_TIME";
    static inline const std::string STATUS_DELAYED      = "DELAYED";
    static inline const std::string STATUS_PARTIAL      = "PARTIAL";
    static inline const std::string STATUS_WRITTEN_OFF  = "WRITTEN_OFF";

    // ----- Constructor -----
    LoanAccount(int id, int borrower_id,
                double loan_amt, double rate);

    // ----- Business Logic -----
    void updateStatus(const std::string& new_status);
    void markPaymentMissed();
    void recordPayment(double amount);

    // ----- Getters -----
    int getAccountId() const { return account_id; }
    int getBorrowerId() const { return borrower_id; }
    double getLoanAmount() const { return loan_amount; }
    double getRemainingAmount() const { return remaining_amount; }
    double getInterestRate() const { return interest_rate; }
    int getMissedPayments() const { return number_of_missed_payments; }
    int getDaysPastDue() const { return days_past_due; }
    const std::string& getStatus() const { return account_status; }

private:
    // ----- Validation helpers -----
    bool canUpdateStatus(const std::string& from_status,
                         const std::string& to_status) const;

private:
    // ----- Core attributes -----
    int account_id;
    int borrower_id;

    double loan_amount;
    double remaining_amount;
    double interest_rate;

    // ----- Delinquency tracking -----
    int number_of_missed_payments = 0;
    int days_past_due = 0;

    // ----- Status -----
    std::string account_status = STATUS_ON_TIME;
};

} // namespace sdrs::borrower

#endif // SDRS_BORROWER_LOANACCOUNT_H
