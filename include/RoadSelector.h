#ifndef ROADSELECTOR_H
#define ROADSELECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

class RoadSelector {
public:
    // Only select two reference lines that are 15m apart
    static std::vector<cv::Point2f> selectReferenceLines(const std::string& videoPath);
};

#endif