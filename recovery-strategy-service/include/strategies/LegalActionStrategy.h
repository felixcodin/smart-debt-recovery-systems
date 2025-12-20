#ifndef LEGAL_ACTION_STRATEGY_H
#define LEGAL_ACTION_STRATEGY_H

#include "RecoveryStrategy.h"
#include "../interfaces/IPaymentChecker.h"
#include "../../../communication-service/include/interfaces/ICommunicationService.h"
#include <chrono>
#include <string>

namespace sdrs::strategy
{
 
enum class LegalStage
{
    NotStarted,
    NoticesSent, // legal notice sent
    CaseFiled, // Lawsuit field
    InCourt, // Proceeding in court
    JudgmentObtained, // Judgment issued
    Enforcing // Enforcement underway
};

class LegalActionStrategy : public RecoveryStrategy
{
private:
    std::string _lawFirm;
    std::string _caseNumber;
    LegalStage _legalStage;
    std::chrono::sys_days _noticeDate;
    std::chrono::sys_days _filingDate;

    std::shared_ptr<IPaymentChecker> _paymentChecker;
    std::shared_ptr<sdrs::communication::ICommunicationService> _channel;

public:
    LegalActionStrategy(
        int accountId,
        int borrowerId,
        const sdrs::money::Money& expectedAmount,
        std::shared_ptr<IPaymentChecker> paymentChecker,
        std::shared_ptr<sdrs::communication::ICommunicationService> channel,
        const std::string& lawFirm = "Default Law Firm"
    );

public:
    StrategyStatus execute() override;
    StrategyType getType() const override;
    std::string toJson() const override;

    LegalStage getLegalStage() const;
    std::string getCaseNumber() const;

private:
    void sendLegalNotice();
    void fileLawsuit();
    bool checkPaymentReceived() const;
    std::string generateCaseNumber() const;
    std::string legalStageToString() const;

};

} // namespace sdrs::strategy


#endif