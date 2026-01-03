// RiskScorer.cpp - Implementation

#include "../../include/models/RiskScorer.h"
#include "../../include/algorithms/RandomForest.h"
#include "../../../common/include/exceptions/ValidationException.h"
#include "../../../common/include/utils/Constants.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace sdrs::constants;
using namespace sdrs::constants::risk;
using namespace sdrs::exceptions;
using namespace sdrs::borrower;

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
        throw ValidationException(
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
    if (score < constants::risk::LOW_RISK_MAX)
    {
        return RiskLevel::Low;
    }
    if (score < constants::risk::MEDIUM_RISK_MAX)
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
        throw ValidationException("Invalid account ID", "accountId");
    }
    if (borrowerId <= 0)
    {
        throw ValidationException("Invalid borrower ID", "borrowerId");
    }
    if ((loanAmount.getAmount() < 0)
    || (remainingAmount.getAmount() < 0))
    {
        throw ValidationException("Negative amounts", "loanAmount");
    }
    if ((interestRate < 0)
    || (interestRate > 1))
    {
        throw ValidationException("Interest rate out of range", "interestRate");
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
    constexpr size_t EXPECTED_FEATURE = 9;  // Updated: added Age

    std::vector<double> features = {
        static_cast<double>(input.daysPastDue),
        static_cast<double>(input.numberOfMissedPayments),
        input.remainingAmount.getAmount() / (input.loanAmount.getAmount() + 1e-6),
        input.interestRate,
        input.monthlyIncome.getAmount(),
        static_cast<double>(input.accountAgeMonths),
        static_cast<double>(input.age),  // NEW: Age feature
        encodeEmploymentStatus(input.employmentStatus),
        encodeAccountStatus(input.accountStatus)
    };

    if (features.size() != EXPECTED_FEATURE)
    {
        throw ValidationException("Feature size mismatch");
    }

    return features;
}

