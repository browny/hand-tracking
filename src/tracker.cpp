
#include "../include/tracker.h"

Tracker::Tracker(CvSize imgSz) :
    kCcNum(10),
    kCcAreaRemoveTh(1500),
    kHandBufferLength(12),
    kClusteredCenterBufferLength(12),
    kMotionRatio(0.4),
    kDarkAvgGrayTh(50.0),
    kCcSizeTh(500.0),
    kClusteredCenterMergeTh(50.0) 
{
    m_trackNum = 1;
    m_adptDiffTh = 0.0;
    m_adptResampleMargin = 15;
    m_adptCcRemoveAreaTh = kCcAreaRemoveTh;
    m_adptClusteredCenterMergeTh = kClusteredCenterMergeTh;
    m_adptClusterCheckLength = 6;
    m_adptHandBufferAvgLength = 6;

    currentHands.resize(m_trackNum);
    m_clusteredCenterBuffers =
        vector<Points>(m_trackNum, Points(kClusteredCenterBufferLength, cvPoint(-1, -1)));
    m_handBuffers = vector<Points>(m_trackNum, Points(kHandBufferLength, cvPoint(-1, -1)));

    m_src              = cvCreateImage(imgSz, IPL_DEPTH_8U, 3);
    m_srcGray          = cvCreateImage(imgSz, IPL_DEPTH_8U, 1);
    m_srcGrayOld       = cvCreateImage(imgSz, IPL_DEPTH_8U, 1);
    m_diffImg          = cvCreateImage(imgSz, IPL_DEPTH_8U, 1);
    m_motionSkinImg    = cvCreateImage(imgSz, IPL_DEPTH_8U, 1);
    m_resampledSkinImg = cvCreateImage(imgSz, IPL_DEPTH_8U, 1);

    m_cluster = Cluster();
}

void Tracker::setTrackNum(int num) {
    m_trackNum = num;
    currentHands.resize(m_trackNum);
    m_clusteredCenterBuffers =
        vector<Points> (m_trackNum, Points(kClusteredCenterBufferLength, cvPoint(-1, -1)));
    m_handBuffers = vector<Points> (m_trackNum, Points(kHandBufferLength, cvPoint(-1, -1)));

    cout << "Tracking Num: " << m_trackNum << endl;
}

void Tracker::track(const IplImage* src, const IplImage* skinImg, Points &hands) {
    cvCopy(src, m_src);

    // Tracking the motion & skin color part
    getMovingSkinImg(src, skinImg);

    int num = kCcNum;
    Rects rects(0);
    Points centers(0);

    // Use connected compont to filter out hand candidates
    getCandidates(skinImg, m_motionSkinImg, &num, rects, centers);

    // Clustering these candidates (multiple hands tracking)
    clusterCandidates(centers);

    // Get the final hand location
    getHands();

    drawHands(currentHands, m_trackNum, m_src);
    cvShowImage("hand", m_src);

    cvCopy(m_srcGray, m_srcGrayOld);
}

void Tracker::getMovingSkinImg(const IplImage* src, const IplImage* skinImg) {
    cvCvtColor(src, m_srcGray, CV_BGR2GRAY);

    getDiffImg(m_srcGray, m_srcGrayOld, m_diffImg);
    cvAnd(m_diffImg, skinImg, m_motionSkinImg, 0);
}

void Tracker::getCandidates(const IplImage* skinImg, IplImage* motionSkinImg,
        int* num, Rects &rects, Points &centers) {
    connectComponent(motionSkinImg, 1, kCcSizeTh*10, num, rects, centers);

    // Use the first connected component result to resample skin image
    resampleByPoints(skinImg, m_adptResampleMargin, centers, m_resampledSkinImg);
    cvShowImage("resampled skin", m_resampledSkinImg);

    // Use the resampled skin image to do connected component again
    *num = kCcNum;
    rects.clear();
    centers.clear();

    connectComponent(m_resampledSkinImg, 1, kCcSizeTh, num, rects, centers);

    // Remove small connected components
    removeSmallCcs(num, rects, centers);
}

