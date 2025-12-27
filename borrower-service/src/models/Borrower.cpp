// Borrower.cpp - Implementation

#include "../../include/models/Borrower.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <regex>
#include <cmath>
#include <sstream>

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
    std::ostringstream oss;
    oss << "{"
        << "\"id\":" << _id << ","
        << "\"firstName\":\"" << _firstName << "\","
        << "\"lastName\":\"" << _lastName << "\","
        << "\"email\":\"" << _email << "\","
        << "\"isActive\":" << (_isActive ? "true" : "false")
        << "}";

    return oss.str();
}

/*
Borrower Borrower::fromJson(const std::string& json)
{
    // TODO:
    // Parser JSON string
    // Extract fields
    // Create and return Borrower object
}
*/

}