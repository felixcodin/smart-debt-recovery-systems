// PaymentHistory.cpp - Implementation

#include "../../include/models/PaymentHistory.h"
#include "../../../common/include/models/Money.h"
#include "../../../common/include/exceptions/ValidationException.h"
#include "../../../common/include/utils/Constants.h"
#include <sstream>
#include <chrono>
#include <string>
#include <format>
#include <nlohmann/json.hpp>

using namespace sdrs::exceptions;
using namespace sdrs::constants;

namespace sdrs::borrower
{

void PaymentHistory::touch()
{
    _updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

PaymentHistory::PaymentHistory(
    int paymentId,
    int accountId,
    const sdrs::money::Money& paymentAmount,
    PaymentMethod method,
    std::chrono::sys_days paymentDate,
    std::optional<std::chrono::sys_days> dueDate)
    :_paymentId(paymentId),
    _accountId(accountId),
    _paymentAmount(paymentAmount),
    _method(method),
    _status(PaymentStatus::Pending),
    _paymentDate(paymentDate),
    _dueDate(dueDate)
{
    _createdAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    _updatedAt = _createdAt;

    if (!isValid())
    {
        throw ValidationException(validate(), "PaymentHistory");
    }
}

int PaymentHistory::getPaymentId() const
{
    return _paymentId;
}

int PaymentHistory::getAccountId() const
{
    return _accountId;
}

const sdrs::money::Money& PaymentHistory::getPaymentAmount() const
{
    return _paymentAmount;
}

PaymentMethod PaymentHistory::getMethod() const
{
    return _method;
}

PaymentStatus PaymentHistory::getStatus() const
{
    return _status;
}

std::chrono::sys_days PaymentHistory::getPaymentDate() const
{
    return _paymentDate;
}

std::optional<std::chrono::sys_days> PaymentHistory::getDueDate() const
{
    return _dueDate;
}

std::string PaymentHistory::getNotes() const
{
    return _notes;
}

bool PaymentHistory::isLate() const
{
    if (!_dueDate.has_value())
    {
        return false;
    }
    return _paymentDate > _dueDate.value();
}

bool PaymentHistory::isTerminalStatus() const
{
    return _status == PaymentStatus::Completed || _status == PaymentStatus::Failed || _status == PaymentStatus::Cancelled;
}

void PaymentHistory::markCompleted()
{
    if (_status != PaymentStatus::Pending)
    {
        throw ValidationException("Only pending payment can be completed", "paymentStatus");
    }
    _status = PaymentStatus::Completed;
    touch();
}

void PaymentHistory::markFailed(const std::string& reason)
{
    if (_status != PaymentStatus::Pending)
    {
        throw ValidationException("Only pending payment can be failed", "paymentStatus");
    }
    _status = PaymentStatus::Failed;
    _notes = reason;
    touch();
}

void PaymentHistory::cancel(const std::string& reason)
{
    if (_status == PaymentStatus::Completed)
    {
        throw ValidationException("Completed payment cannot be cancelled", "paymentStatus");
    }
    _status = PaymentStatus::Cancelled;
    _notes = reason;
    touch();
}

void PaymentHistory::setNotes(const std::string& notes)
{
    _notes = notes;
    touch();
}

bool PaymentHistory::isValid() const
{
    return validate().empty();
}

std::string PaymentHistory::validate() const
{
    if (_paymentId <= 0)
    {
        return "Invalid paymentId";
    }
    if (_accountId <= 0)
    {
        return "Invalid accountId";
    }
    if (_paymentAmount < sdrs::money::Money(0))
    {
        return "Payment amount must be positive";
    }
    return "";
}

std::string PaymentHistory::toJson() const
{
    std::ostringstream oss;
    oss << "{";
    oss << "\"payment_id\":" << _paymentId << ",";
    oss << "\"account_id\":" << _accountId << ",";
    oss << "\"payment_amount\":" << _paymentAmount.getAmount() << ",";
    oss << "\"payment_method\":\"" << sdrs::constants::paymentMethodToString(_method) << "\",";
    oss << "\"payment_status\":\"" << paymentStatusToString(_status) << "\",";
    oss << "\"payment_date\":\"" << std::format("{:%Y-%m-%d}", _paymentDate) << "\",";
    if (_dueDate.has_value()) {
        oss << "\"due_date\":\"" << std::format("{:%Y-%m-%d}", _dueDate.value()) << "\",";
    }
    oss << "\"is_late\":" << (isLate() ? "true" : "false") << ",";
    if (!_notes.empty()) {
        oss << "\"notes\":\"" << _notes << "\",";
    }
    oss << "\"created_at\":\"" << std::format("{:%Y-%m-%d %H:%M:%S}", _createdAt) << "\",";
    oss << "\"updated_at\":\"" << std::format("{:%Y-%m-%d %H:%M:%S}", _updatedAt) << "\"";
    oss << "}";
    return oss.str();
}

PaymentHistory PaymentHistory::fromJson(const std::string& json)
{
    auto j = nlohmann::json::parse(json);
    
    int paymentId = j.value("payment_id", j.value("paymentId", 0));
    int accountId = j.value("account_id", j.value("accountId", 0));
    double amount = j["payment_amount"].get<double>();
    
    PaymentMethod method = PaymentMethod::Cash;
    if (j.contains("payment_method") && !j["payment_method"].is_null())
    {
        std::string methodStr = j["payment_method"].get<std::string>();
        method = sdrs::constants::stringToPaymentMethod(methodStr);
    }
    
    std::chrono::sys_days paymentDate = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    if (j.contains("payment_date") && !j["payment_date"].is_null())
    {
        std::string dateStr = j["payment_date"].get<std::string>();
        std::istringstream iss(dateStr);
        std::chrono::sys_days parsed;
        std::chrono::from_stream(iss, "%Y-%m-%d", parsed);
        paymentDate = parsed;
    }
    
    std::optional<std::chrono::sys_days> dueDate;
    if (j.contains("due_date") && !j["due_date"].is_null())
    {
        std::string dateStr = j["due_date"].get<std::string>();
        std::istringstream iss(dateStr);
        std::chrono::sys_days parsed;
        std::chrono::from_stream(iss, "%Y-%m-%d", parsed);
        dueDate = parsed;
    }
    
    PaymentHistory payment(paymentId, accountId, sdrs::money::Money(amount), method, paymentDate, dueDate);
    
    if (j.contains("payment_status") && !j["payment_status"].is_null())
    {
        std::string statusStr = j["payment_status"].get<std::string>();
        payment._status = stringToPaymentStatus(statusStr);
    }
    
    if (j.contains("notes") && !j["notes"].is_null())
    {
        payment._notes = j["notes"].get<std::string>();
    }
    
    return payment;
}

std::string PaymentHistory::paymentStatusToString(PaymentStatus status)
{
    switch (status)
    {
        case PaymentStatus::Pending:
            return "Pending";
        case PaymentStatus::Completed:
            return "Completed";
        case PaymentStatus::Failed:
            return "Failed";
        case PaymentStatus::Cancelled:
            return "Cancelled";
        default:
            return "Unknown";
    }
}

std::string PaymentHistory::paymentMethodToString(PaymentMethod method)
{
    switch (method)
    {
        case PaymentMethod::BankTransfer:
            return "BankTransfer";
        case PaymentMethod::Cash:
            return "Cash";
        case PaymentMethod::Card:
            return "Card";
        case PaymentMethod::Other:
            return "Other";
        default:
            return "Unknown";
    }
}

PaymentStatus PaymentHistory::stringToPaymentStatus(const std::string& value)
{
    if (value == "Pending") 
    {
        return PaymentStatus::Pending;
    }
    if (value == "Completed") 
    {
        return PaymentStatus::Completed;
    }
    if (value == "Failed") 
    {
        return PaymentStatus::Failed;
    }
    if (value == "Cancelled") 
    {
        return PaymentStatus::Cancelled;
    }
    throw ValidationException("Unknown payment status", "paymentStatus");
}

PaymentMethod PaymentHistory::stringToPaymentMethod(const std::string& value)
{
    if (value == "BankTransfer") 
    {
        return PaymentMethod::BankTransfer;
    }
    if (value == "Cash") 
    {
        return PaymentMethod::Cash;
    }
    if (value == "Card")
    {
        return PaymentMethod::Card;
    }
    if (value == "Other")
    {
        return PaymentMethod::Other;
    }
    throw ValidationException("Unknown payment method", "paymentMethod");
}

} // namespace sdrs::borrower
