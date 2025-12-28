// Borrower.cpp - Implementation

#include "../../include/models/Borrower.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <regex>
#include <cmath>
#include <sstream>
#include <nlohmann/json.hpp>

using namespace sdrs::constants;
using namespace sdrs::exceptions;

namespace sdrs::borrower
{

Borrower::Borrower(int id,
    const std::string& fname,
    const std::string& lname)
    : _id(id), _firstName(fname), _lastName(lname)
{
    _isActive = true;
    _createdAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

void Borrower::setEmail(const std::string& email)
{
    static const std::regex emailPattern(validation::EMAIL_PATTERN);

    if (!std::regex_match(email, emailPattern))
    {
        throw ValidationException("Invalid email format: ", "email");
    }
    _email = email;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

void Borrower::setMonthlyIncome(double income)
{
    if (income < 0)
    {
        throw ValidationException("Income cannot be negative", "monthlyIncome");
    }

    if (std::isnan(income) || std::isinf(income))
    {
        throw ValidationException("Income must be a valid number", "monthlyIncome");
    }

    _monthlyIncome = income;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

void Borrower::setActive()
{
    _isActive = true;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

void Borrower::setInactive(InactiveReason reason)
{
    _isActive = false;
    _inactiveReason = reason;
    _inactivatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

void Borrower::setEmploymentStatus(EmploymentStatus status)
{
    _employmentStatus = status;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

InactiveReason Borrower::getInactiveReason() const
{
    return _inactiveReason;
}

int Borrower::getId() const
{
    return _id;
}

std::string Borrower::getFirstName() const
{
    return _firstName;
}

std::string Borrower::getLastName() const
{
    return _lastName;
}

std::string Borrower::getFullName() const
{
    return _firstName + " " + _lastName;
}

std::chrono::sys_seconds Borrower::getInactiveAt() const
{
    return _inactivatedAt;
}

EmploymentStatus Borrower::getEmploymentStatus() const
{
    return _employmentStatus;
}

bool Borrower::isActive() const
{
    return _isActive;
}

bool Borrower::canContact() const
{
    return !_email.empty() && _isActive;
}

std::chrono::sys_seconds Borrower::getCreatedAt() const
{
    return _createdAt;
}

std::chrono::sys_seconds Borrower::getUpdatedAt() const
{
    return _updatedAt;
}

std::string Borrower::getMaskedEmail() const
{
    if (_email.empty())
    {
        return "";
    }

    size_t atPos = _email.find('@');
    if (atPos == std::string::npos || atPos == 0)
    {
        return _email;
    }

    return _email.substr(0, 1) + "*****" + _email.substr(atPos);
}

std::string Borrower::getEmail() const
{
    return _email;
}

bool Borrower::hasEmail() const
{
    return !_email.empty();
}

bool Borrower::hasValidIncome() const
{
    return _monthlyIncome > 0;
}

bool Borrower::canSendCommunication() const
{
    return (_isActive)
        && (!_email.empty());
}

bool Borrower::canApplyRecoveryStrategy() const
{
    return (_isActive) 
        && (hasCompleteProfile())
        && (_monthlyIncome > 0);
}

bool Borrower::hasCompleteProfile() const
{
    return (_id > 0)
        && (!_firstName.empty())
        && (!_lastName.empty())
        && (!_email.empty());
}

void Borrower::setPhoneNumber(const std::string& phoneNumber)
{
    std::regex phoneNumberPattern(validation::PHONE_PATTERN);

    if (!std::regex_match(phoneNumber, phoneNumberPattern))
    {
        throw ValidationException("Invalid phone number format", "phoneNumber");
    }
    _phoneNumber = phoneNumber;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

std::string Borrower::getPhoneNumber() const
{
    return _phoneNumber;
}

double Borrower::getMonthlyIncome() const
{
    return _monthlyIncome;
}

void Borrower::setAddress(const std::string& address)
{
    _address = address;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

std::string Borrower::getAddress() const
{
    return _address;
}

void Borrower::setFirstName(const std::string& fname)
{
    _firstName = fname;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

void Borrower::setLastName(const std::string& lname)
{
    _lastName = lname;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

std::chrono::year_month_day Borrower::getDateOfBirth() const
{
    return std::chrono::year_month_day{_dateOfBirth};
}

void Borrower::setDateOfBirth(const std::string& dobString)
{
    std::istringstream iss(dobString);
    std::chrono::sys_days tmp;

    std::chrono::from_stream(iss, "%Y-%m-%d", tmp);

    if (iss.fail())
    {
        throw ValidationException("Invalid date format. Expected YYYY-MM-DD", "dateOfBirth");
    }

    _dateOfBirth = tmp;
    _updatedAt = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

std::string Borrower::toJson() const
{
    nlohmann::json j;
    j["borrower_id"] = _id;
    j["first_name"] = _firstName;
    j["last_name"] = _lastName;
    j["email"] = _email;
    j["phone_number"] = _phoneNumber;
    j["address"] = _address;
    j["date_of_birth"] = std::format("{:%Y-%m-%d}", _dateOfBirth);
    j["employment_status"] = sdrs::constants::employmentStatusToString(_employmentStatus);
    j["monthly_income"] = _monthlyIncome;
    j["is_active"] = _isActive;
    j["inactive_reason"] = sdrs::constants::inactiveReasonToString(_inactiveReason);
    j["created_at"] = std::format("{:%Y-%m-%d %H:%M:%S}", _createdAt);
    j["updated_at"] = std::format("{:%Y-%m-%d %H:%M:%S}", _updatedAt);
    if (!_isActive && _inactivatedAt.time_since_epoch().count() > 0)
    {
        j["inactivated_at"] = std::format("{:%Y-%m-%d %H:%M:%S}", _inactivatedAt);
    }
    
    return j.dump();
}

Borrower Borrower::fromJson(const std::string& json)
{
    auto j = nlohmann::json::parse(json);
    
    // Required fields
    int id = j.value("borrower_id", j.value("id", 0));
    std::string firstName = j["first_name"].get<std::string>();
    std::string lastName = j["last_name"].get<std::string>();
    
    Borrower borrower(id, firstName, lastName);
    
    // Optional fields from schema
    if (j.contains("email") && !j["email"].is_null())
    {
        borrower.setEmail(j["email"].get<std::string>());
    }
    
    if (j.contains("phone_number") && !j["phone_number"].is_null())
    {
        borrower.setPhoneNumber(j["phone_number"].get<std::string>());
    }
    
    if (j.contains("address") && !j["address"].is_null())
    {
        borrower.setAddress(j["address"].get<std::string>());
    }
    
    if (j.contains("date_of_birth") && !j["date_of_birth"].is_null())
    {
        borrower.setDateOfBirth(j["date_of_birth"].get<std::string>());
    }
    
    if (j.contains("employment_status") && !j["employment_status"].is_null())
    {
        std::string status = j["employment_status"].get<std::string>();
        borrower.setEmploymentStatus(sdrs::constants::stringToEmploymentStatus(status));
    }
    
    if (j.contains("monthly_income") && !j["monthly_income"].is_null())
    {
        borrower.setMonthlyIncome(j["monthly_income"].get<double>());
    }
    
    if (j.contains("is_active") && !j["is_active"].is_null())
    {
        bool active = j["is_active"].get<bool>();
        if (active)
        {
            borrower.setActive();
        }
        else if (j.contains("inactive_reason") && !j["inactive_reason"].is_null())
        {
            std::string reason = j["inactive_reason"].get<std::string>();
            borrower.setInactive(sdrs::constants::stringToInactiveReason(reason));
        }
    }
    
    return borrower;
}

}
