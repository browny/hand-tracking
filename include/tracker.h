
#ifndef _TRACKER_H_
#define _TRACKER_H_

#include <vector>
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include "cluster.h"

using namespace std;

typedef vector<CvPoint> Points;
typedef vector<CvRect> Rects;

class Tracker
{
public:
    Tracker(CvSize imgSz);

    Points currentHands;

    void setTrackNum(int num);
    void track(const IplImage* src, const IplImage* skinImg, Points &hands);

    ~Tracker();

private:
    const int kCcNum;
    const int kCcAreaRemoveTh;
    const int kHandBufferLength;
    const int kClusteredCenterBufferLength;
    const double kMotionRatio;
    const double kDarkAvgGrayTh;
    const double kCcSizeTh;
    const double kClusteredCenterMergeTh;

    int m_trackNum;
    int m_adptResampleMargin;
    int m_adptCcRemoveAreaTh;
    int m_adptClusterCheckLength;
    int m_adptHandBufferAvgLength;
    double m_adptDiffTh;
    double m_adptClusteredCenterMergeTh;

    vector<Points> m_handBuffers;
    vector<Points> m_clusteredCenterBuffers;

    IplImage* m_src;
    IplImage* m_srcGray;
    IplImage* m_srcGrayOld;
    IplImage* m_diffImg;
    IplImage* m_motionSkinImg;
    IplImage* m_resampledSkinImg;

    Cluster m_cluster;

    void getMovingSkinImg(const IplImage* src, const IplImage* skinImg);
    void getCandidates(const IplImage* skinImg, IplImage* motionSkinImg, int* num,
            Rects &rects, Points &centers);
    void clusterCandidates(const Points &centers);
    void getHands();

    //--- Helper methods ---//
    double avgGrayValue(const IplImage* src);
    void getDiffImg(const IplImage* img1, const IplImage* img2, IplImage* diff);
    void connectComponent(IplImage* src, const int poly_hull0,
            const float perimScale, int* num, Rects &rects, Points &centers);
    void removeSmallCcs(int* ccNum, Rects &rects, Points &centers);
    void resampleByPoints(const IplImage* input, const int srMar,
            const Points &points, IplImage* output);
    void drawCcs(const int num, const Rects &rects, const Points &centers,
            IplImage* targetFrame);
    template <class myType>
        void shiftVector(vector<myType> &vec, myType element);
    bool getEmptyIndex(const vector<Points> &massbuffer, vector<bool> &empty);
    int getClosestCenterBufIndex(CvPoint mass, const vector<Points> &buf,
            const vector<bool> &empty);
    double distance(const CvPoint &pt1, const CvPoint &pt2);
    CvPoint avgPoints(const Points &points, int num);
    void drawHands(const Points &hands, int num, IplImage* dst);
};

#endif
