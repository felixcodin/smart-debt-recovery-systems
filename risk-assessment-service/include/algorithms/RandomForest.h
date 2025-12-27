// RandomForest.h - Random Forest algorithm for risk prediction

#ifndef SDRS_RISK_RANDOMFOREST_H
#define SDRS_RISK_RANDOMFOREST_H

#include "../../../common/include/utils/Constants.h"
#include <vector>
#include <memory>
#include <random>
#include <map>

namespace sdrs::risk
{

struct TreeNode
{
    int featureIndex;
    double threshold;
    double leafValue;
    
    std::unique_ptr<TreeNode> leftChild;
    std::unique_ptr<TreeNode> rightChild;
    
    TreeNode();
    
    bool isLeaf() const;
};

class DecisionTree
{
private:
    std::unique_ptr<TreeNode> _root;
    int _maxDepth;
    int _minSamplesSplit;
    int _currentDepth;
    
    mutable std::mt19937 _randomEngine;
    mutable std::vector<double> _tempFeatureValues;
    mutable std::vector<double> _tempLeftY;
    mutable std::vector<double> _tempRightY;

public:
    DecisionTree(int maxDepth = sdrs::constants::risk::RF_MAX_DEPTH, int minSamplesSplit = sdrs::constants::risk::RF_MIN_SAMPLES_SPLIT);
    ~DecisionTree();

    void train(const std::vector<std::vector<double>>& X, const std::vector<double>& y);  // X=features, y=labels
    double predict(const std::vector<double>& features) const;  // returns predicted value
    bool isTrained() const;
    int getDepth() const;

private:
    std::unique_ptr<TreeNode> buildTree(const std::vector<std::vector<double>>& X, const std::vector<double>& y, int depth);
    
    void findBestSplit(
        const std::vector<std::vector<double>>& X,
        const std::vector<double>& y,
        int& bestFeature,
        double& bestThreshold,
        double& bestGain
    ) const;

    double calculateVariance(const std::vector<double>& values) const;
    
    double calculateMean(const std::vector<double>& values) const;
    
    void splitData(
        const std::vector<std::vector<double>>& X,
        const std::vector<double>& y,
        int featureIdx,
        double threshold,
        std::vector<std::vector<double>>& leftX,
        std::vector<double>& leftY,
        std::vector<std::vector<double>>& rightX,
        std::vector<double>& rightY
    ) const;
    
    double predictRecursive(const TreeNode* node, const std::vector<double>& features) const;
};

// Ensemble of decision trees - averages predictions for better accuracy
class RandomForest
{
private:
    std::vector<std::unique_ptr<DecisionTree>> _trees;
    
    int _numTrees;
    int _maxDepth;
    int _minSamplesSplit;
    bool _isTrained;
    
    std::mt19937 _randomEngine;

public:
    RandomForest(
        int numTrees = sdrs::constants::risk::RF_NUM_TREES,
        int maxDepth = sdrs::constants::risk::RF_MAX_DEPTH,
        int minSamples = sdrs::constants::risk::RF_MIN_SAMPLES_SPLIT
    );
    ~RandomForest();
    
    void train(const std::vector<std::vector<double>>& X, const std::vector<double>& y);  // trains all trees with bootstrap sampling
    double predict(const std::vector<double>& features) const;  // returns average of all tree predictions
    bool isTrained() const;
    int getNumTrees() const;
    const std::map<int, double>& getFeatureImportances() const;  // which features matter most

private:
    void bootstrapSample(
        const std::vector<std::vector<double>>& X,
        const std::vector<double>& y,
        std::vector<std::vector<double>>& sampleX,
        std::vector<double>& sampleY
    );
};

}

#endif