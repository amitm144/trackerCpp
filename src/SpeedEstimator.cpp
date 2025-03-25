#include "SpeedEstimator.h"

using namespace cv;

SpeedEstimator::SpeedEstimator(double frameRate, const std::vector<Point2f>& roadPoints) {
    fps = frameRate;

    // Approximate the road's real-world width
    double roadWidthMeters = 3.5;  // Approximate single-lane width in meters
    double pixelWidth = norm(roadPoints[0] - roadPoints[1]); // Distance between two top points

    metersPerPixel = roadWidthMeters / pixelWidth;
}

double SpeedEstimator::calculateSpeed(const Rect& bbox) {
    double pixelDisplacement = bbox.width; // Movement in pixels
    double displacementMeters = pixelDisplacement * metersPerPixel; // Convert to meters

    double speed = (displacementMeters * fps) * 3.6; // Convert to km/h
    return speed;
}
