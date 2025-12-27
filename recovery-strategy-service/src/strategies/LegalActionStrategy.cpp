// LegalActionStrategy.cpp - Implementation

#include "../../include/strategies/LegalActionStrategy.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <random>
#include <format>

using namespace sdrs::constants;
using namespace sdrs::exceptions;
using namespace sdrs::money;
using namespace sdrs::communication;

namespace sdrs::strategy
{
    
LegalActionStrategy::LegalActionStrategy(
    int accountId,
    int borrowerId,
    const Money& expectedAmount,
    std::shared_ptr<IPaymentChecker> paymentChecker,
    std::shared_ptr<ICommunicationService> channel,
    const std::string& lawFirm,
    int maxLegalAttempt)
    :  RecoveryStrategy(accountId, borrowerId, expectedAmount),
    _paymentChecker(paymentChecker),
    _channel(channel),
    _lawFirm(lawFirm),
    _legalStage(LegalStage::NotStarted),
    _maxLegalAttempt(maxLegalAttempt)
{
    if (lawFirm.empty())
    {
        throw ValidationException("Law firm name cannot be empty", "LegalActionStrategy");
    }

    if (!_paymentChecker)
    {
        throw ValidationException("Payment checker cannot be null", "LegalActionStrategy");
    }

    if (maxLegalAttempt < 0)
    {
        throw ValidationException("Legal Attempt cannot be negative", "LegalActionStrategy");
    }
}

StrategyType LegalActionStrategy::getType() const
{
    return StrategyType::LegalAction;
}

std::string LegalActionStrategy::generateCaseNumber() const
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now = *std::localtime(&time_t_now);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    return "CASE-" + std::to_string(1900 + tm_now.tm_year) + "-" + std::to_string(_accountId) + "-" + std::to_string(dis(gen));
}

void LegalActionStrategy::sendLegalNotice()
{
    _attemptCount++;
    _noticeDate = std::chrono::floor<std::chrono::days>(
        std::chrono::system_clock::now()
    );
    _legalStage = LegalStage::NoticesSent;
    std::string message = std::format(
        "LEGAL NOTICE\n Amount owed:{}\n Law firm: {}\n Notice: Pay within {} days or legal action will be taken!\n",
        _expectedAmount.format(),
        _lawFirm,
        recovery::LEGAL_NOTICE_DEADLINE_DAYS
    );
    _channel->sendMessage(_accountId, message);
}

void LegalActionStrategy::fileLawsuit()
{
    _caseNumber = generateCaseNumber();
    _filingDate = std::chrono::floor<std::chrono::days>(
        std::chrono::system_clock::now()
    );
    _legalStage = LegalStage::CaseFiled;
}

bool LegalActionStrategy::checkPaymentReceived() const
{
    if (!_paymentChecker)
    {
        return false;
    }
    return _paymentChecker->hasPaymentReceived(_accountId);
}

StrategyStatus LegalActionStrategy::execute()
{
    if (!canExecute())
    {
        return _status;
    }

    _status = StrategyStatus::Active;

    while (_attemptCount < _maxLegalAttempt)
    {
        if (checkPaymentReceived())
        {
            _status = StrategyStatus::Completed;
            return _status;
        }
        sendLegalNotice();
        fileLawsuit();
        _attemptCount++;
    }

    if (checkPaymentReceived())
    {
        _status = StrategyStatus::Completed;
    }
    else
    {
        _status = StrategyStatus::Failed;
    }

    return _status;
}

LegalStage LegalActionStrategy::getLegalStage() const
{
    return _legalStage;
}

[[nodiscard]]
std::string LegalActionStrategy::legalStageToString() const
{
    switch (_legalStage)
    {
    case LegalStage::CaseFiled:
        return "CaseFiled";
    case LegalStage::Enforcing:
        return "Enforcing";
    case LegalStage::InCourt:
        return "InCourt";
    case LegalStage::JudgmentObtained:
        return "JudgmentObtained";
    case LegalStage::NoticesSent:
        return "NoticesSent";
    case LegalStage::NotStarted:
        return "NotStarted";
    }

    throw ValidationException("Invalid legal stage", "LegalActionStrategy");
}

std::string LegalActionStrategy::toJson() const
{
    return "{\"type\":\"LegalAction\",\"accountId\":" 
           + std::to_string(_accountId) 
           + ",\"lawFirm\":\"" + _lawFirm + "\""
           + ",\"caseNumber\":\"" + _caseNumber + "\""
           + ",\"legalStage\":\"" + legalStageToString() + "\""
           + "}";
}

std::string LegalActionStrategy::getCaseNumber() const
{
    return _caseNumber;
}

} // namespace sdrs::strategy
