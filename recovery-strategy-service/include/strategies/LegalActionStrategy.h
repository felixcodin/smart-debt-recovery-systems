// LegalActionStrategy.h - Strategy for initiating legal proceedings

#ifndef LEGAL_ACTION_STRATEGY_H
#define LEGAL_ACTION_STRATEGY_H

#include "RecoveryStrategy.h"
#include "../interfaces/IPaymentChecker.h"
#include "../../../communication-service/include/interfaces/ICommunicationService.h"
#include "../../../common/include/utils/Constants.h"
#include <chrono>
#include <string>

namespace sdrs::strategy
{


class LegalActionStrategy : public RecoveryStrategy
{
private:
    std::string _lawFirm;
    std::string _caseNumber;
    sdrs::constants::LegalStage _legalStage;
    std::chrono::sys_days _noticeDate;
    std::chrono::sys_days _filingDate;

    std::shared_ptr<IPaymentChecker> _paymentChecker;
    std::shared_ptr<sdrs::communication::ICommunicationService> _channel;

    int _maxLegalAttempt = 0;

public:
    LegalActionStrategy(
        int accountId,
        int borrowerId,
        const sdrs::money::Money& expectedAmount,
        std::shared_ptr<IPaymentChecker> paymentChecker,
        std::shared_ptr<sdrs::communication::ICommunicationService> channel,
        const std::string& lawFirm = "Default Law Firm",
        int maxLegalAttempt = sdrs::constants::recovery::MAX_LEGAL_ATTEMPTS
    );

public:
    StrategyStatus execute() override;  // sends legal notice, then files lawsuit if needed
    StrategyType getType() const override;
    std::string toJson() const override;

    sdrs::constants::LegalStage getLegalStage() const;
    std::string getCaseNumber() const;

private:
    void sendLegalNotice();           // send formal legal warning
    void fileLawsuit();               // file case with court, generates case number
    bool checkPaymentReceived() const;
    std::string generateCaseNumber() const;  // format: LEGAL-YYYYMMDD-XXXXX
    std::string legalStageToString() const;

};

} // namespace sdrs::strategy


#endif