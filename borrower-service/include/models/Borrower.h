#ifndef BORROWER_H
#define BORROWER_H

#include <string>
#include <chrono>

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

enum class EmploymentStatus
{
    Employed,
    Unemployed,
    SelfEmployed,
    Student,
    Retired,
    Contract,
    PartTime,
    None
};

class Borrower
{
private:
    int _id = 0;
    std::string _firstName;
    std::string _lastName;
    std::string _email;
    std::string _phoneNumber;
    std::string _address;
    std::chrono::sys_days _dateOfBirth;
    EmploymentStatus _employmentStatus = EmploymentStatus::None;
    bool _isActive = false;
    std::chrono::sys_seconds _createdAt;
    std::chrono::sys_seconds _updatedAt;
    double _monthlyIncome = 0.0;
    InactiveReason _inactiveReason = InactiveReason::None;
    std::chrono::sys_seconds _inactivatedAt;

public:
    Borrower() = delete;
    Borrower(int id, const std::string& fname, const std::string& lname);

public:
    void setEmail(const std::string& email);
    void setMonthlyIncome(double income);
    void setActive();
    void setInactive(InactiveReason reason);
    void setPhoneNumber(const std::string& phoneNumber);
    void setAddress(const std::string& address);
    void setDateOfBirth(const std::string& dobString);
    void setFirstName(const std::string& fname);
    void setLastName(const std::string& lname);
    void setEmploymentStatus(EmploymentStatus status);

public:
    int getId() const;
    std::string getFirstName() const;
    std::string getLastName() const;
    std::string getFullName() const;
    std::chrono::sys_seconds getCreatedAt() const;
    std::chrono::sys_seconds getUpdatedAt() const;
    std::chrono::sys_seconds getInactiveAt() const;
    std::string getMaskedEmail() const;
    InactiveReason getInactiveReason() const;
    std::string getPhoneNumber() const;
    std::string getAddress() const;
    double getMonthlyIncome() const;
    std::chrono::year_month_day getDateOfBirth() const;
    EmploymentStatus getEmploymentStatus() const;

public:
    bool hasEmail() const;
    bool hasValidIncome() const;
    bool isActive() const;
    bool canContact() const;
    bool canSendCommunication() const;
    bool canApplyRecoveryStrategy() const;
    bool hasCompleteProfile() const;

public:
    std::string toJson() const;
    // static Borrower fromJson(const std::string& json);

};

}
#endif