// KMeansClustering.cpp - Implementation

#include "../../include/algorithms/KMeansClustering.h"
#include "../../../common/include/exceptions/ValidationException.h"
#include <numeric>
#include <cmath>
#include <algorithm>

using namespace sdrs::constants::risk;
using namespace sdrs::exceptions;

namespace sdrs::risk
{

KMeansClustering::KMeansClustering(int k, int maxIterations)
    : _k(k),
    _maxIterations(maxIterations),
    _isTrained(false),
    _randomEngine(std::random_device{}())
{
    if (k <= 0)
    {
        throw ValidationException("K must be positive number", "KMean");
    }
}

double KMeansClustering::euclideanDistance(const std::vector<double>& a, const std::vector<double>& b) const
{
    double sumSquared = 0.0;
    for (size_t i = 0; i < a.size(); ++i)
    {
        double diff = a[i] - b[i];
        sumSquared += diff * diff;
    }
    return std::sqrt(sumSquared);
}

void KMeansClustering::initializeCentroids(const std::vector<std::vector<double>>& X)
{
    size_t n = X.size();

    std::vector<size_t> indices(n);
    std::iota(indices.begin(), indices.end(), 0);

    std::shuffle(indices.begin(), indices.end(), _randomEngine);

    _centroids.clear();
    _centroids.reserve(_k);

    for (int i = 0; i < _k; ++i)
    {
        _centroids.push_back(X[indices[i]]);
    }
}

void KMeansClustering::assignClusters(const std::vector<std::vector<double>>& X)
{
    _labels.resize(X.size());

    for (size_t i = 0; i < X.size(); ++i)
    {
        _labels[i] = findNearestCentroid(X[i]);
    }
}

int KMeansClustering::findNearestCentroid(const std::vector<double>& points) const
{
    int nearestCluster = 0;
    double minDistance = std::numeric_limits<double>::max();

    for (int i = 0; i < _k; ++i)
    {
        double dist = euclideanDistance(points, _centroids[i]);
    
        if (dist < minDistance)
        {
            minDistance = dist;
            nearestCluster = i;
        }
    }
    return nearestCluster;
}

bool KMeansClustering::updateCentroids(const std::vector<std::vector<double>>& X)
{
    size_t numFeatures = X[0].size();
    std::vector<std::vector<double>> newCentroids(_k, std::vector<double>(numFeatures, 0.0));
    std::vector<int> clusterCounts(_k, 0);

    for (size_t i = 0; i < X.size(); ++i)
    {
        int cluster = _labels[i];
        clusterCounts[cluster]++;

        for (size_t j = 0; j < numFeatures; ++j)
        {
            newCentroids[cluster][j] += X[i][j];
        }
    }

    for (int i = 0; i < _k; ++i)
    {
        if (clusterCounts[i] > 0)
        {
            for (size_t j = 0; j < numFeatures; ++j)
            {
                newCentroids[i][j] /= clusterCounts[i];
            }
        }
        else
        {
            newCentroids[i] = _centroids[i];
        }
    }

    double totalShift = 0.0;
    for (int i = 0; i < _k; ++i)
    {
        totalShift += euclideanDistance(_centroids[i], newCentroids[i]);
    }

    _centroids = std::move(newCentroids);

    return totalShift < KMEANS_TOLERANCE;
}

void KMeansClustering::train(const std::vector<std::vector<double>>& X)
{
    if (X.empty())
    {
        throw ValidationException("Training data cannot be empty", "KMean");
    }

    if (static_cast<int>(X.size()) < _k)
    {
        throw ValidationException("Not enough data points for K clusters", "KMean");
    }

    initializeCentroids(X);

    for (int iter = 0; iter < _maxIterations; ++iter)
    {
        assignClusters(X);

        bool converged = updateCentroids(X);

        if (converged)
        {
            break;
        }
    }
    _isTrained = true;
}

ClusterResult KMeansClustering::predict(const std::vector<double>& points) const
{
    if (!_isTrained)
    {
        throw ValidationException("Model must be trained before prediction", "KMean");
    }

    if (points.size() != _centroids[0].size())
    {
        throw ValidationException("Input features size ("
            + std::to_string(points.size())
            + ") does not match trained features size ("
            + std::to_string(_centroids[0].size())
            + ")", "KMean"
        );
    }

    int clusterId = findNearestCentroid(points);
    double distance = euclideanDistance(points, _centroids[clusterId]);

    return ClusterResult {
        clusterId,
        distance,
        _centroids[clusterId]
    };
}

std::vector<int> KMeansClustering::predictBatch(const std::vector<std::vector<double>>& X) const
{
    std::vector<int> predictions;
    predictions.reserve(X.size());

    for (const auto& point : X)
    {
        predictions.push_back(predict(point).clusterId);
    }

    return predictions;
}

double KMeansClustering::getInertia() const
{
    return 0.0;
}

bool KMeansClustering::isTrained() const { return _isTrained; }
int KMeansClustering::getK() const { return _k; }
const std::vector<std::vector<double>>& KMeansClustering::getCentroids() const { return _centroids; }
const std::vector<int>& KMeansClustering::getLabels() const { return _labels; }

}