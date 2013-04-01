
#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#include <vector>
#include <cv.h>
using namespace std;

typedef vector<CvPoint> Points;

class Cluster
{
public:
    Cluster();

    int cluster(const Points &points, int clusterNum, double mergeLength,
            vector<Points> &clusteredPts, Points &centers);

    ~Cluster();

private:
    double dist(const CvPoint &point1, const CvPoint &point2);
    void kMeans(const Points &pts, int clusterNum, vector<Points> &clusteredPts);
    void packIntoClusteredPts(int clusterNum, const CvMat* points,
            const CvMat* clusters, vector<Points> &clusteredPts);
    void removeEmptyCluster(vector<Points> &clusteredPts);
    void getClusterCenters(const vector<Points> &clusteredPts, Points &centers);
    int mergeClusters(double mergeLength, Points &centers,
            vector<Points> &clusteredPts);
    CvPoint findMostSimilar(double mergeLength, bool* mergeDone, Points &centers);
    void merge(CvPoint toFrom, Points &centers, vector<Points> &clusteredPts);
};

#endif
