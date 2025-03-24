#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H

#include <opencv2/opencv.hpp>
#include <string>

class VideoHandler {
private:
    cv::VideoCapture video;
    cv::VideoWriter output;
    int frameWidth, frameHeight;

public:
    VideoHandler(const std::string& filename);
    bool getFrame(cv::Mat& frame);
    void writeFrame(const cv::Mat& frame);
    ~VideoHandler();
};

#endif