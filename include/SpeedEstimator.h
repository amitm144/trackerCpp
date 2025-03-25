#ifndef SPEEDESTIMATOR_H
#define SPEEDESTIMATOR_H

#include <opencv2/opencv.hpp>
#include <vector>

class SpeedEstimator {
private:
    double fps;
    double metersPerPixel; // Scaling factor for speed calculation

public:
    SpeedEstimator(double frameRate, const std::vector<cv::Point2f>& roadPoints);
    double calculateSpeed(const cv::Rect& bbox);
};

#endif
