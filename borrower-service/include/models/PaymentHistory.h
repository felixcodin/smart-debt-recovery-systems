#ifndef SDRS_BORROWER_PAYMENTHISTORY_H
#define SDRS_BORROWER_PAYMENTHISTORY_H

#include <string>
#include <optional>
#include <chrono>

namespace sdrs::borrower
{
enum class PaymentStatus
{
    Pending,
    Completed,
    Failed,
    Cancelled
};
enum class PaymentMethod
{
    BankTransfer,
    Cash,
    Card,
    Other
};

class PaymentHistory
{
private:
    int _paymentId;
    int _accountId;
    double _paymentAmount;
    PaymentMethod _method;
    PaymentStatus _status;
    std::chrono::sys_days _paymentDate;
    std::optional<std::chrono::sys_days> _dueDate;
    std::string _notes;
    std::chrono::sys_seconds _createdAt;
    std::chrono::sys_seconds _updatedAt;

private:
    void touch();

public:
    PaymentHistory(int paymentId,int accountId,double paymentAmount,PaymentMethod method
        ,std::chrono::sys_days paymentDate,std::optional<std::chrono::sys_days> dueDate = std::nullopt);

public:
    int getPaymentId() const;
    int getAccountId() const;
    double getPaymentAmount() const;
    PaymentMethod getMethod() const;
    PaymentStatus getStatus() const;
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
    static std::string paymentStatusToString(PaymentStatus status);
    static std::string paymentMethodToString(PaymentMethod method);
    static PaymentStatus stringToPaymentStatus(const std::string& value);
    static PaymentMethod stringToPaymentMethod(const std::string& value);
};

}

#endif // SDRS_BORROWER_PAYMENTHISTORY_H
