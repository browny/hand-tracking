
#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include <string>
#include <vector>
#include <iostream>
#include "./opencv/core/core_c.h"
#include "./opencv/imgproc/imgproc_c.h"
#include "./opencv/highgui/highgui_c.h"
using namespace std;

enum VIDEO_TYPE {
	CAMMERA = 0,
	VIDEOFILE
};

class Capture {

public:

	Capture(int num, CvSize dispSize, CvSize resolutionSize);
	Capture(string fileName, CvSize dispSize, CvSize resolutionSize);

	int channelNum;
	vector<IplImage*> channelImgList;

	void captureNext();

	~Capture();

private:

	string m_fileName;
	vector<CvCapture*> m_channelList;

	CvSize m_resolutioSize;
	CvSize m_displaySize;

	void initChannelList(VIDEO_TYPE type);
	void initChannelImgList(CvSize sz);
	IplImage* getNextVideoFrame(CvCapture* pCapture);


};

#endif

