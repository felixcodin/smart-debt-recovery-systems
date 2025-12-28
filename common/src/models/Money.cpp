// Money.cpp - Implementation

#include "../../include/models/Money.h"
#include "../../include/exceptions/ValidationException.h"

#include <sstream>
#include <locale>
#include <cmath>
#include <iomanip>
#include <nlohmann/json.hpp>

using namespace sdrs::exceptions;
using namespace sdrs::constants;

namespace sdrs::money
{

struct VietnamNumpunct : public std::numpunct<char>
{
protected:
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
    char do_decimal_point() const override { return ','; }
    
};

struct USDNumpunct : public std::numpunct<char>
{
protected:
    char do_thousands_sep() const override { return ','; }
    std::string do_grouping() const override { return "\3"; }

};

Money::Money(double amt, MoneyType moneyType) : _amount(amt), _moneyType(moneyType)
{
    if (amt < 0)
    {
        throw ValidationException("Money cannot be negative", "Money");
    }
}

double Money::getAmount() const
{
    return _amount;
}

MoneyType Money::getMoneyType() const
{
    return _moneyType; 
}

std::string Money::format() const
{
    std::stringstream ss;

    if (_moneyType == MoneyType::VND)
    {
        std::locale vnLocale(std::locale::classic(), new VietnamNumpunct);
        ss.imbue(vnLocale);
    
        ss << std::fixed << std::setprecision(currency::DECIMAL_PLACES) << _amount;
    
        return ss.str() + " VND";
    }
    std::locale usdLocale(std::locale::classic(), new USDNumpunct);
    ss.imbue(usdLocale);

    ss << std::fixed << std::setprecision(2) << _amount;

    return "$" + ss.str();
}

void Money::setAmount(double amt)
{
    if (amt < 0)
    {
        throw ValidationException("Money cannot be negative", "Money");
    }
    _amount = amt;
}

Money Money::operator+(const Money& other) const
{
    if (_moneyType != other.getMoneyType())
    {
        throw ValidationException("Cannot add two different currencies together", "Money");
    }
    return Money(_amount + other.getAmount(), _moneyType);
}

Money Money::operator-(const Money& other) const
{
    if (_moneyType != other.getMoneyType())
    {
        throw ValidationException("It is not possible to subtract two different currencies", "Money");
    }
    double result = _amount - other.getAmount();
    if (result < 0)
    {
        throw ValidationException("Insufficient funds", "Money");
    }
    return Money(result, _moneyType);
}

Money Money::operator*(double factor) const
{
    if (factor < 0)
    {
        throw ValidationException("Multiplier cannot be negative", "Money");
    }
    return Money(_amount * factor, _moneyType);
}

Money Money::operator/(double divisor) const
{
    if (divisor == 0)
    {
        throw ValidationException("Division by zero", "Money");
    }
    if (divisor < 0)
    {
        throw ValidationException("Divisor cannot be negative", "Money");
    }
    return Money(_amount / divisor, _moneyType);
}

bool Money::operator==(const Money& other) const
{
    if (_moneyType != other.getMoneyType())
    {
        throw ValidationException("It is impossible to compare two different currencies", "Money");
    }
    return std::abs(_amount - other.getAmount()) < currency::EPSILON;
}

bool Money::operator<(const Money& other) const
{
    if (_moneyType != other.getMoneyType())
    {
        throw ValidationException("It is impossible to compare two different currencies", "Money");
    }
    return _amount < other.getAmount() - currency::EPSILON;
}

bool Money::operator>(const Money& other) const
{
    if (_moneyType != other.getMoneyType())
    {
        throw ValidationException("It is impossible to compare two different currencies", "Money");
    }
    return _amount > other.getAmount() + currency::EPSILON;
}

Money& Money::operator+=(const Money& other)
{
    if (_moneyType != other.getMoneyType())
    {
        throw ValidationException("It is impossible to compare two different currencies", "Money");
    }
    _amount += other.getAmount();
    return *this;
}

Money& Money::operator-=(const Money& other)
{
    if (_moneyType != other.getMoneyType())
    {
        throw ValidationException("It is impossible to compare two different currencies", "Money");
    }
    double newAmount = _amount - other.getAmount();
    if (newAmount < 0)
    {
        throw ValidationException("Insufficient funds", "Money");
    }
    _amount = newAmount;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const Money& money)
{
    os << money.format();
    return os;
}

std::istream& operator>>(std::istream& is, Money& money)
{
    double amount;
    is >> amount;

    if (amount < 0)
    {
        throw ValidationException("Money cannot be negative", "Money");
    }

    money._amount = amount;
    return is;
}

std::string Money::toString() const
{
    return format();
}

std::string Money::toJson() const
{
    nlohmann::json j;
    j["amount"] = _amount;
    j["currency"] = (_moneyType == MoneyType::VND) ? "VND" : "USD";
    return j.dump();
}

Money Money::fromJson(const std::string& json)
{
    auto j = nlohmann::json::parse(json);
    
    double amount = j["amount"].get<double>();
    
    MoneyType type = MoneyType::VND;
    if (j.contains("currency") && !j["currency"].is_null())
    {
        std::string currency = j["currency"].get<std::string>();
        type = (currency == "USD") ? MoneyType::USD : MoneyType::VND;
    }
    
    return Money(amount, type);
}

bool Money::operator>=(const Money& other) const
{
    return !(*this < other);
}

bool Money::operator<=(const Money& other) const
{
    return !(*this > other);
}

bool Money::operator!=(const Money& other) const
{
    return !(*this == other);
}

}