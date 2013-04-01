
#ifndef _SKINDETECTOR_H_
#define _SKINDETECTOR_H_

#include <cv.h>
#include "skin-lookup-table.h"

class SkinDetector
{
public:
    SkinDetector(CvSize imgSz);

    IplImage* skinBinaryImg;
    void detectSkin(const IplImage* src, IplImage* dst);

    ~SkinDetector();

private:
    IplImage* m_yCbCrImg;
    void skinDetectKernel(const IplImage *src, IplImage* yCbCrImg, IplImage* dst);
};

#endif
