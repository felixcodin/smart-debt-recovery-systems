#include "../../include/algorithms/RandomForest.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>

namespace sdrs::risk
{

TreeNode::TreeNode()
    : featureIndex(-1),
    threshold(0.0),
    leafValue(0.0),
    leftChild(nullptr),
    rightChild(nullptr)
{
    // Do nothing
}

bool TreeNode::isLeaf() const
{
    return featureIndex == -1;
}

DecisionTree::DecisionTree(int maxDepth, int minSamplesSplit)
    : _root(nullptr),
    _maxDepth(maxDepth),
    _minSamplesSplit(minSamplesSplit),
    _currentDepth(0),
    _randomEngine(std::random_device{}())
{
    // Do nothing
}

DecisionTree::~DecisionTree() = default;

void DecisionTree::train(const std::vector<std::vector<double>>& X, const std::vector<double>& y)
{
    if ((X.empty())
    || (y.empty())
    || (X.size() != y.size()))
    {
        return;
    }
    
    _root = buildTree(X, y, 0);
    _currentDepth = 0;
}

std::unique_ptr<TreeNode> DecisionTree::buildTree(
    const std::vector<std::vector<double>>& X,
    const std::vector<double>& y,
    int depth)
{
    auto node = std::make_unique<TreeNode>();
    
    if (depth >= _maxDepth)
    {
        node->featureIndex = -1;
        node->leafValue = calculateMean(y);
        return node;
    }
    
    if (X.size() < static_cast<size_t>(_minSamplesSplit))
    {
        node->featureIndex = -1;
        node->leafValue = calculateMean(y);
        return node;
    }
    
    double variance = calculateVariance(y);
    if (variance < VARIANCE_EPSILON)
    {
        node->featureIndex = -1;
        node->leafValue = calculateMean(y);
        return node;
    }
    
    int bestFeature = -1;
    double bestThreshold = 0.0;
    double bestGain = 0.0;
    
    findBestSplit(X, y, bestFeature, bestThreshold, bestGain);
    
    if ((bestFeature == -1)
    || (bestGain <= 0.0))
    {
        node->featureIndex = -1;
        node->leafValue = calculateMean(y);
        return node;
    }
    
    std::vector<std::vector<double>> leftX, rightX;
    std::vector<double> leftY, rightY;
    
    splitData(X, y, bestFeature, bestThreshold, leftX, leftY, rightX, rightY);
    
    if ((leftX.empty())
    || (rightX.empty()))
    {
        node->featureIndex = -1;
        node->leafValue = calculateMean(y);
        return node;
    }
    
    node->featureIndex = bestFeature;
    node->threshold = bestThreshold;
    node->leftChild = buildTree(leftX, leftY, depth + 1);
    node->rightChild = buildTree(rightX, rightY, depth + 1);
    
    return node;
}

void DecisionTree::findBestSplit(
    const std::vector<std::vector<double>>& X,
    const std::vector<double>& y,
    int& bestFeature,
    double& bestThreshold,
    double& bestGain) const
{
    bestFeature = -1;
    bestThreshold = 0.0;
    bestGain = -std::numeric_limits<double>::infinity();
    
    if (X.empty()) return;
    
    int sampleCount = X.size();
    int numFeatures = X[0].size();
    double parentVariance = calculateVariance(y);
    
    _tempFeatureValues.clear();
    _tempFeatureValues.reserve(sampleCount);

    _tempLeftY.clear();
    _tempRightY.clear();
    _tempLeftY.reserve(sampleCount);
    _tempRightY.reserve(sampleCount);

    for (int featureIdx = 0; featureIdx < numFeatures; ++featureIdx)
    {
        _tempFeatureValues.clear();
        for (const auto& sample : X)
        {
            _tempFeatureValues.push_back(sample[featureIdx]);
        }
        
        std::sort(_tempFeatureValues.begin(), _tempFeatureValues.end());
        _tempFeatureValues.erase(
            std::unique(_tempFeatureValues.begin(), _tempFeatureValues.end()),
            _tempFeatureValues.end()
        );
        
        for (size_t i = 0; i + 1 < _tempFeatureValues.size(); ++i)
        {
            std::uniform_real_distribution<double> dist(_tempFeatureValues[i], _tempFeatureValues[i + 1]);
            double threshold = dist(_randomEngine);
            
            _tempLeftY.clear();
            _tempRightY.clear();
            for (size_t j = 0; j < X.size(); ++j)
            {
                if (X[j][featureIdx] < threshold)
                {
                    _tempLeftY.push_back(y[j]);
                }
                else
                {
                    _tempRightY.push_back(y[j]);
                }
            }
            
            if ((_tempLeftY.empty()) 
            || (_tempRightY.empty()))
            {
                continue;
            }
            
            double leftVariance = calculateVariance(_tempLeftY);
            double rightVariance = calculateVariance(_tempRightY);
            
            double weightedVariance = (_tempLeftY.size() * leftVariance + _tempRightY.size() * rightVariance) / (_tempLeftY.size() + _tempRightY.size());
            
            double gain = parentVariance - weightedVariance;
            
            if (gain > bestGain)
            {
                bestGain = gain;
                bestFeature = featureIdx;
                bestThreshold = threshold;
            }
        }
    }
}

double DecisionTree::calculateVariance(const std::vector<double>& values) const
{
    if (values.empty())
    {
        return 0.0;
    }
    
    double mean = calculateMean(values);
    double variance = 0.0;
    
    for (double val : values)
    {
        variance += (val - mean) * (val - mean);
    }
    
    return variance / values.size();
}

double DecisionTree::calculateMean(const std::vector<double>& values) const
{
    if (values.empty())
    {
        return 0.0;
    }
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

void DecisionTree::splitData(
    const std::vector<std::vector<double>>& X,
    const std::vector<double>& y,
    int featureIdx,
    double threshold,
    std::vector<std::vector<double>>& leftX,
    std::vector<double>& leftY,
    std::vector<std::vector<double>>& rightX,
    std::vector<double>& rightY) const
{
    leftX.clear();
    leftY.clear();
    rightX.clear();
    rightY.clear();
    
    for (size_t i = 0; i < X.size(); ++i)
    {
        if (X[i][featureIdx] < threshold)
        {
            leftX.push_back(X[i]);
            leftY.push_back(y[i]);
        }
        else
        {
            rightX.push_back(X[i]);
            rightY.push_back(y[i]);
        }
    }
}

double DecisionTree::predict(const std::vector<double>& features) const
{
    if (!_root) return 0.0;
    
    return predictRecursive(_root.get(), features);
}

double DecisionTree::predictRecursive(const TreeNode* node, const std::vector<double>& features) const
{
    if (node->isLeaf())
    {
        return node->leafValue;
    }
    
    if (features[node->featureIndex] < node->threshold)
    {
        return predictRecursive(node->leftChild.get(), features);
    }
    else
    {
        return predictRecursive(node->rightChild.get(), features);
    }
}

bool DecisionTree::isTrained() const
{
    return _root != nullptr;
}

int DecisionTree::getDepth() const
{
    return _currentDepth;
}

RandomForest::RandomForest(int numTrees, int maxDepth, int minSamples)
    : _numTrees(numTrees),
    _maxDepth(maxDepth),
    _minSamplesSplit(minSamples),
    _isTrained(false),
    _randomEngine(std::random_device{}())
{
}

RandomForest::~RandomForest() = default;

void RandomForest::train(const std::vector<std::vector<double>>& X, const std::vector<double>& y)
{
    if ((X.empty())
    || (y.empty()))
    {
        return;
    }
    
    _trees.clear();
    _trees.reserve(_numTrees);
    
    for (int i = 0; i < _numTrees; ++i)
    {
        std::vector<std::vector<double>> sampleX;
        std::vector<double> sampleY;
        
        bootstrapSample(X, y, sampleX, sampleY);
        
        auto tree = std::make_unique<DecisionTree>(_maxDepth, _minSamplesSplit);
        tree->train(sampleX, sampleY);
        
        _trees.push_back(std::move(tree));
    }
    
    _isTrained = true;
}

void RandomForest::bootstrapSample(
    const std::vector<std::vector<double>>& X,
    const std::vector<double>& y,
    std::vector<std::vector<double>>& sampleX,
    std::vector<double>& sampleY)
{
    sampleX.clear();
    sampleY.clear();
    
    size_t numSamples = X.size();
    std::uniform_int_distribution<size_t> dist(0, numSamples - 1);
    
    for (size_t i = 0; i < numSamples; ++i)
    {
        size_t idx = dist(_randomEngine);
        sampleX.push_back(X[idx]);
        sampleY.push_back(y[idx]);
    }
}

double RandomForest::predict(const std::vector<double>& features) const
{
    if ((!_isTrained)
    || (_trees.empty()))
    {
        return 0.0;
    }
    
    double sum = 0.0;
    int validTrees = 0;
    
    for (const auto& tree : _trees)
    {
        if (tree && tree->isTrained())
        {
            sum += tree->predict(features);
            ++validTrees;
        }
    }
    
    return validTrees > 0 ? (sum / validTrees) : 0.0;
}

bool RandomForest::isTrained() const
{
    return _isTrained;
}

int RandomForest::getNumTrees() const
{
    return _numTrees;
}

}
