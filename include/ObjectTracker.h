#ifndef OBJECTTRACKER_H
#define OBJECTTRACKER_H

#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/tracking/tracking_legacy.hpp>

class ObjectTracker
{
private:
    cv::Ptr<cv::legacy::TrackerMOSSE> tracker;
    cv::Rect2d trackingBox;

public:
    ObjectTracker();
    bool initialize(cv::Mat &frame, cv::Rect trackingBox);
    bool update(cv::Mat &frame);
    void drawBoundingBox(cv::Mat &frame);
};

#endif