#include "../../include/models/Borrower.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <regex>
#include <cmath>

namespace sdrs::borrower
{

Borrower::Borrower(int id,
    const std::string& fname,
    const std::string& lname)
    : _id(id), _firstName(fname), _lastName(lname)
{
    _isActive = true;
    time(&_createdAt);
    time(&_updatedAt);

}

void Borrower::setEmail(const std::string& email)
{
    static const std::regex emailPattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");

    if (!std::regex_match(email, emailPattern))
    {
        throw sdrs::exceptions::ValidationException("Invalid email format: ", "email");
    }
    _email = email;
    time(&_updatedAt);
}

void Borrower::setMonthlyIncome(double income)
{
    if (income < 0)
    {
        throw sdrs::exceptions::ValidationException("Income cannot be negative", "monthlyIncome");
    }

    if (std::isnan(income) || std::isinf(income))
    {
        throw sdrs::exceptions::ValidationException("Income must be a valid number", "monthlyIncome");
    }

    _monthlyIncome = income;
    time(&_updatedAt);
}

void Borrower::setActive()
{
    _isActive = true;
    time(&_updatedAt);
}

void Borrower::setInactive(InactiveReason reason)
{
    _isActive = false;
    _inactiveReason = reason;
    time(&_inactivatedAt);
    time(&_updatedAt);
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

bool Borrower::isActive() const
{
    return _isActive;
}

bool Borrower::canContact() const
{
    return !_email.empty() && _isActive;
}

time_t Borrower::getCreatedAt() const
{
    return _createdAt;
}

time_t Borrower::getUpdatedAt() const
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

}