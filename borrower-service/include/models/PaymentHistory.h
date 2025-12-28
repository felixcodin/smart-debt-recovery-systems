// PaymentHistory.h - Domain entity for tracking individual payment records

#ifndef SDRS_BORROWER_PAYMENTHISTORY_H
#define SDRS_BORROWER_PAYMENTHISTORY_H

#include "../../../common/include/models/Money.h"
#include <string>
#include <optional>
#include <chrono>


namespace sdrs::borrower
{

class PaymentHistory
{
private:
    int _paymentId;
    int _accountId;
    sdrs::money::Money _paymentAmount; 
    sdrs::constants::PaymentMethod _method;
    sdrs::constants::PaymentStatus _status;
    std::chrono::sys_days _paymentDate;
    std::optional<std::chrono::sys_days> _dueDate;
    std::string _notes;
    std::chrono::sys_seconds _createdAt;
    std::chrono::sys_seconds _updatedAt;

private:
    void touch();

public:
    PaymentHistory(int paymentId,int accountId,const sdrs::money::Money& paymentAmount, sdrs::constants::PaymentMethod method,
        std::chrono::sys_days paymentDate,std::optional<std::chrono::sys_days> dueDate = std::nullopt);

public:
    int getPaymentId() const;
    int getAccountId() const;
    const sdrs::money::Money& getPaymentAmount() const; 
    sdrs::constants::PaymentMethod getMethod() const;
    sdrs::constants::PaymentStatus getStatus() const;
    std::chrono::sys_days getPaymentDate() const;
    std::optional<std::chrono::sys_days> getDueDate() const;
    std::string getNotes() const;

public:
    bool isLate() const;
    bool isTerminalStatus() const;

public:
    void markCompleted();
    void markFailed(const std::string& reason);
    void cancel(const std::string& reason);
    void setNotes(const std::string& notes);

public:
    bool isValid() const;
    std::string validate() const;

public:
    std::string toJson() const;
    static PaymentHistory fromJson(const std::string& json);
    static std::string paymentStatusToString(sdrs::constants::PaymentStatus status);
    static std::string paymentMethodToString(sdrs::constants::PaymentMethod method);
    static sdrs::constants::PaymentStatus stringToPaymentStatus(const std::string& value);
    static sdrs::constants::PaymentMethod stringToPaymentMethod(const std::string& value);
};
} 
#endif 
