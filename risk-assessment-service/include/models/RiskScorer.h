#ifndef RISK_SCORER_H
#define RISK_SCORER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <chrono>

#include "../../../common/include/models/Money.h"
#include "../../../borrower-service/include/models/Borrower.h"
#include "../../../borrower-service/include/models/LoanAccount.h"

namespace sdrs::risk
{

enum class RiskLevel
{
    Low, // < 0.33
    Medium, // 0.33 - 0.67
    High // >= 0.67
};

class RandomForest;

enum class AlgorithmUsed
{
    RandomForest,
    RuleBase
};

class RiskAssessment
{
private:
    int _assessmentId;
    int _accountId;
    int _borrowerId;
    double _riskScore;
    RiskLevel _riskLevel;
    AlgorithmUsed _algorithmUsed;
    std::map<std::string, double> _riskFactors;
    std::chrono::sys_seconds _assessmentDate;
    std::chrono::sys_seconds _createdAt;

public:
    RiskAssessment() = delete;
    RiskAssessment(int accountId, int borrowerId, double score, AlgorithmUsed algorithm);

public:
    int getAssessmentId() const;
    int getAccountId() const;
    int getBorrowerId() const;
    RiskLevel getRiskLevel() const;
    AlgorithmUsed getAlgorithmUsed() const;
    std::chrono::sys_seconds getAssessmentDate() const;
    std::chrono::sys_seconds getCreatedAt() const;

    const std::map<std::string, double>& getRiskFactors() const;
    void addRiskFactor(const std::string& factorName, double contribution);

    std::string toJson() const;
    static std::string riskLevelToString(RiskLevel level);
    static RiskLevel stringToRiskLevel(const std::string& levelStr);

private:
    RiskLevel determineRiskLevel(double score) const;
    void touch();

};

// Input data for risk assessment (combines loan and borrower information)
struct RiskFeatures
{
    // Loan account data
    int accountId = 0;
    int borrowerId = 0;
    sdrs::money::Money loanAmount{0.0};
    sdrs::money::Money remainingAmount{0.0};
    double interestRate = 0.0;
    int daysPastDue = 0;
    int numberOfMissedPayments = 0;
    int loanTermMonths = 0;
    sdrs::borrower::AccountStatus accountStatus;

    // Borrower data
    sdrs::money::Money monthlyIncome;
    sdrs::borrower::EmploymentStatus employmentStatus;  // Enum: Employed, Unemployed, SelfEmployed, etc.

    // Calculated metrics
    int accountAgeMonths;
    double debtToIncomeRatio;

    RiskFeatures();
    void validate() const;
};

// Assess loan risk using ML (RandomForest) or Rule-Based algorithm
// OOP: Encapsulation + Composition (contains RandomForest) + Strategy Pattern (ML vs Rule)
class RiskScorer
{
private:
    std::shared_ptr<RandomForest> _randomForest;
    bool _useMLModel;
    bool _isModelTrained;

    // Feature normalization constants
    static constexpr double MAX_DAYS_PAST_DUE = 365.0;
    static constexpr double MAX_MISSED_PAYMENTS = 12.0;
    static constexpr double MAX_INTEREST_RATE = 0.30;
    static constexpr double MAX_MONTHLY_INCOME = 50000.0; // USD
    static constexpr double MAX_LOAN_TERM = 120.0; // Months
    static constexpr double MAX_REMAINING_BALANCE_RATIO = 1.0;
    static constexpr double MAX_EMPLOYMENT_RISK_SCORE = 1.0;
    static constexpr double MAX_ACCOUNT_STATUS_RISK_SCORE = 1.0;

public:
    RiskScorer();
    ~RiskScorer();

    // Assess risk for a loan account, returns RiskAssessment with score, level, and factors
    RiskAssessment assessRisk(const RiskFeatures& features);

    // Train RandomForest model with synthetic data (10 trees, depth=7)
    void trainModel();
    
    // Load pre-trained model from file (not yet implemented)
    void loadModel(const std::string& modelPath);
    
    // Check if model is ready to use
    bool isModelReady() const;
    
    // Toggle ML model on/off (if off, falls back to rule-based scoring)
    void setUseMLModel(bool useML);

private:
    std::vector<double> extractFeatures(const RiskFeatures& input) const;
    std::vector<double> normalizeFeatures(const std::vector<double>& raw) const;

    double predictWithML(const std::vector<double>& features) const;
    double calculateRuleBasedScore(const RiskFeatures& features) const;

    std::map<std::string, double> calculateFeatureContributions(const RiskFeatures& features, double finalScore) const;
    
    static double encodeEmploymentStatus(sdrs::borrower::EmploymentStatus status);
    static double getEmploymentRiskContribution(sdrs::borrower::EmploymentStatus status);

    static double encodeAccountStatus(sdrs::borrower::AccountStatus status);
    static double getAccountRiskContribution(sdrs::borrower::AccountStatus status);
    
    // Generate synthetic training data for ML model
    void generateSyntheticData(
        std::vector<std::vector<double>>& X,
        std::vector<double>& y,
        int numSamples
    ) const;
};

}

#endif