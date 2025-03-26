#ifndef SPEEDESTIMATOR_H
#define SPEEDESTIMATOR_H

#include <opencv2/opencv.hpp>
#include <map>

class SpeedEstimator {
private:
    double fps;
    double referenceDistance; // 21 meters
    std::vector<cv::Point2f> referenceLines;
    
    struct VehicleData {
        bool crossedFirstLine;
        int frameAtFirstLine;
        bool crossedSecondLine;
        int frameAtSecondLine;
        double speed;
        cv::Point2f lastPosition;
    };
    
    std::map<int, VehicleData> vehicles;

public:
    SpeedEstimator(double frameRate, const std::vector<cv::Point2f>& referenceLines);
    void processVehicle(int id, const cv::Point2f& position, int currentFrame);
    double getSpeed(int id) const;
    bool hasSpeed(int id) const;
};

#endif