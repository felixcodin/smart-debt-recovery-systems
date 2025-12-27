// StrategyManager.cpp - Implementation

#include "../../include/managers/StrategyManager.h"
#include "../../../common/include/exceptions/ValidationException.h"

using namespace sdrs::exceptions;
using namespace sdrs::money;
using namespace sdrs::communication;
using namespace sdrs::constants;
using namespace sdrs::risk;

namespace sdrs::strategy
{

StrategyManager::StrategyManager(
    std::shared_ptr<IPaymentChecker> paymentChecker,
    std::shared_ptr<ICommunicationService> communicationService
) : _paymentChecker(paymentChecker),
    _communicationService(communicationService),
    _riskScorer(std::make_unique<RiskScorer>()),
    _kmeans(std::make_unique<KMeansClustering>(constants::risk::KMEANS_NUM_CLUSTERS)),
    _isModelsTrained(false)
{
    if (!paymentChecker)
    {
        throw ValidationException("Payment Checker cannot be null", "StrategyManager");
    }

    if (!communicationService)
    {
        throw ValidationException("Communication Service cannot be null", "StrategyManager");
    }
}
    
void StrategyManager::trainModels()
{
    _riskScorer->trainModel();

    std::vector<std::vector<double>> trainingData;


    trainingData.push_back({0.05, 0.0, 0.2, 0.15});
    trainingData.push_back({0.08, 0.08, 0.25, 0.20});
    trainingData.push_back({0.03, 0.0, 0.15, 0.10});

    trainingData.push_back({0.25, 0.25, 0.45, 0.50});
    trainingData.push_back({0.30, 0.33, 0.50, 0.55});
    trainingData.push_back({0.20, 0.17, 0.40, 0.45});

    trainingData.push_back({0.70, 0.67, 0.80, 0.85});
    trainingData.push_back({0.85, 0.83, 0.90, 0.90});
    trainingData.push_back({0.60, 0.58, 0.75, 0.75});

    _kmeans->train(trainingData);
    _isModelsTrained = true;
}

bool StrategyManager::areModelsReady() const
{
    return _isModelsTrained && _riskScorer->isModelReady() && _kmeans->isTrained();
}

AnalysisResult StrategyManager::analyzeAccount(const RiskFeatures& features)
{
    if (!areModelsReady())
    {
        throw ValidationException(
            "Model not trained. Call trainModels() first.",
            "StrategyManager"
        );
    }

    AnalysisResult result;

    auto assessment = _riskScorer->assessRisk(features);
    result.riskScore = assessment.getRiskScore();

    auto clusterFeatures = prepareClusterFeatures(features, result.riskScore);
    auto clusterResult = _kmeans->predict(clusterFeatures);
    result.clusterId = clusterResult.clusterId;
    result.distanceToCluster = clusterResult.distanceToCentroid;

    result.riskLevel = combineAnalysis(result.riskScore, result.clusterId);

    switch (result.riskLevel)
    {
    case RiskLevel::Low:
        result.recommendedStrategy = StrategyType::AutomatedReminder;
        break;
    case RiskLevel::Medium:
        result.recommendedStrategy = StrategyType::SettlementOffer;
        break;
    case RiskLevel::High:
        result.recommendedStrategy = StrategyType::LegalAction;
        break;
    default:
        throw ValidationException("Unknown risk level", "StrategyManager");
    }

    return result;
}

std::vector<double> StrategyManager::prepareClusterFeatures(
    const RiskFeatures& features,
    double riskScore) const
{
    return {
        static_cast<double>(features.daysPastDue) / static_cast<double>(constants::time::DAYS_PER_YEAR),
        static_cast<double>(features.numberOfMissedPayments) / 12.0, // Max 12 missed payments per year
        features.debtToIncomeRatio,
        riskScore
    };
}

RiskLevel StrategyManager::clusterToRiskLevel(int clusterId) const
{
    switch (clusterId)
    {
    case 0:
        return RiskLevel::Low;
    case 1:
        return RiskLevel::Medium;
    case 2:
        return RiskLevel::High;
    }

    throw ValidationException("Unknown cluster ID to convert to Risk Level", "StrategyManager");
}

RiskLevel StrategyManager::combineAnalysis(
    double riskScore,
    int clusterId) const
{
    RiskLevel scoreLevel;
    if (riskScore < constants::risk::LOW_RISK_MAX)
    {
        scoreLevel = RiskLevel::Low;
    }
    else if (riskScore < constants::risk::MEDIUM_RISK_MAX)
    {
        scoreLevel = RiskLevel::Medium;
    }
    else
    {
        scoreLevel = RiskLevel::High;
    }

    RiskLevel clusterLevel = clusterToRiskLevel(clusterId);

    if ((scoreLevel == RiskLevel::High)
    || (clusterLevel == RiskLevel::High))
    {
        return RiskLevel::High;
    }
    if ((scoreLevel == RiskLevel::Medium)
    || (clusterLevel == RiskLevel::Medium))
    {
        return RiskLevel::Medium;
    }

    return RiskLevel::Low;
}

std::unique_ptr<RecoveryStrategy> StrategyManager::createStrategy(
    int accountId,
    int borrowerId,
    const sdrs::money::Money& amount,
    RiskLevel riskLevel)
{
    switch (riskLevel)
    {
    case RiskLevel::Low:
        return createRemindersStrategy(accountId, borrowerId, amount);
    case RiskLevel::Medium:
        return createSettlementStrategy(accountId, borrowerId, amount);
    case RiskLevel::High:
        return createLegalStrategy(accountId, borrowerId, amount);
    }

    throw ValidationException("Invalid risk level", "StrategyManager");
}

StrategyStatus StrategyManager::assignAndExecute(
    int accountId,
    int borrowerId,
    const sdrs::money::Money& amount,
    RiskLevel riskLevel)
{
    if (hasActiveStrategy(accountId))
    {
        throw ValidationException(
            "Account already has an active strategy",
            "StrategyManager"
        );
    }

    auto strategy = createStrategy(accountId, borrowerId, amount, riskLevel);
    
    StrategyStatus result = strategy->execute();

    if ((result == StrategyStatus::Active)
    || (result == StrategyStatus::Pending))
    {
        _activeStrategies[accountId] = std::move(strategy);
    }

    return result;
}

StrategyStatus StrategyManager::assignWithFullAnalysis(
    int accountId,
    int borrowerId,
    const sdrs::money::Money& amount,
    const RiskFeatures& features)
{
    if (hasActiveStrategy(accountId))
    {
        throw ValidationException("Account already has an active strategy", "StrategyManager"); 
    }

    AnalysisResult analysis = analyzeAccount(features);

    auto strategy = createStrategy(accountId, borrowerId, amount, analysis.riskLevel);
    StrategyStatus result = strategy->execute();

    if ((result == StrategyStatus::Active)
    || (result == StrategyStatus::Pending))
    {
        _activeStrategies[accountId] = std::move(strategy);
    }

    return result;

}

RecoveryStrategy* StrategyManager::getActiveStrategy(int accountId)
{
    auto it = _activeStrategies.find(accountId);
    if (it != _activeStrategies.end())
    {
        return it->second.get();
    }
    return nullptr;
}

bool StrategyManager::hasActiveStrategy(int accountId) const
{
    return _activeStrategies.find(accountId) != _activeStrategies.end();
} 

bool StrategyManager::cancelStrategy(int accountId)
{
    auto it = _activeStrategies.find(accountId);
    if (it != _activeStrategies.end())
    {
        _activeStrategies.erase(it);
        return true;
    }
    return false;
}

std::unique_ptr<RecoveryStrategy> StrategyManager::createRemindersStrategy(
    int accountId,
    int borrowerId,
    const sdrs::money::Money& amount)
{
    return std::make_unique<AutomatedReminderStrategy>(
        accountId,
        borrowerId,
        amount,
        _paymentChecker,
        _communicationService
    );
}

std::unique_ptr<RecoveryStrategy> StrategyManager::createSettlementStrategy(
    int accountId,
    int borrowerId,
    const sdrs::money::Money& amount)
{
    return std::make_unique<SettlementOfferStrategy>(
        accountId,
        borrowerId,
        amount,
        _paymentChecker,
        _communicationService
    );
}

std::unique_ptr<RecoveryStrategy> StrategyManager::createLegalStrategy(
    int accountId,
    int borrowerId,
    const sdrs::money::Money& amount)
{
    return std::make_unique<LegalActionStrategy>(
        accountId,
        borrowerId,
        amount,
        _paymentChecker,
        _communicationService
    );
}

} // namespace sdrs::strategy
