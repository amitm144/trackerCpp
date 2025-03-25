#ifndef ROADSELECTOR_H
#define ROADSELECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

class RoadSelector {
public:
    static std::vector<cv::Point2f> selectRoadPoints(const std::string& videoPath);
};

#endif
