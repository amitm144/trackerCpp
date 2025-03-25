#ifndef SPEEDESTIMATOR_H
#define SPEEDESTIMATOR_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <deque>

class SpeedEstimator {
private:
    double fps;
    double topMetersPerPixel;    // Conversion at top of the road
    double bottomMetersPerPixel; // Conversion at bottom of the road
    double lastMetersPerPixel;   // Last used conversion factor
    double lastFrameTime;        // Timestamp of last processed frame
    
    std::vector<cv::Point2f> roadPoints;      // The road boundary points
    std::vector<cv::Point2f> referencePoints; // Previous positions of tracked objects
    std::vector<cv::Point2f> lastPositions;   // Last positions of tracked objects
    
    // Calculate the relative vertical position within the road (0=top, 1=bottom)
    double calculateVerticalPositionRatio(const cv::Point2f& point);

public:
    SpeedEstimator(double frameRate, const std::vector<cv::Point2f>& roadPoints);
    double calculateSpeed(const cv::Rect& bbox);
};

#endif