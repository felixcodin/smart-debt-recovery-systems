// KMeansClustering.h - K-Means clustering for borrower segmentation

#ifndef SDRS_RISK_KMEANS_CLUSTERING_H
#define SDRS_RISK_KMEANS_CLUSTERING_H

#include "../../../common/include/utils/Constants.h"
#include <vector>
#include <random>

namespace sdrs::risk
{

// Cluster assignment result
struct ClusterResult
{
    int clusterId;              // 0 to k-1
    double distanceToCentroid;  // how far from cluster center
    std::vector<double> centroid;
};

// Groups borrowers into k clusters based on risk features
class KMeansClustering
{
private:
    int _k;
    int _maxIterations;
    bool _isTrained;

    std::vector<std::vector<double>> _centroids;
    std::vector<int> _labels;

    mutable std::mt19937 _randomEngine;

public:
    KMeansClustering(int k = sdrs::constants::risk::KMEANS_NUM_CLUSTERS, int maxIterations = sdrs::constants::risk::KMEANS_MAX_ITERATIONS);
    ~KMeansClustering() = default;

    void train(const std::vector<std::vector<double>>& X);           // find k centroids
    ClusterResult predict(const std::vector<double>& points) const;  // assign to nearest cluster
    std::vector<int> predictBatch(const std::vector<std::vector<double>>& X) const;

public:
    bool isTrained() const;
    int getK() const;
    const std::vector<std::vector<double>>& getCentroids() const;
    const std::vector<int>& getLabels() const;
    double getInertia() const;  // sum of squared distances (lower = better fit)

private:
    void initializeCentroids(const std::vector<std::vector<double>>& X);
    void assignClusters(const std::vector<std::vector<double>>& X);
    bool updateCentroids(const std::vector<std::vector<double>>& X);
    double euclideanDistance(const std::vector<double>& a, const std::vector<double>& b) const;
    int findNearestCentroid(const std::vector<double>& points) const;

};

}

#endif