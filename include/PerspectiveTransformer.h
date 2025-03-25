#ifndef PERSPECTIVETRANSFORMER_H
#define PERSPECTIVETRANSFORMER_H

#include <opencv2/opencv.hpp>
#include <vector>

class PerspectiveTransformer {
private:
    cv::Mat homographyMatrix;

public:
    PerspectiveTransformer(const std::vector<cv::Point2f>& roadPoints);
    cv::Mat apply(const cv::Mat& frame);
};

#endif
