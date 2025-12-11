#ifndef BORROWER_H
#define BORROWER_H

#include <string>
#include <ctime>

namespace sdrs::borrower
{

enum class InactiveReason
{
    PaidOff,
    AccountClosed,
    Deceased,
    Fraud,
    LegalAction,
    None
};

class Borrower
{
private:
    int _id = 0;
    std::string _firstName;
    std::string _lastName;
    std::string _email;
    bool _isActive = false;
    time_t _createdAt = 0;
    time_t _updatedAt = 0;
    double _monthlyIncome = 0.0;
    InactiveReason _inactiveReason = InactiveReason::None;
    time_t _inactivatedAt = 0;

public:
    Borrower() = delete;
    Borrower(int id, const std::string& fname, const std::string& lname);

public:
    void setEmail(const std::string& email);
    void setMonthlyIncome(double income);
    void setActive();
    void setInactive(InactiveReason reason);

public:
    int getId() const;
    std::string getFirstName() const;
    std::string getLastName() const;
    std::string getFullName() const;
    time_t getCreatedAt() const;
    time_t getUpdatedAt() const;
    std::string getMaskedEmail() const;
    InactiveReason getInactiveReason() const;

public:
    bool hasEmail() const;
    bool hasValidIncome() const;
    bool isActive() const;
    bool canContact() const;
    bool canSendCommunication() const;
    bool canApplyRecoveryStrategy() const;
    bool hasCompleteProfile() const;

};

}
#endif