void Tracker::clusterCandidates(const Points &centers) {
    for (int i = 0; i < m_trackNum; ++i)
        shiftVector<CvPoint>( m_clusteredCenterBuffers[i], cvPoint(-1, -1) );

    if (centers.size() > 0) {
        Points clusteredCenters;
        vector<Points> clusteredPts;

        int clusterNum = m_cluster.cluster(centers, m_trackNum,
                m_adptClusteredCenterMergeTh, clusteredPts, clusteredCenters);

        // Check clusteredCenterBuffers conditon
        vector<bool> empty(m_trackNum, true);
        bool isAllBufferEmpty = getEmptyIndex(m_clusteredCenterBuffers, empty);

        if (isAllBufferEmpty) {
            // Put the clusteredCenters into the corresponding clusteredCenterBuffers
            for (int i = 0; i < clusterNum; ++i)
                m_clusteredCenterBuffers[i][0] = clusteredCenters[i];
        } else {
            for (int i = 0; i < clusterNum; ++i) {
                CvPoint center = clusteredCenters[i];

                // Find the center belongs to which centerBuffer
                int idx = getClosestCenterBufIndex(center, m_clusteredCenterBuffers, empty);

                if (idx != -1) {
                    // The center belong to 'idx' cluster
                    m_clusteredCenterBuffers[idx][0] = center;
                } else {
                    // The center doesn't belong to any non-empty centerBuffer.
                    // If there were any empty buffer, it will be first member of the new cluster
                    for (int j = 0; j < m_trackNum; ++j) {
                        if (empty[j] == true) {
                            m_clusteredCenterBuffers[j][0] = center;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void Tracker::getHands() {
    for (int i = 0; i < m_trackNum; ++i) {
        shiftVector<CvPoint> (m_handBuffers[i], m_clusteredCenterBuffers[i][0]);

        // Average the hand buffer as the final detected hand
        currentHands[i] = avgPoints(m_handBuffers[i], m_adptHandBufferAvgLength);
    }
}

// Below are helper methods
double Tracker::avgGrayValue(const IplImage* src) {
    CvScalar avgGrayValue = cvAvg(src, 0);
    return avgGrayValue.val[0];
}

void Tracker::getDiffImg(const IplImage* img1, const IplImage* img2, IplImage* diff) {
    cvAbsDiff(img1, img2, diff);
    CvScalar avgGrayValue = cvAvg(img1);

    if (avgGrayValue.val[0] < kDarkAvgGrayTh)
        m_adptDiffTh = 40;
    else
        m_adptDiffTh = kMotionRatio * avgGrayValue.val[0];

    cvThreshold(diff, diff, m_adptDiffTh, 255, CV_THRESH_BINARY);
}

void Tracker::connectComponent(IplImage* src, const int poly_hull0,
        const float perimScale, int* num, Rects &rects, Points &centers) {

    CvMemStorage* mem_storage = NULL;
    CvSeq* contours = NULL;

    // Clean up
    cvMorphologyEx(src, src, 0, 0, CV_MOP_OPEN, 1);
    cvMorphologyEx(src, src, 0, 0, CV_MOP_CLOSE, 1);

    // Find contours around only bigger regions
    mem_storage = cvCreateMemStorage(0);

    CvContourScanner scanner = cvStartFindContours(src, mem_storage,
            sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    CvSeq* c;
    int numCont = 0;
    while ((c = cvFindNextContour(scanner)) != NULL) {
        double len = cvContourPerimeter(c);

        // calculate perimeter len threshold
        double q = (double) (src->height + src->width) / perimScale;

        // get rid of blob if its perimeter is too small
        if (len < q) {
            cvSubstituteContour(scanner, NULL);
        } else {
            // smooth its edge if its large enough
            CvSeq* c_new;
            if (poly_hull0) {
                // polygonal approximation
                c_new = cvApproxPoly(c, sizeof(CvContour), mem_storage, CV_POLY_APPROX_DP, 2, 0);
            } else {
                // convex hull of the segmentation
                c_new = cvConvexHull2(c, mem_storage, CV_CLOCKWISE, 1);
            }

            cvSubstituteContour(scanner, c_new);
            numCont++;
        }
    }

    contours = cvEndFindContours(&scanner);

    // Calc center of mass and/or bounding rectangles
    if (num != NULL) {
        // user wants to collect statistics
        int numFilled = 0, i = 0;

        for (i = 0, c = contours; c != NULL; c = c->h_next, i++) {
            if (i < *num) {
                // bounding retangles around blobs
                rects.push_back(cvBoundingRect(c));

                CvPoint center = cvPoint(rects[i].x + rects[i].width / 2, rects[i].y
                        + rects[i].height / 2);
                centers.push_back(center);

                numFilled++;
            }
        }

        *num = numFilled;
    }

    cvReleaseMemStorage(&mem_storage);
}

void Tracker::removeSmallCcs(int* ccNum, Rects &rects, Points &centers) {
    Rects::iterator it = rects.begin();
    Points::iterator it2 = centers.begin();

    while (it != rects.end()) {
        int area = it->width * it->height;
        if ( area < m_adptCcRemoveAreaTh ) {
            rects.erase(it);
            centers.erase(it2);
            (*ccNum)--;

            it  = rects.begin();
            it2 = centers.begin();
        } else {
            it++;
            it2++;
        }
    }
}

void Tracker::resampleByPoints(const IplImage* input, const int srMar,
        const Points &points, IplImage* output) {
    cvSetZero(output); // clear output image
    CvSize sz = cvGetSize(output);

    for (unsigned int i = 0; i < points.size(); ++i) {
        CvPoint leftTop     = cvPoint(points[i].x - srMar, points[i].y - srMar);
        CvPoint rightBottom = cvPoint(points[i].x + srMar, points[i].y + srMar);

        if (leftTop.x < 0)
            leftTop.x = 0;

        if (leftTop.y < 0)
            leftTop.y = 0;

        if (rightBottom.x > sz.width)
            rightBottom.x = sz.width;

        if (rightBottom.y > sz.height)
            rightBottom.y = sz.height;

        for (int j = leftTop.y; j < rightBottom.y; j++) {
            uchar* ptr = (uchar*)(input->imageData + j*input->widthStep);
            uchar* ptrNew = (uchar*)(output->imageData + j*output->widthStep);
            for (int k = leftTop.x; k < rightBottom.x; k++)
                ptrNew[k] = ptr[k];
        }
    }
}

void Tracker::drawCcs(const int num, const Rects &rects,
        const Points &centers, IplImage* dst) {
    if (num != 0) {
        for (int i = 0; i < num; ++i) {
            cvCircle(dst, centers[i], 5, CV_RGB(0, 255, 255), -1);
            cvRectangle(dst, cvPoint(rects[i].x, rects[i].y),
                    cvPoint(rects[i].x + rects[i].width, rects[i].y + rects[i].height),
                    CV_RGB(255, 255, 0), 1);
        }
    }
}

template<class myType>
void Tracker::shiftVector(vector<myType> &vec, myType element) {
    for (int i = (vec.size() - 2); i >= 0; i--)
        vec[i + 1] = vec[i];

    vec[0] = element;
}

bool Tracker::getEmptyIndex(const vector<Points> &centerBufs, vector<bool> &empty) {
    // Judge each centerBufs is empty or not
    for (int i = 0; i < m_trackNum; ++i) {
        empty[i] = true;

        for (int j = 0; j < m_trackNum; ++j)
            if (centerBufs[i][j].x != -1)
                empty[i] = false;
    }

    bool isAllBufferEmpty = true;
    for (int i = 0; i < m_trackNum; ++i)
        if (empty[i] == false)
            isAllBufferEmpty = false;

    return isAllBufferEmpty;
}

int Tracker::getClosestCenterBufIndex(CvPoint center, const vector<Points> &buf,
        const vector<bool> &empty) {
    double minDis = 999999999;
    int bufIndex = -1;

    for (int j = 0; j < m_trackNum; ++j) {
        if (!empty[j]) {
            for (int k = 1; k < (1 + m_adptClusterCheckLength); ++k) {
                if ( buf[j][k].x != -1 ) {
                    double dist = distance(center, buf[j][k]);

                    if ( (dist < m_adptClusteredCenterMergeTh) && (dist < minDis) ) {
                        minDis = dist;
                        bufIndex = j;
                    }
                }
            }
        }
    }

    return bufIndex;
}

double Tracker::distance(const CvPoint &pt1, const CvPoint &pt2) {
    return sqrt((double)((pt1.x - pt2.x)*(pt1.x - pt2.x) +
                (pt1.y - pt2.y)*(pt1.y - pt2.y)));
}

CvPoint Tracker::avgPoints(const Points &points, int num) {
    // Calculate the average of the points vector excluding (-1, -1)
    int goodCenterNum = 0;
    int accX = 0, accY = 0;

    for (int i = 0; i < num; i++) {
        if ((points[i].x != -1) && (points[i].y != -1)) {
            accX += points[i].x;
            accY += points[i].y;
            goodCenterNum++;
        }
    }

    if (goodCenterNum > 0) {
        int bufCenter_x = accX / goodCenterNum;
        int bufCenter_y = accY / goodCenterNum;

        return cvPoint(bufCenter_x, bufCenter_y);
    } else {
        // if all points in buffer are (-1, -1), the hand point will disappear
        return cvPoint(-1, -1);
    }
}

void Tracker::drawHands(const Points &hands, int num, IplImage* dst) {
    CvScalar ColorBox[4] = { CV_RGB(255,0,0), CV_RGB(0,255,0),
        CV_RGB(0,0,255), CV_RGB(255,0,255) };

    const int kRecSize = 15;
    CvSize sz = cvGetSize(dst);

    for (int i = 0; i < num; ++i) {
        if (hands[i].x != -1) {

            cvCircle(dst, hands[i], 5, ColorBox[i], -2);

            int x1 = 0;
            int y1 = 0;
            int x2 = 0;
            int y2 = 0;

            if ((hands[i].x - kRecSize) < 0)
                x1 = 0;
            else
                x1 = hands[i].x - kRecSize;

            if ((hands[i].y - kRecSize) < 0)
                y1 = 0;
            else
                y1 = hands[i].y - kRecSize;

            if ((hands[i].x + kRecSize) >= sz.width)
                x2 = sz.width - 1;
            else
                x2 = hands[i].x + kRecSize;

            if ((hands[i].y + kRecSize) >= sz.height)
                y2 = sz.height - 1;
            else
                y2 = hands[i].y + kRecSize;

            CvPoint leftTop = cvPoint(x1, y1);
            CvPoint rightBot = cvPoint(x2, y2);
            cvRectangle(dst, leftTop, rightBot, ColorBox[i], 2);

            //cout << "x: " << hands[i].x << " y: " << hands[i].y << endl;
        } else {
            ;
        }
    }
}

Tracker::~Tracker() {
    cvReleaseImage(&m_src);
    cvReleaseImage(&m_srcGray);
    cvReleaseImage(&m_srcGrayOld);
    cvReleaseImage(&m_diffImg);
    cvReleaseImage(&m_motionSkinImg);
    cvReleaseImage(&m_resampledSkinImg);
}
