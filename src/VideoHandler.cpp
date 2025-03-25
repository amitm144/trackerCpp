#include "VideoHandler.h"
#include <stdexcept>

using namespace cv;
using namespace std;

VideoHandler::VideoHandler(const string& filename) {
    video.open(filename);
    if (!video.isOpened()) {
        throw runtime_error("Error opening video file");
    }
    frameWidth = video.get(CAP_PROP_FRAME_WIDTH);
    frameHeight = video.get(CAP_PROP_FRAME_HEIGHT);
    output.open("output.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 120, Size(frameWidth, frameHeight));
}

bool VideoHandler::getFrame(Mat& frame) {
    return video.read(frame);
}

void VideoHandler::writeFrame(const Mat& frame) {
    output.write(frame);
}
double VideoHandler::getFPS() {
    return video.get(CAP_PROP_FPS);
}


VideoHandler::~VideoHandler() {
    output.release();
    video.release();
}