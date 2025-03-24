#include "ObjectTracker.h"

using namespace cv;
using namespace std;

ObjectTracker::ObjectTracker()
{
    // tracker = TrackerKCF::create();
    tracker = cv::legacy::TrackerMOSSE::create();
}

bool ObjectTracker::initialize(Mat &frame, Rect trackingBox)
{
    if (frame.empty() || trackingBox.width <= 0 || trackingBox.height <= 0) {
        return false;
    }

    if (trackingBox.x >= 0 && trackingBox.y >= 0 &&
        trackingBox.x + trackingBox.width <= frame.cols &&
        trackingBox.y + trackingBox.height <= frame.rows) {

        this->trackingBox = trackingBox;
        cv::Rect2d trackingBoxDouble(trackingBox);

        tracker->init(frame, trackingBoxDouble);
        return true;
    }
    
    return false;
}

bool ObjectTracker::update(Mat& frame) {
    if (frame.empty() || trackingBox.width <= 0 || trackingBox.height <= 0) {
        return false;
    }
    cv::Rect2d trackingBoxDouble(trackingBox);
    return tracker->update(frame, trackingBoxDouble);

}

void ObjectTracker::drawBoundingBox(Mat &frame)
{
    if (trackingBox.width > 0 && trackingBox.height > 0) {
        rectangle(frame, trackingBox, Scalar(0, 255, 0), 2);
    }
}