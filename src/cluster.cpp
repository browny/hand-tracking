
#include "../include/cluster.h"

Cluster::Cluster() {}

int Cluster::cluster(const Points &points, int clusterNum, double mergeLength,
        vector<Points> &clusteredPts, Points &centers) {

    kMeans(points, clusterNum, clusteredPts);
    getClusterCenters(clusteredPts, centers);

    return mergeClusters(mergeLength, centers, clusteredPts);
}

void Cluster::kMeans(const Points &pts, int clusterNum,
        vector<Points> &clusteredPts) {

    int length = pts.size();

    // Preparing the input data format
    CvMat* points   = cvCreateMat(length, 1, CV_32FC2);
    CvMat* clusters = cvCreateMat(length, 1, CV_32SC1);

    for (int row = 0; row < points->rows; row++) {
        float* ptr = (float*) (points->data.ptr + row * points->step);

        for (int col = 0; col < points->cols; col++) {
            *ptr = static_cast<float> (pts[row].x);
            ptr++;
            *ptr = static_cast<float> (pts[row].y);
        }
    }

    // The Kmeans algorithm function (OpenCV function)
    cvKMeans2(points, clusterNum, clusters,
            cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 1, 2));

    // Pack result to 'clusteredPts': each element in 'clusteredPts' means one cluster,
    // each cluster is one vector<CvPoint> which contains all points belong to that cluster
    packIntoClusteredPts(clusterNum, points, clusters, clusteredPts);

    removeEmptyCluster(clusteredPts);

    cvReleaseMat(&points);
    cvReleaseMat(&clusters);
}

void Cluster::packIntoClusteredPts(int clusterNum, const CvMat* points,
        const CvMat* clusters, vector<Points> &clusteredPts) {
    Points tempPts;
    for (int i = 0; i < clusterNum; i++) {
        tempPts.clear();

        for (int row = 0; row < clusters->rows; row++) {
            float* p_point = (float*) (points->data.ptr + row * points->step);
            int X = static_cast<int> (*p_point);
            p_point++;
            int Y = static_cast<int> (*p_point);

            if (clusters->data.i[row] == i)
                tempPts.push_back(cvPoint(X, Y));
        }

        clusteredPts.push_back(tempPts);
    }
}

void Cluster::removeEmptyCluster(vector<Points> &clusteredPts) {
    for (unsigned int i = 0; i < clusteredPts.size(); ++i) {
        if (clusteredPts[i].empty()) {
            vector<Points>::iterator it = clusteredPts.begin();
            it = it + i;
            clusteredPts.erase(it);
            i = i - 1;
        }
    }
}

void Cluster::getClusterCenters(const vector<Points> &clusteredPts, Points &centers) {
    for (unsigned int i = 0; i < clusteredPts.size(); i++) {
        int x = 0, y = 0;
        vector<CvPoint> tempClass(clusteredPts[i]);

        for (unsigned int j = 0; j < tempClass.size(); j++) {
            x += tempClass[j].x;
            y += tempClass[j].y;
        }

        centers.push_back(cvPoint(x / tempClass.size(), y / tempClass.size()));
    }
}

int Cluster::mergeClusters(double mergeLength, Points &centers, vector<Points> &clusteredPts) {
    bool mergeDone = false;
    while (mergeDone != true) {
        CvPoint toFrom = findMostSimilar(mergeLength, &mergeDone, centers);

        if (mergeDone == false)
            merge(toFrom, centers, clusteredPts);
    }
    return centers.size();
}

CvPoint Cluster::findMostSimilar(double mergeLength, bool* mergeDone, Points &centers) {

    *mergeDone = true;

    int mergeToIndex = 0;
    int mergeFromIndex = 0;
    double centerDis = 0;
    double minDis = 1000000;

    for (unsigned int i = 0; i < centers.size() - 1; ++i) {
        for (unsigned int j = i + 1; j < centers.size(); ++j) {
            centerDis = dist(centers[i], centers[j]);

            if ( (centerDis < mergeLength) && (centerDis < minDis) ) {
                minDis = centerDis;

                if (i < j) {
                    mergeToIndex = i;
                    mergeFromIndex = j;
                } else {
                    mergeToIndex = j;
                    mergeFromIndex = i;
                }

                *mergeDone = false;
            }
        }
    }

    return cvPoint(mergeToIndex, mergeFromIndex);
}

void Cluster::merge(CvPoint toFrom, Points &centers, vector<Points> &clusteredPts) {

    vector<Points> mergedPts;
    Points mergedCenters;
    unsigned int toIdx = (unsigned int) toFrom.x;
    unsigned int frIdx = (unsigned int) toFrom.y;

    for (unsigned int i = 0; i < centers.size(); ++i) {
        if (i == toIdx) {
            vector<CvPoint> tempClass(clusteredPts[toIdx]);
            tempClass.insert(tempClass.end(), clusteredPts[frIdx].begin(),
                    clusteredPts[frIdx].end());
            mergedPts.push_back(tempClass);
        } else if ((i != toIdx) && (i != frIdx)) {
            mergedPts.push_back(clusteredPts[i]);
        }
    }

    getClusterCenters(mergedPts, mergedCenters);

    clusteredPts.clear();
    centers.clear();

    clusteredPts = mergedPts;
    centers = mergedCenters;
}

double Cluster::dist(const CvPoint &point1, const CvPoint &point2) {

    return sqrt((double) ((point1.x - point2.x) * (point1.x - point2.x) +
                (point1.y - point2.y) * (point1.y - point2.y)));
}

Cluster::~Cluster() {}
