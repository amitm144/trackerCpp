#ifndef YOLODETECTOR_H
#define YOLODETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>

class YOLODetector {
private:
    cv::dnn::Net net;
    std::vector<std::string> classNames;
    
    void loadClassNames(const std::string& classFile);
    std::vector<std::pair<cv::Rect, std::string>> postprocess(const cv::Mat& frame, const std::vector<cv::Mat>& outputs);

public:
    YOLODetector(const std::string& modelConfig, const std::string& modelWeights, const std::string& classFile);
    std::vector<std::pair<cv::Rect, std::string>> detectVehicles(const cv::Mat& frame);
};

#endif
