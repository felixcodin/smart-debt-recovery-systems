#ifndef SDRS_RISK_KMEANS_CLUSTERING_H
#define SDRS_RISK_KMEANS_CLUSTERING_H

#include <vector>
#include <random>

namespace sdrs::risk
{

struct ClusterResult
{
    int clusterId;
    double distanceToCentroid;
    std::vector<double> centroid;
};

class KMeansClustering
{
private:
    int _k;
    static constexpr double MAX_TOLERANCE = 1e-4;
    int _maxIterations;
    bool _isTrained;

    std::vector<std::vector<double>> _centroids;
    std::vector<int> _labels;

    mutable std::mt19937 _randomEngine;

public:
    KMeansClustering(int k = 3, int maxIterations = 100);
    ~KMeansClustering() = default;

    void train(const std::vector<std::vector<double>>& X);
    ClusterResult predict(const std::vector<double>& points) const;
    std::vector<int> predictBatch(const std::vector<std::vector<double>>& X) const;

public:
    bool isTrained() const;
    int getK() const;
    const std::vector<std::vector<double>>& getCentroids() const;
    const std::vector<int>& getLabels() const;
    double getInertia() const;

private:
    void initializeCentroids(const std::vector<std::vector<double>>& X);
    void assignClusters(const std::vector<std::vector<double>>& X);
    bool updateCentroids(const std::vector<std::vector<double>>& X);
    double euclideanDistance(const std::vector<double>& a, const std::vector<double>& b) const;
    int findNearestCentroid(const std::vector<double>& points) const;

};

}

#endif