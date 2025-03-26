#include "ObjectTracker.h"

using namespace cv;
using namespace std;

ObjectTracker::ObjectTracker()
{
    tracker = cv::legacy::TrackerMOSSE::create();
}

bool ObjectTracker::initialize(Mat &frame, Rect bbox)
{
    if (frame.empty() || bbox.width <= 0 || bbox.height <= 0 ||
        bbox.x < 0 || bbox.y < 0 ||
        bbox.x + bbox.width > frame.cols ||
        bbox.y + bbox.height > frame.rows) {
        return false;
    }

    trackingBox = bbox;
    return tracker->init(frame, trackingBox);
}

bool ObjectTracker::update(Mat& frame) {
    if (frame.empty()) {
        return false;
    }
    return tracker->update(frame, trackingBox);
}

void ObjectTracker::drawBoundingBox(Mat &frame)
{
    if (trackingBox.width > 0 && trackingBox.height > 0) {
        rectangle(frame, trackingBox, Scalar(0, 255, 0), 2);
    }
}