#include "PerspectiveTransformer.h"

using namespace cv;

PerspectiveTransformer::PerspectiveTransformer(const std::vector<Point2f>& roadPoints) {
    std::vector<Point2f> dstPoints = { {0, 0}, {500, 0}, {0, 700}, {500, 700} };
    homographyMatrix = getPerspectiveTransform(roadPoints, dstPoints);
}

Mat PerspectiveTransformer::apply(const Mat& frame) {
    Mat transformedFrame;
    warpPerspective(frame, transformedFrame, homographyMatrix, Size(500, 700));
    return transformedFrame;
}
