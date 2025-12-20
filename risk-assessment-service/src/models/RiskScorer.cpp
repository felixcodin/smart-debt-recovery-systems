#include "../../include/models/RiskScorer.h"
#include "../../include/algorithms/RandomForest.h"
#include "../../../common/include/exceptions/ValidationException.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace sdrs::risk
{

RiskAssessment::RiskAssessment(int accountId,
    int borrowerId,
    double score,
    AlgorithmUsed algorithm)
    : _assessmentId(0),
    _accountId(accountId),
    _borrowerId(borrowerId),
    _riskScore(score),
    _algorithmUsed(algorithm)
{
    if (score < 0.0 || score > 1.0)
    {
        throw sdrs::exceptions::ValidationException(
            "Risk score must be between 0 and 1", "riskScore"
        );
    }
    
    _riskLevel = determineRiskLevel(score);
    _assessmentDate = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
    _createdAt = _assessmentDate;
}

RiskLevel RiskAssessment::determineRiskLevel(double score) const
{
    if (score < 0.33)
    {
        return RiskLevel::Low;
    }
    if (score < 0.67)
    {
        return RiskLevel::Medium;   
    }
    return RiskLevel::High;
}

void RiskAssessment::addRiskFactor(const std::string& factorName, double contribution)
{
    _riskFactors[factorName] = contribution;
    touch();
}

void RiskAssessment::touch()
{
    _assessmentDate = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}

int RiskAssessment::getAssessmentId() const { return _assessmentId; }
int RiskAssessment::getAccountId() const { return _accountId; }
int RiskAssessment::getBorrowerId() const { return _borrowerId; }
RiskLevel RiskAssessment::getRiskLevel() const { return _riskLevel; }
AlgorithmUsed RiskAssessment::getAlgorithmUsed() const { return _algorithmUsed; }
std::chrono::sys_seconds RiskAssessment::getAssessmentDate() const { return _assessmentDate; }
std::chrono::sys_seconds RiskAssessment::getCreatedAt() const { return _createdAt; }
const std::map<std::string, double>& RiskAssessment::getRiskFactors() const { return _riskFactors; }
double RiskAssessment::getRiskScore() const { return _riskScore; }

std::string RiskAssessment::toJson() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "{\"assessment_id\":" << _assessmentId
        << ",\"account_id\":" << _accountId
        << ",\"borrower_id\":" << _borrowerId
        << ",\"risk_score\":" << _riskScore
        << ",\"risk_level\":\"" << riskLevelToString(_riskLevel) << "\""
        << ",\"algorithm\":\"" << (_algorithmUsed == AlgorithmUsed::RandomForest ? "RandomForest" : "RuleBased") << "\""
        << ",\"risk_factors\":{";
    bool first = true;
    for (const auto& [key, value] : _riskFactors)
    {
        if (!first) oss << ",";
        oss << "\"" << key << "\":" << value;
        first = false;
    }
    oss << "}}";
    return oss.str();
}

std::string RiskAssessment::riskLevelToString(RiskLevel level)
{
    switch (level)
    {
        case RiskLevel::Low:
            return "Low";
        case RiskLevel::Medium:
            return "Medium";
        case RiskLevel::High:
            return "High";
        default:
            return "Unknown";
    }
}

RiskLevel RiskAssessment::stringToRiskLevel(const std::string& levelStr)
{
    if (levelStr == "Low")
    {
        return RiskLevel::Low;
    }
    if (levelStr == "Medium")
    {
        return RiskLevel::Medium;
    }
    if (levelStr == "High")
    {
        return RiskLevel::High;
    }
    return RiskLevel::Medium;
}

RiskFeatures::RiskFeatures()
{
    // Do nothing
}

void RiskFeatures::validate() const
{
    if (accountId <= 0)
    {
        throw sdrs::exceptions::ValidationException("Invalid account ID", "accountId");
    }
    if (borrowerId <= 0)
    {
        throw sdrs::exceptions::ValidationException("Invalid borrower ID", "borrowerId");
    }
    if ((loanAmount < 0)
    || (remainingAmount < 0))
    {
        throw sdrs::exceptions::ValidationException("Negative amounts", "loanAmount");
    }
    if ((interestRate < 0)
    || (interestRate > 1))
    {
        throw sdrs::exceptions::ValidationException("Interest rate out of range", "interestRate");
    }
}

RiskScorer::RiskScorer() : _randomForest(nullptr), _useMLModel(false), _isModelTrained(false)
{
    // Do nothing
}
RiskScorer::~RiskScorer() = default;

RiskAssessment RiskScorer::assessRisk(const RiskFeatures& features)
{
    features.validate();
    double riskScore;
    AlgorithmUsed algorithm;
    
    if (_useMLModel && _isModelTrained && _randomForest)
    {
        auto rawFeatures = extractFeatures(features);
        auto normalized = normalizeFeatures(rawFeatures);
        riskScore = predictWithML(normalized);
        algorithm = AlgorithmUsed::RandomForest;
    }
    else
    {
        riskScore = calculateRuleBasedScore(features);
        algorithm = AlgorithmUsed::RuleBase;
    }
    
    riskScore = std::max(0.0, std::min(1.0, riskScore));
    RiskAssessment assessment(features.accountId, features.borrowerId, riskScore, algorithm);
    auto contributions = calculateFeatureContributions(features, riskScore);
    for (const auto& [feature, contrib] : contributions)
    {
        assessment.addRiskFactor(feature, contrib);
    }
    return assessment;
}

