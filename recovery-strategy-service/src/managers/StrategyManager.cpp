#include "../../include/managers/StrategyManager.h"
#include "../../../common/include/exceptions/ValidationException.h"

namespace sdrs::strategy
{

StrategyManager::StrategyManager(
    std::shared_ptr<IPaymentChecker> paymentChecker,
    std::shared_ptr<sdrs::communication::ICommunicationService> communicationService
) : _paymentChecker(paymentChecker),
    _communicationService(communicationService),
    _riskScorer(std::make_unique<sdrs::risk::RiskScorer>()),
    _kmeans(std::make_unique<sdrs::risk::KMeansClustering>(3)),
    _isModelsTrained(false)
{
    if (!paymentChecker)
    {
        throw sdrs::exceptions::ValidationException("Payment Checker cannot be null", "StrategyManager");
    }

    if (!communicationService)
    {
        throw sdrs::exceptions::ValidationException("Communication Service cannot be null", "StrategyManager");
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

AnalysisResult StrategyManager::analyzeAccount(const sdrs::risk::RiskFeatures& features)
{
    if (!areModelsReady())
    {
        throw sdrs::exceptions::ValidationException(
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
    case sdrs::risk::RiskLevel::Low:
        result.recommendedStrategy = StrategyType::AutomatedReminder;
        break;
    case sdrs::risk::RiskLevel::Medium:
        result.recommendedStrategy = StrategyType::SettlementOffer;
        break;
    case sdrs::risk::RiskLevel::High:
        result.recommendedStrategy = StrategyType::LegalAction;
        break;
    default:
        throw sdrs::exceptions::ValidationException("Unknown risk level", "StrategyManager");
    }

    return result;
}

std::vector<double> StrategyManager::prepareClusterFeatures(
    const sdrs::risk::RiskFeatures& features,
    double riskScore) const
{
    return {
        static_cast<double>(features.daysPastDue) / 365.0,
        static_cast<double>(features.numberOfMissedPayments) / 12.0,
        features.debtToIncomeRatio,
        riskScore
    };
}

sdrs::risk::RiskLevel StrategyManager::clusterToRiskLevel(int clusterId) const
{
    switch (clusterId)
    {
    case 0:
        return sdrs::risk::RiskLevel::Low;
    case 1:
        return sdrs::risk::RiskLevel::Medium;
    case 2:
        return sdrs::risk::RiskLevel::High;
    }

    throw sdrs::exceptions::ValidationException("Unknown cluster ID to convert to Risk Level", "StrategyManager");
}

sdrs::risk::RiskLevel StrategyManager::combineAnalysis(
    double riskScore,
    int clusterId) const
{
    sdrs::risk::RiskLevel scoreLevel;
    if (riskScore < 0.33)
    {
        scoreLevel = sdrs::risk::RiskLevel::Low;
    }
    else if (riskScore < 0.67)
    {
        scoreLevel = sdrs::risk::RiskLevel::Medium;
    }
    else
    {
        scoreLevel = sdrs::risk::RiskLevel::High;
    }

    sdrs::risk::RiskLevel clusterLevel = clusterToRiskLevel(clusterId);

    if ((scoreLevel == sdrs::risk::RiskLevel::High)
    || (clusterLevel == sdrs::risk::RiskLevel::High))
    {
        return sdrs::risk::RiskLevel::High;
    }
    if ((scoreLevel == sdrs::risk::RiskLevel::Medium)
    || (clusterLevel == sdrs::risk::RiskLevel::Medium))
    {
        return sdrs::risk::RiskLevel::Medium;
    }

    return sdrs::risk::RiskLevel::Low;
}

std::unique_ptr<RecoveryStrategy> StrategyManager::createStrategy(
    int accountId,
    int borrowerId,
    const sdrs::money::Money& amount,
    sdrs::risk::RiskLevel riskLevel)
{
    switch (riskLevel)
    {
    case sdrs::risk::RiskLevel::Low:
        return createRemindersStrategy(accountId, borrowerId, amount);
    case sdrs::risk::RiskLevel::Medium:
        return createSettlementStrategy(accountId, borrowerId, amount);
    case sdrs::risk::RiskLevel::High:
        return createLegalStrategy(accountId, borrowerId, amount);
    }

    throw sdrs::exceptions::ValidationException("Invalid risk level", "StrategyManager");
}

StrategyStatus StrategyManager::assignAndExecute(
    int accountId,
    int borrowerId,
    const sdrs::money::Money& amount,
    sdrs::risk::RiskLevel riskLevel)
{
    if (hasActiveStrategy(accountId))
    {
        throw sdrs::exceptions::ValidationException(
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
    const sdrs::risk::RiskFeatures& features)
{
    if (hasActiveStrategy(accountId))
    {
        throw sdrs::exceptions::ValidationException("Account already has an active strategy", "StrategyManager"); 
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
