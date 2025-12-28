// Money.h - Value object for currency amounts with arithmetic operations

#ifndef MONEY_H
#define MONEY_H

#include <iostream>
#include <string>
#include "../utils/Constants.h"

namespace sdrs::money   
{

class Money
{
private:
    double _amount;
    sdrs::constants::MoneyType _moneyType = sdrs::constants::MoneyType::VND;
    

public:
    explicit Money(
        double amt = 0.0, 
        sdrs::constants::MoneyType moneyType = sdrs::constants::MoneyType::VND
    );

    double getAmount() const;
    sdrs::constants::MoneyType getMoneyType() const;
    std::string format() const;  // "1.000.000 VND" or "$1,000.00"
    void setAmount(double amt);

    // Arithmetic operators (validates same currency)
    Money operator+(const Money& other) const;
    Money operator-(const Money& other) const;
    Money operator*(double factor) const;  // e.g., amount * interestRate
    Money operator/(double divisor) const;

    bool operator==(const Money& other) const;
    bool operator<(const Money& other) const;
    bool operator>(const Money& other) const;
    bool operator>=(const Money& other) const;
    bool operator<=(const Money& other) const;
    bool operator!=(const Money& other) const;

    Money& operator+=(const Money& other);
    Money& operator-=(const Money& other);

    friend std::ostream& operator<<(std::ostream& os, const Money& money);
    friend std::istream& operator>>(std::istream& is, Money& money);

    std::string toString() const;
    std::string toJson() const;
    static Money fromJson(const std::string& json);

};



}


#endif