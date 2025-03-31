#ifndef OBJECTTRACKER_H
#define OBJECTTRACKER_H

#include <opencv2/opencv.hpp>
#include <map>
#include <string>
#include <vector>

class ObjectTracker
{
private:
    // Track multiple vehicles with their IDs
    struct TrackedObject
    {
        cv::Rect bbox;
        cv::Point2f position;
        int missingFrames;
        std::string classLabel;
    };
    
    std::map<int, TrackedObject> trackedObjects;
    int nextId;
    int frameCount;
    double iouThreshold;
    int maxMissingFrames;

public:
    ObjectTracker(double iouThreshold = 0.3, int maxMissingFrames = 5);
    void update(const std::vector<std::pair<cv::Rect, std::string>>& detections);
    void drawBoundingBoxes(cv::Mat &frame, bool showLabels = true);
    bool getObjectPosition(int id, cv::Point2f &position) const;
    const std::map<int, TrackedObject>& getAllObjects() const;
    int getFrameCount() const;
    void incrementFrameCount();
};

#endif