std::vector<double> RiskScorer::extractFeatures(const RiskFeatures& input) const
{
    constexpr size_t EXPECTED_FEATURE = 8;

    std::vector<double> features = {
        static_cast<double>(input.daysPastDue),
        static_cast<double>(input.numberOfMissedPayments),
        input.remainingAmount.getAmount() / (input.loanAmount.getAmount() + 1e-6),
        input.interestRate,
        input.monthlyIncome.getAmount(),
        static_cast<double>(input.accountAgeMonths),
        encodeEmploymentStatus(input.employmentStatus),
        encodeAccountStatus(input.accountStatus)
    };

    if (features.size() != EXPECTED_FEATURE)
    {
        throw sdrs::exceptions::ValidationException("Feature size mismatch");
    }

    return features;
}

std::vector<double> RiskScorer::normalizeFeatures(const std::vector<double>& raw) const
{
    std::vector<double> normalized;
    std::vector<double> maxVals = {
        MAX_DAYS_PAST_DUE, MAX_MISSED_PAYMENTS, MAX_REMAINING_BALANCE_RATIO,
        MAX_INTEREST_RATE, MAX_MONTHLY_INCOME, MAX_LOAN_TERM, MAX_EMPLOYMENT_RISK_SCORE, MAX_ACCOUNT_STATUS_RISK_SCORE
    };
    for (size_t i = 0; i < raw.size(); ++i)
    {
        normalized.push_back(std::min(1.0, raw[i] / maxVals[i]));
    }
    return normalized;
}

double RiskScorer::predictWithML(const std::vector<double>& features) const
{
    return _randomForest->predict(features);
}

double RiskScorer::calculateRuleBasedScore(const RiskFeatures& features) const
{
    double score = 0.0;

    if (features.daysPastDue >= 90)
    {
        score += 0.40;
    }
    else if (features.daysPastDue >= 60)
    {
        score += 0.30;
    }
    else if (features.daysPastDue >= 30)
    {
        score += 0.20;
    }
    else 
    {
        score += features.daysPastDue / 90.0 * 0.20;
    }
    
    score += std::min(0.30, features.numberOfMissedPayments * 0.10);
    
    double debtRatio = features.remainingAmount.getAmount() / (features.loanAmount.getAmount() + 1e-6);
    score += debtRatio * 0.20;
    
    score += getEmploymentRiskContribution(features.employmentStatus);
    score += getAccountRiskContribution(features.accountStatus);
    
    return std::min(1.0, score);
}

void RiskScorer::trainModel()
{
    if (!_randomForest)
    {
        _randomForest = std::make_shared<RandomForest>(10, 7, 5);
    }
    
    std::vector<std::vector<double>> X;
    std::vector<double> y;
    generateSyntheticData(X, y, 100);
    
    std::vector<std::vector<double>> X_normalized;
    X_normalized.reserve(X.size());
    for (const auto& sample : X) {
        X_normalized.push_back(normalizeFeatures(sample));
    }
    
    _randomForest->train(X_normalized, y);
    _isModelTrained = true;
    _useMLModel = true;
}

void RiskScorer::generateSyntheticData(
    std::vector<std::vector<double>>& X,
    std::vector<double>& y,
    int numSamples) const
{
    X.clear();
    y.clear();
    X.reserve(numSamples);
    y.reserve(numSamples);
    
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    for (int i = 0; i < numSamples; ++i)
    {
        std::vector<double> features(8);
        double riskScore;

        double scenario = uniform(rng);
        
        if (scenario < 0.30)
        {
            features[0] = uniform(rng) * 15.0;
            
            features[1] = (uniform(rng) < 0.8) ? 0.0 : 1.0;
           
            features[2] = 0.3 + uniform(rng) * 0.4;
            
            features[3] = 0.05 + uniform(rng) * 0.07;
            
            features[4] = 4000.0 + uniform(rng) * 4000.0;
            
            features[5] = 12.0 + uniform(rng) * 36.0;
            
            features[6] = (uniform(rng) < 0.9) ? 1.0 : 0.5;
            
            features[7] = 1.0;
            
            riskScore = 0.05 + uniform(rng) * 0.25;
        }
        else if (scenario < 0.70)
        {
            features[0] = 15.0 + uniform(rng) * 45.0;

            features[1] = 1.0 + std::floor(uniform(rng) * 3.0);

            features[2] = 0.6 + uniform(rng) * 0.3;

            features[3] = 0.12 + uniform(rng) * 0.08;

            features[4] = 2500.0 + uniform(rng) * 2000.0;

            features[5] = 6.0 + uniform(rng) * 18.0;

            double emp = uniform(rng);
            features[6] = (emp < 0.5) ? 1.0 : 0.5;

            features[7] = (uniform(rng) < 0.7) ? 1.0 : 0.8;

            riskScore = 0.30 + uniform(rng) * 0.35;
        }
        else
        {
            features[0] = 60.0 + uniform(rng) * 120.0;

            features[1] = 3.0 + std::floor(uniform(rng) * 6.0);

            features[2] = 0.85 + uniform(rng) * 0.15;

            features[3] = 0.18 + uniform(rng) * 0.12;

            features[4] = 1000.0 + uniform(rng) * 2000.0;

            features[5] = 3.0 + uniform(rng) * 15.0;

            double emp = uniform(rng);
            features[6] = (emp < 0.6) ? 0.0 : 0.5;

            double status = uniform(rng);
            if (status < 0.4) features[7] = 0.0;
            else if (status < 0.7) features[7] = 0.4;
            else features[7] = 0.6;

            riskScore = 0.65 + uniform(rng) * 0.30;
        }
        
        X.push_back(features);
        y.push_back(riskScore);
    }
}

