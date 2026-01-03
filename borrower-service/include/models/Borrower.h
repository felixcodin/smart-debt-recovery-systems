// Borrower.h - Domain entity representing a loan borrower

#ifndef BORROWER_H
#define BORROWER_H

#include <string>
#include <chrono>
#include "../../../common/include/utils/Constants.h"

namespace sdrs::borrower
{

// Risk segmentation based on K-Means clustering
enum class RiskSegment
{
    Unclassified,  // Not yet clustered
    Low,           // Cluster 0 - Low risk borrowers
    Medium,        // Cluster 1 - Medium risk borrowers
    High           // Cluster 2 - High risk borrowers
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
    sdrs::constants::EmploymentStatus _employmentStatus = sdrs::constants::EmploymentStatus::None;
    bool _isActive = false;
    std::chrono::sys_seconds _createdAt;
    std::chrono::sys_seconds _updatedAt;
    double _monthlyIncome = 0.0;
    sdrs::constants::InactiveReason _inactiveReason = sdrs::constants::InactiveReason::None;
    std::chrono::sys_seconds _inactivatedAt;
    RiskSegment _riskSegment = RiskSegment::Unclassified;  // K-Means cluster assignment

public:
    Borrower() = delete;
    Borrower(int id, const std::string& fname, const std::string& lname);

    // Setters with validation
public:
    void setEmail(const std::string& email);  // validates email format
    void setMonthlyIncome(double income);     // must be positive
    void setActive();
    void setInactive(sdrs::constants::InactiveReason reason);
    void setPhoneNumber(const std::string& phoneNumber);
    void setAddress(const std::string& address);
    void setDateOfBirth(const std::string& dobString);  // format: YYYY-MM-DD
    void setFirstName(const std::string& fname);
    void setLastName(const std::string& lname);
    void setEmploymentStatus(sdrs::constants::EmploymentStatus status);
    void assignSegment(RiskSegment segment);  // Assign K-Means cluster

public:
    int getId() const;
    std::string getFirstName() const;
    std::string getLastName() const;
    std::string getFullName() const;
    std::chrono::sys_seconds getCreatedAt() const;
    std::chrono::sys_seconds getUpdatedAt() const;
    std::chrono::sys_seconds getInactiveAt() const;
    std::string getEmail() const;
    std::string getMaskedEmail() const;
    sdrs::constants::InactiveReason getInactiveReason() const;
    std::string getPhoneNumber() const;
    std::string getAddress() const;
    double getMonthlyIncome() const;
    std::chrono::year_month_day getDateOfBirth() const;
    std::string getDateOfBirthString() const;  // Returns YYYY-MM-DD format
    int getAge() const;  // Calculate age from date of birth
    RiskSegment getRiskSegment() const;  // Get K-Means cluster assignment
    sdrs::constants::EmploymentStatus getEmploymentStatus() const;

    // Business rule checks
public:
    bool hasEmail() const;
    bool hasValidIncome() const;            // income > 0
    bool isActive() const;
    bool canContact() const;                // has phone or email
    bool canSendCommunication() const;      // active + contactable
    bool canApplyRecoveryStrategy() const;  // active + has income
    bool hasCompleteProfile() const;        // all required fields filled

public:
    std::string toJson() const;
    static Borrower fromJson(const std::string& json);

};

}
#endif