
#include "../include/skin-detector.h"

SkinDetector::SkinDetector(CvSize imgSz) {
    skinBinaryImg = cvCreateImage(imgSz, IPL_DEPTH_8U, 1);
    m_yCbCrImg = cvCreateImage(imgSz, IPL_DEPTH_8U, 3);
}

void SkinDetector::detectSkin(const IplImage* src, IplImage* dst) {
    skinDetectKernel(src, m_yCbCrImg, dst);
    cvSmooth(dst, dst);
}

void SkinDetector::skinDetectKernel(const IplImage *src, IplImage* yCbCrImg, IplImage* dst) {
    /*
     * 	Skin Color Detection:
     *		Using the skin color ellipse model.
     *	reference form:
     *		J. Kovac, P. Peer and F. Solina,
     *		"Human Skin Color Clustering for Face Detection,би
     *		Proceedings of the IEEE Region 8 Computer as a Tool, 2003, Vol. 2, pp. 144-148
     *
     *	@brief	Skin color detection
     *	@param	src	input image buffer with YCbCr color space
     *	@ret	the result of skin color detection
     */

    int i;
    int width = src->width;
    int height = src->height;
    int imagesize = width * height;

    cvCvtColor(src, yCbCrImg, CV_BGR2YCrCb);

    unsigned char *p_data = (unsigned char*) yCbCrImg->imageData;
    unsigned char *p_out = (unsigned char*) dst->imageData;
    unsigned char Cr, Cb;

    int result;
    int nx, ny;

    for (i = 0; i < imagesize; ++i) {
        ++p_data;
        Cr = *p_data++;
        Cb = *p_data++;

        nx = NUMERATOR_X_RESULT[p_data[2]][p_data[1]];
        ny = NUMERATOR_Y_RESULT[p_data[1]][p_data[2]];
        result = (nx == -1 || ny == -1) ? 1004 : nx + ny;

        if (result <= 1000) // fixed-point
        {
            *p_out = 255;
        } else {
            *p_out = 0;
        }
        ++p_out;
    }
}

SkinDetector::~SkinDetector() {
    cvReleaseImage(&skinBinaryImg);
    cvReleaseImage(&m_yCbCrImg);
}
