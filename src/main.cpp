
#include <string>
#include "../include/capture.h"
#include "../include/skin-detector.h"
#include "../include/tracker.h"
using namespace std;

int main(int argc, char* argv[]) {

    // Read file
    //Capture capture("test.wmv", windowSize, windowSize);

    // Read camera
    CvSize dispSize = cvSize(640, 480);
    CvSize resolutionSize = dispSize;
    Capture capture(1, dispSize, resolutionSize);

    SkinDetector skinDetector(dispSize);
    Tracker tracker(dispSize);

    IplImage* frame;

    while (1) {
        capture.captureNext();
        frame = capture.channelImgList[0];

        // Skin detection
        skinDetector.detectSkin(frame, skinDetector.skinBinaryImg);

        // Hand tracking
        tracker.track(frame, skinDetector.skinBinaryImg, tracker.currentHands);

        int c = cvWaitKey(30);
        if ( (char) c == 27 ) // 'Esc' to terminate
            break;

        if (c == '1' || c == '!')
            tracker.setTrackNum(1);
        if (c == '2' || c == '@')
            tracker.setTrackNum(2);
        if (c == '3' || c == '#')
            tracker.setTrackNum(3);
    }

    return 0;
}

