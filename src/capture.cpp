
#include "../include/capture.h"

Capture::Capture(int num, CvSize dispSize, CvSize resolutionSize) {
    channelNum = num;
    m_fileName = "";
    m_resolutioSize = resolutionSize;

    m_channelList.resize(channelNum);
    channelImgList.resize(channelNum);

    initChannelList(CAMMERA);
    initChannelImgList(dispSize);
}

Capture::Capture(string fileName, CvSize dispSize, CvSize resolutionSize) {
    channelNum = 1;
    m_fileName = fileName;
    m_resolutioSize = resolutionSize;

    m_channelList.resize(channelNum);
    channelImgList.resize(channelNum);

    initChannelList(VIDEOFILE);
    initChannelImgList(dispSize);
}

void Capture::captureNext() {
    for (int i = 0; i < channelNum; ++i) {
        IplImage* nextFrame = getNextVideoFrame(m_channelList[i]);
        IplImage* channelFrame = channelImgList[i];
        cvResize(nextFrame, channelFrame);
    }
}

void Capture::initChannelList(VIDEO_TYPE type) {
    if (type == CAMMERA) {
        for (int i = 0; i < channelNum; ++i) {
            m_channelList[i] = cvCreateCameraCapture(i);

            //set resolution
            cvSetCaptureProperty(m_channelList[i], CV_CAP_PROP_FRAME_WIDTH, m_resolutioSize.width);
            cvSetCaptureProperty(m_channelList[i], CV_CAP_PROP_FRAME_HEIGHT, m_resolutioSize.height);

            if ( !(m_channelList[i]) ) {
                cout << "failed to initialize video capture" << endl;
                exit(EXIT_FAILURE);

            }
        }
    } else if (type == VIDEOFILE) {
        const char* fileNameChar = m_fileName.c_str();
        m_channelList[0] = cvCreateFileCapture(fileNameChar);

        if ( !(m_channelList[0]) ) {
            cout << "failed to initialize video capture" << endl;
            exit(EXIT_FAILURE);
        }
    }
}

void Capture::initChannelImgList(CvSize sz) {
    for (int i = 0; i < channelNum; ++i)
        channelImgList[i] = cvCreateImage(sz, 8, 3);
}

IplImage* Capture::getNextVideoFrame(CvCapture* pCapture) {
    IplImage* nextFrame = cvQueryFrame(pCapture);
    if (!nextFrame) {
        cout << "failed to get a video frame" << endl;
        exit(EXIT_FAILURE);
    }
    return nextFrame;
}

Capture::~Capture() {
    for (int i = 0; i < channelNum; ++i) {
        cvReleaseImage( &(channelImgList[i]) );
        cvReleaseCapture(&m_channelList[i]);
    }
}
