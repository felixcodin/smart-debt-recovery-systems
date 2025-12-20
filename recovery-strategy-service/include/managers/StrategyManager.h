#ifndef STRATEGY_MANAGER_H
#define STRATEGY_MANAGER_H

#include "../strategies/RecoveryStrategy.h"
#include "../strategies/AutomatedReminderStrategy.h"
#include "../strategies/LegalActionStrategy.h"
#include "../strategies/SettlementOfferStrategy.h"
#include "../interfaces/IPaymentChecker.h"
#include "../../../communication-service/include/interfaces/ICommunicationService.h"
#include "../../../risk-assessment-service/include/algorithms/KMeansClustering.h"
#include "../../../risk-assessment-service/include/models/RiskScorer.h"
#include "../../../common/include/models/Money.h"

#include <memory>
#include <vector>
#include <map>

namespace sdrs::strategy
{
    
struct AnalysisResult
{
    double riskScore;
    sdrs::risk::RiskLevel riskLevel;
    int clusterId;
    double distanceToCluster;
    StrategyType recommendedStrategy;
};

class StrategyManager
{
private:
    std::shared_ptr<IPaymentChecker> _paymentChecker;
    std::shared_ptr<sdrs::communication::ICommunicationService> _communicationService;

    std::unique_ptr<sdrs::risk::RiskScorer> _riskScorer;
    std::unique_ptr<sdrs::risk::KMeansClustering> _kmeans;
    bool _isModelsTrained;

    std::map<int, std::unique_ptr<RecoveryStrategy>> _activeStrategies;

public:
    StrategyManager(
        std::shared_ptr<IPaymentChecker> paymentChecker,
        std::shared_ptr<sdrs::communication::ICommunicationService> communicationService
    );

    ~StrategyManager() = default;

public:
    void trainModels();
    bool areModelsReady() const;

    AnalysisResult analyzeAccount(const sdrs::risk::RiskFeatures& features);

    StrategyStatus assignWithFullAnalysis(
        int accountId,
        int borrowerId,
        const sdrs::money::Money& amount,
        const sdrs::risk::RiskFeatures& features
    );

public:
    std::unique_ptr<RecoveryStrategy> createStrategy(
        int accountId,
        int borrowerId,
        const sdrs::money::Money& amount,
        sdrs::risk::RiskLevel riskLevel
    );

    StrategyStatus assignAndExecute(
        int accountId,
        int borrowerId,
        const sdrs::money::Money& amount,
        sdrs::risk::RiskLevel riskLevel
    );

    RecoveryStrategy* getActiveStrategy(int accountId);

    bool hasActiveStrategy(int accountId) const;

    bool cancelStrategy(int accountId);

private:
    std::vector<double> prepareClusterFeatures(
        const sdrs::risk::RiskFeatures& features,
        double riskScore
    ) const;

    sdrs::risk::RiskLevel combineAnalysis(
        double riskScore,
        int clusterId
    ) const;

    sdrs::risk::RiskLevel clusterToRiskLevel(int clusterId) const;

    std::unique_ptr<RecoveryStrategy> createRemindersStrategy(
        int accountId, int borrowerId, const sdrs::money::Money& amount
    );

    std::unique_ptr<RecoveryStrategy> createSettlementStrategy(
        int accountId, int borrowerId, const sdrs::money::Money& amount
    );

    std::unique_ptr<RecoveryStrategy> createLegalStrategy(
        int accountId, int borrowerId, const sdrs::money::Money& amount
    );

};

} // namespace sdrs::strategy


#endif