std::vector<double> RiskScorer::normalizeFeatures(const std::vector<double>& raw) const
{
    std::vector<double> normalized;
    std::vector<double> maxVals = {
        MAX_DAYS_PAST_DUE,         // [0] days_past_due
        MAX_MISSED_PAYMENTS,        // [1] missed_payments
        MAX_REMAINING_BALANCE_RATIO,// [2] debt_ratio
        MAX_INTEREST_RATE,          // [3] interest_rate
        MAX_MONTHLY_INCOME,         // [4] monthly_income
        MAX_LOAN_TERM,              // [5] account_age_months (in months, so MAX_LOAN_TERM=120 is appropriate)
        100.0,                      // [6] age (max 100 years)
        MAX_EMPLOYMENT_RISK_SCORE,  // [7] employment_status (encoded)
        MAX_ACCOUNT_STATUS_RISK_SCORE// [8] account_status (encoded)
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

    if (features.daysPastDue >= DPD_HIGH_THRESHOLD)
    {
        score += 0.40;
    }
    else if (features.daysPastDue >= DPD_MEDIUM_THRESHOLD)
    {
        score += 0.30;
    }
    else if (features.daysPastDue >= DPD_LOW_THRESHOLD)
    {
        score += 0.20;
    }
    else 
    {
        score += features.daysPastDue / DPD_DEFAULT_THRESHOLD * 0.20;
    }
    
    score += std::min(0.30, features.numberOfMissedPayments * 0.10);
    
    double debtRatio = features.remainingAmount.getAmount() / (features.loanAmount.getAmount() + 1e-6);
    score += debtRatio * 0.20;
    
    // If no income (0 VND) and has debt -> VERY HIGH RISK
    double monthlyIncome = features.monthlyIncome.getAmount();
    // Calculate minimum monthly payment: (remaining * (1 + interest)) / 12 months
    double monthlyPaymentRequired = features.remainingAmount.getAmount() * (1.0 + features.interestRate) / 12.0;
    
    if (monthlyIncome <= 0 && features.remainingAmount.getAmount() > 0) {
        // No income + has debt = cannot repay!
        score += 0.35;  // Major risk factor
    } else if (monthlyIncome > 0) {
        // Debt-to-Income ratio (monthly payment / monthly income)
        double dti = monthlyPaymentRequired / (monthlyIncome + 1e-6);
        if (dti > 0.5) {
            score += 0.25;  // Payment > 50% of income
        } else if (dti > 0.35) {
            score += 0.15;  // Payment 35-50% of income
        } else if (dti > 0.20) {
            score += 0.10;  // Payment 20-35% of income (borderline)
        } else if (dti > 0.10) {
            score += 0.05;  // Payment 10-20% of income (manageable)
        }
        // dti <= 0.10 is very healthy, no additional risk
    }
    
    // Age factor: younger borrowers (18-25) and older borrowers (65+) have higher risk
    if (features.age > 0) {
        if (features.age < 25) {
            score += 0.10;  // Young, less financial stability
        } else if (features.age >= 65) {
            score += 0.08;  // Older, income concerns
        } else if (features.age >= 35 && features.age <= 50) {
            score -= 0.05;  // Prime working age, lower risk
        }
    }
    
    // Account age factor: new accounts are riskier
    if (features.accountAgeMonths > 0) {
        if (features.accountAgeMonths < 3) {
            score += 0.10;  // Very new account (< 3 months)
        } else if (features.accountAgeMonths < 6) {
            score += 0.05;  // New account (3-6 months)
        }
        // 6+ months is established, no additional risk
    }
    
    score += getEmploymentRiskContribution(features.employmentStatus);
    score += getAccountRiskContribution(features.accountStatus);
    
    return std::min(1.0, score);
}

void RiskScorer::trainModel()
{
    if (!_randomForest)
    {
        _randomForest = std::make_shared<RandomForest>(
            constants::risk::RF_NUM_TREES,
            constants::risk::RF_MAX_DEPTH,
            constants::risk::RF_MIN_SAMPLES_SPLIT
        );
    }
    
    std::vector<std::vector<double>> X;
    std::vector<double> y;
    generateSyntheticData(X, y, 5000);  // Increased to 5000 samples for better model generalization
    
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
        std::vector<double> features(9);  // Updated: 9 features including Age
        double riskScore;

        double scenario = uniform(rng);
        
        // EDGE CASE SCENARIOS (10% of data) - Critical for production safety
        if (scenario < 0.10)
        {
            // Zero or very low income with debt
            features[0] = uniform(rng) * 30.0;  // 0-30 days past due
            
            features[1] = (uniform(rng) < 0.5) ? 0.0 : std::floor(uniform(rng) * 2.0);
            
            features[2] = 0.7 + uniform(rng) * 0.25;  // High debt ratio
            
            features[3] = 0.12 + uniform(rng) * 0.10;
            
            // Zero or minimal income (0-500k VND)
            features[4] = uniform(rng) * 500000.0;
            
            // Very new accounts (0-6 months)
            features[5] = uniform(rng) * 6.0;
            
            // Age (risky profiles - young or retirement age)
            features[6] = (uniform(rng) < 0.5) ? 20.0 + uniform(rng) * 5.0 : 60.0 + uniform(rng) * 10.0;
            
            features[7] = (uniform(rng) < 0.7) ? 0.0 : 0.5;  // Mostly unemployed
            
            features[8] = (uniform(rng) < 0.6) ? 0.4 : 0.6;  // Delayed/Delinquent
            
            // HIGH RISK for zero/low income cases
            riskScore = 0.70 + uniform(rng) * 0.25;
        }
        // LOW RISK (40% of data) - Healthy borrowers
        else if (scenario < 0.50)
        {
            features[0] = uniform(rng) * 15.0;
            
            features[1] = (uniform(rng) < 0.8) ? 0.0 : 1.0;
           
            features[2] = 0.3 + uniform(rng) * 0.4;
            
            features[3] = 0.05 + uniform(rng) * 0.07;
            
            features[4] = 4000000.0 + uniform(rng) * 4000000.0;  // Good income (4-8M VND)
            
            features[5] = 12.0 + uniform(rng) * 36.0;
            
            // Prime working age (30-50)
            features[6] = 30.0 + uniform(rng) * 20.0;
            
            features[7] = (uniform(rng) < 0.9) ? 1.0 : 0.5;
            
            features[8] = 1.0;
            
            riskScore = 0.05 + uniform(rng) * 0.25;
        }
        // MEDIUM RISK (30% of data) - Average borrowers
        else if (scenario < 0.80)
        {
            features[0] = 15.0 + uniform(rng) * 45.0;

            features[1] = 1.0 + std::floor(uniform(rng) * 3.0);

            features[2] = 0.6 + uniform(rng) * 0.3;

            features[3] = 0.12 + uniform(rng) * 0.08;

            features[4] = 2500000.0 + uniform(rng) * 2000000.0;  // Medium income (2.5-4.5M VND)

            features[5] = 6.0 + uniform(rng) * 18.0;

            // Working age
            features[6] = 25.0 + uniform(rng) * 35.0;

            double emp = uniform(rng);
            features[7] = (emp < 0.5) ? 1.0 : 0.5;

            features[8] = (uniform(rng) < 0.7) ? 1.0 : 0.8;

            riskScore = 0.30 + uniform(rng) * 0.35;
        }
        // HIGH RISK (20% of data) - Problem accounts
        // HIGH RISK (20% of data) - Problem accounts
        else
        {
            features[0] = 60.0 + uniform(rng) * 120.0;

            features[1] = 3.0 + std::floor(uniform(rng) * 6.0);

            features[2] = 0.85 + uniform(rng) * 0.15;

            features[3] = 0.18 + uniform(rng) * 0.12;

            features[4] = 1000000.0 + uniform(rng) * 2000000.0;  // Low income (1-3M VND)

            features[5] = 3.0 + uniform(rng) * 15.0;

            // Broad age range
            features[6] = 20.0 + uniform(rng) * 50.0;

            double emp = uniform(rng);
            features[7] = (emp < 0.6) ? 0.0 : 0.5;

            double status = uniform(rng);
            if (status < 0.4) features[8] = 0.0;
            else if (status < 0.7) features[8] = 0.4;
            else features[8] = 0.6;

            riskScore = 0.65 + uniform(rng) * 0.30;
        }
        
        X.push_back(features);
        y.push_back(riskScore);
    }
}

