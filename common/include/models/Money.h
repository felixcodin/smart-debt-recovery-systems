#ifndef MONEY_H
#define MONEY_H

#include <iostream>
#include <string>

namespace sdrs::money   
{

class Money
{
private:
    double _amount;
    static constexpr double EPSILON = 0.01;

public:
    Money(double amt = 0.0);

    double getAmount() const;
    std::string format() const;
    std::string formatUSD() const;
    void setAmount(double amt);

    Money operator+(const Money& other) const;
    Money operator-(const Money& other) const;
    Money operator*(double factor) const;
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

};



}


#endif