void RiskScorer::loadModel(const std::string& modelPath) { (void)modelPath; }
bool RiskScorer::isModelReady() const { return _useMLModel && _isModelTrained && _randomForest && _randomForest->isTrained(); }
void RiskScorer::setUseMLModel(bool useML) { _useMLModel = useML; }

double RiskScorer::encodeEmploymentStatus(sdrs::borrower::EmploymentStatus status)
{
    switch (status)
    {
    case sdrs::borrower::EmploymentStatus::Employed:
    case sdrs::borrower::EmploymentStatus::Contract:
    case sdrs::borrower::EmploymentStatus::SelfEmployed:
        return 1.0;
    case sdrs::borrower::EmploymentStatus::PartTime:
    case sdrs::borrower::EmploymentStatus::Student:
        return 0.5;
    case sdrs::borrower::EmploymentStatus::Unemployed:
    case sdrs::borrower::EmploymentStatus::Retired:
    case sdrs::borrower::EmploymentStatus::None:
        return 0.0;
    default:
        return 0.0;
    }
}

double RiskScorer::getEmploymentRiskContribution(sdrs::borrower::EmploymentStatus status)
{
    switch (status)
    {
    case sdrs::borrower::EmploymentStatus::Employed:
    case sdrs::borrower::EmploymentStatus::Contract:
    case sdrs::borrower::EmploymentStatus::SelfEmployed:
        return 0.0;
    case sdrs::borrower::EmploymentStatus::PartTime:
    case sdrs::borrower::EmploymentStatus::Student:
        return 0.05;
    case sdrs::borrower::EmploymentStatus::Unemployed:
    case sdrs::borrower::EmploymentStatus::Retired:
    case sdrs::borrower::EmploymentStatus::None:
        return 0.10;
    default:
        return 0.10;
    }
}

double RiskScorer::encodeAccountStatus(sdrs::borrower::AccountStatus status)
{
    switch (status)
    {
    case sdrs::borrower::AccountStatus::PaidOff:
    case sdrs::borrower::AccountStatus::Current:
        return 1.0;
    case sdrs::borrower::AccountStatus::Partial:
        return 0.8;
    case sdrs::borrower::AccountStatus::Settled:
        return 0.6;
    case sdrs::borrower::AccountStatus::Delinquent:
        return 0.4;
    case sdrs::borrower::AccountStatus::Default:
    case sdrs::borrower::AccountStatus::ChargedOff:
        return 0.0;
    default:
        return 0.5;
    }
}

double RiskScorer::getAccountRiskContribution(sdrs::borrower::AccountStatus status)
{
    switch (status)
    {
    case sdrs::borrower::AccountStatus::PaidOff:
    case sdrs::borrower::AccountStatus::Current:
        return 0.0;
    case sdrs::borrower::AccountStatus::Partial:
        return 0.02;
    case sdrs::borrower::AccountStatus::Settled:
        return 0.04;
    case sdrs::borrower::AccountStatus::Delinquent:
        return 0.06;
    case sdrs::borrower::AccountStatus::Default:
    case sdrs::borrower::AccountStatus::ChargedOff:
        return 0.10;
    default:
        return 0.10;
    }
}

std::map<std::string, double> RiskScorer::calculateFeatureContributions(const RiskFeatures& features, double finalScore) const
{
    std::map<std::string, double> contributions;
    
    contributions["days_past_due"] = (features.daysPastDue / MAX_DAYS_PAST_DUE) * 0.4;
    contributions["missed_payments"] = (features.numberOfMissedPayments / MAX_MISSED_PAYMENTS) * 0.3;
    contributions["debt_ratio"] = (features.remainingAmount.getAmount() / (features.loanAmount.getAmount() + 1e-6)) * 0.2;
    contributions["employment"] = getEmploymentRiskContribution(features.employmentStatus);
    contributions["account_status"] = getAccountRiskContribution(features.accountStatus);

    return contributions;
}

}