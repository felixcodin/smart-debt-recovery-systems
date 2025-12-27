// StrategyManager.h - Orchestrates recovery strategies based on risk assessment

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
#include "../../../common/include/utils/Constants.h"

#include <memory>
#include <vector>
#include <map>

namespace sdrs::strategy
{
    
// Result of analyzing a borrower account
struct AnalysisResult
{
    double riskScore;           // 0.0 (low risk) to 1.0 (high risk)
    sdrs::constants::RiskLevel riskLevel;
    int clusterId;              // K-Means cluster assignment
    double distanceToCluster;   // distance to cluster centroid
    sdrs::constants::StrategyType recommendedStrategy;
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

    // ML model training
public:
    void trainModels();         // trains RandomForest + K-Means on synthetic data
    bool areModelsReady() const;

    // Full analysis pipeline: risk scoring + clustering + strategy recommendation
    AnalysisResult analyzeAccount(const sdrs::risk::RiskFeatures& features);

    // Analyze and execute strategy in one call
    StrategyStatus assignWithFullAnalysis(
        int accountId,
        int borrowerId,
        const sdrs::money::Money& amount,
        const sdrs::risk::RiskFeatures& features
    );

    // Strategy factory - creates strategy based on risk level
public:
    std::unique_ptr<RecoveryStrategy> createStrategy(
        int accountId,
        int borrowerId,
        const sdrs::money::Money& amount,
        sdrs::constants::RiskLevel riskLevel  // Low->Reminder, Medium->Settlement, High->Legal
    );

    // Create, store, and execute strategy
    StrategyStatus assignAndExecute(
        int accountId,
        int borrowerId,
        const sdrs::money::Money& amount,
        sdrs::constants::RiskLevel riskLevel
    );

    RecoveryStrategy* getActiveStrategy(int accountId);  // returns nullptr if not found

    bool hasActiveStrategy(int accountId) const;

    bool cancelStrategy(int accountId);  // removes from active strategies

private:
    std::vector<double> prepareClusterFeatures(
        const sdrs::risk::RiskFeatures& features,
        double riskScore
    ) const;

    sdrs::constants::RiskLevel combineAnalysis(
        double riskScore,
        int clusterId
    ) const;

    sdrs::constants::RiskLevel clusterToRiskLevel(int clusterId) const;

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