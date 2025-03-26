#include "VideoHandler.h"
#include <stdexcept>

using namespace cv;
using namespace std;

VideoHandler::VideoHandler(const string& filename) {
    video.open(filename);
    if (!video.isOpened()) {
        throw runtime_error("Error opening video file");
    }
    
    // Get video properties
    frameWidth = video.get(CAP_PROP_FRAME_WIDTH);
    frameHeight = video.get(CAP_PROP_FRAME_HEIGHT);
    double fps = video.get(CAP_PROP_FPS);
    
    // Open output video with the same frame rate as input
    output.open("output.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(frameWidth, frameHeight));
    
    if (!output.isOpened()) {
        throw runtime_error("Error creating output video file");
    }
}

bool VideoHandler::getFrame(Mat& frame) {
    return video.read(frame);
}

void VideoHandler::writeFrame(const Mat& frame) {
    if (!frame.empty()) {
        output.write(frame);
    }
}

double VideoHandler::getFPS() {
    return video.get(CAP_PROP_FPS);
}

VideoHandler::~VideoHandler() {
    output.release();
    video.release();
}