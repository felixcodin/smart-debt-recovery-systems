#include "../../include/models/Money.h"
#include "../../include/exceptions/ValidationException.h"

#include <sstream>
#include <locale>
#include <cmath>
#include <iomanip>

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

Money::Money(double amt) : _amount(amt)
{
    if (amt < 0)
    {
        throw sdrs::exceptions::ValidationException("Money cannot be negative", "Money");
    }
}

double Money::getAmount() const
{
    return _amount;
}

std::string Money::format() const
{
    std::stringstream ss;

    std::locale vnLocale(std::locale::classic(), new VietnamNumpunct);
    ss.imbue(vnLocale);

    ss << std::fixed << std::setprecision(2) << _amount;

    return ss.str() + " VND";
}

std::string Money::formatUSD() const
{
    std::stringstream ss;
    
    std::locale usdLocale(std::locale::classic(), new USDNumpunct);
    ss.imbue(usdLocale);

    ss << std::fixed << std::setprecision(2) << _amount;

    return "$" + ss.str();
}

void Money::setAmount(double amt)
{
    if (amt < 0)
    {
        throw sdrs::exceptions::ValidationException("Money cannot be negative", "Money");
    }
    _amount = amt;
}

Money Money::operator+(const Money& other) const
{
    return Money(_amount + other.getAmount());
}

Money Money::operator-(const Money& other) const
{
    double result = _amount - other.getAmount();
    if (result < 0)
    {
        throw sdrs::exceptions::ValidationException("Insufficient funds", "Money");
    }
    return Money(result);
}

Money Money::operator*(double factor) const
{
    if (factor < 0)
    {
        throw sdrs::exceptions::ValidationException("Multiplier cannot be negative", "Money");
    }
    return Money(_amount * factor);
}

Money Money::operator/(double divisor) const
{
    if (divisor == 0)
    {
        throw sdrs::exceptions::ValidationException("Division by zero", "Money");
    }
    if (divisor < 0)
    {
        throw sdrs::exceptions::ValidationException("Divisor cannot be negative", "Money");
    }
    return Money(_amount / divisor);
}

bool Money::operator==(const Money& other) const
{
    return std::abs(_amount - other.getAmount()) < EPSILON;
}

bool Money::operator<(const Money& other) const
{
    return _amount < other.getAmount() - EPSILON;
}

bool Money::operator>(const Money& other) const
{
    return _amount > other.getAmount() + EPSILON;
}

Money& Money::operator+=(const Money& other)
{
    _amount += other.getAmount();
    return *this;
}

Money& Money::operator-=(const Money& other)
{
    double newAmount = _amount - other.getAmount();
    if (newAmount < 0)
    {
        throw sdrs::exceptions::ValidationException("Insufficient funds", "Money");
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
        throw sdrs::exceptions::ValidationException("Money cannot be negative", "Money");
    }

    money._amount = amount;
    return is;
}

std::string Money::toString() const
{
    return format();
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