void RiskScorer::loadModel(const std::string& modelPath) { (void)modelPath; }
bool RiskScorer::isModelReady() const { return _useMLModel && _isModelTrained && _randomForest && _randomForest->isTrained(); }
void RiskScorer::setUseMLModel(bool useML) { _useMLModel = useML; }

double RiskScorer::encodeEmploymentStatus(EmploymentStatus status)
{
    switch (status)
    {
    case EmploymentStatus::Employed:
    case EmploymentStatus::Contract:
    case EmploymentStatus::SelfEmployed:
        return 1.0;
    case EmploymentStatus::PartTime:
    case EmploymentStatus::Student:
        return 0.5;
    case EmploymentStatus::Unemployed:
    case EmploymentStatus::Retired:
    case EmploymentStatus::None:
        return 0.0;
    default:
        return 0.0;
    }
}

double RiskScorer::getEmploymentRiskContribution(EmploymentStatus status)
{
    switch (status)
    {
    case EmploymentStatus::Employed:
    case EmploymentStatus::Contract:
    case EmploymentStatus::SelfEmployed:
        return 0.0;
    case EmploymentStatus::PartTime:
    case EmploymentStatus::Student:
        return 0.05;
    case EmploymentStatus::Unemployed:
    case EmploymentStatus::Retired:
    case EmploymentStatus::None:
        return 0.10;
    default:
        return 0.10;
    }
}

double RiskScorer::encodeAccountStatus(AccountStatus status)
{
    switch (status)
    {
    case AccountStatus::PaidOff:
    case AccountStatus::Current:
        return 1.0;
    case AccountStatus::Partial:
        return 0.8;
    case AccountStatus::Settled:
        return 0.6;
    case AccountStatus::Delinquent:
        return 0.4;
    case AccountStatus::Default:
    case AccountStatus::ChargedOff:
        return 0.0;
    default:
        return 0.5;
    }
}

double RiskScorer::getAccountRiskContribution(AccountStatus status)
{
    switch (status)
    {
    case AccountStatus::PaidOff:
    case AccountStatus::Current:
        return 0.0;
    case AccountStatus::Partial:
        return 0.02;
    case AccountStatus::Settled:
        return 0.04;
    case AccountStatus::Delinquent:
        return 0.06;
    case AccountStatus::Default:
    case AccountStatus::ChargedOff:
        return 0.10;
    default:
        return 0.10;
    }
}

std::map<std::string, double> RiskScorer::calculateFeatureContributions(const RiskFeatures& features, double /* finalScore */) const
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