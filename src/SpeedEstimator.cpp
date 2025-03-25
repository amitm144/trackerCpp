#include "SpeedEstimator.h"
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

SpeedEstimator::SpeedEstimator(double frameRate, const std::vector<Point2f>& roadPoints) {
    fps = frameRate;
    
    // Store the road points for reference
    if (roadPoints.size() == 4) {
        this->roadPoints = roadPoints;
    } else {
        cerr << "Warning: Invalid road points. Using defaults." << endl;
        // Default road points if none provided
        this->roadPoints = {
            Point2f(100, 100), Point2f(500, 100),
            Point2f(500, 400), Point2f(100, 400)
        };
    }
    
    // Calculate real-world distances
    // Assuming standard lane width of 3.5 meters for top of quadrilateral
    double roadWidthMeters = 3.5;
    
    // Calculate the width of the top of the road in pixels
    double topWidthPixels = norm(roadPoints[0] - roadPoints[1]);
    
    // Calculate scaling factor for top of the road
    topMetersPerPixel = roadWidthMeters / topWidthPixels;
    
    // Calculate the width of the bottom of the road in pixels
    double bottomWidthPixels = norm(roadPoints[3] - roadPoints[2]);
    
    // Assuming bottom width represents same 3.5m lane
    bottomMetersPerPixel = roadWidthMeters / bottomWidthPixels;
    
    // Initialize tracking parameters
    lastFrameTime = 0.0;
    referencePoints = {};
    
    cout << "Speed Estimator initialized:" << endl;
    cout << "  - Frame rate: " << fps << " fps" << endl;
    cout << "  - Top meters per pixel: " << topMetersPerPixel << endl;
    cout << "  - Bottom meters per pixel: " << bottomMetersPerPixel << endl;
}

double SpeedEstimator::calculateSpeed(const Rect& bbox) {
    Point2f center(bbox.x + bbox.width/2.0f, bbox.y + bbox.height/2.0f);
    double currentTime = static_cast<double>(getTickCount()) / getTickFrequency();
    
    // Calculate the vertical position ratio (0 at top, 1 at bottom of road)
    double yRatio = calculateVerticalPositionRatio(center);
    
    // Interpolate meters per pixel based on vertical position
    double currentMetersPerPixel = topMetersPerPixel + 
                                   yRatio * (bottomMetersPerPixel - topMetersPerPixel);
    
    // If this is a new vehicle, just store its position and return a default speed
    if (referencePoints.empty() || lastFrameTime == 0.0) {
        referencePoints.push_back(center);
        lastPositions.push_back(center);
        lastFrameTime = currentTime;
        lastMetersPerPixel = currentMetersPerPixel;
        return 0.0; // No speed for first frame
    }
    
    // Find the closest reference point
    int closestIdx = -1;
    double minDist = 100.0; // Maximum pixel distance to match
    
    for (size_t i = 0; i < referencePoints.size(); i++) {
        double dist = norm(center - referencePoints[i]);
        if (dist < minDist) {
            minDist = dist;
            closestIdx = i;
        }
    }
    
    double speed = 0.0;
    
    if (closestIdx >= 0) {
        // Update the reference point
        Point2f prevPos = lastPositions[closestIdx];
        lastPositions[closestIdx] = center;
        referencePoints[closestIdx] = center;
        
        // Calculate time difference in seconds
        double timeDiff = currentTime - lastFrameTime;
        
        if (timeDiff > 0.001) { // Ensure we don't divide by very small values
            // Calculate real-world distance moved in meters
            double pixelDistance = norm(center - prevPos);
            double distanceMeters = pixelDistance * currentMetersPerPixel;
            
            // Calculate speed in km/h
            speed = (distanceMeters / timeDiff) * 3.6; // m/s to km/h
            
            // Apply some constraints for realism
            speed = std::max(0.0, speed);
            speed = std::min(200.0, speed); // Cap at reasonable maximum
        }
    } else {
        // This is a new vehicle
        referencePoints.push_back(center);
        lastPositions.push_back(center);
    }
    
    // Update the last frame time
    lastFrameTime = currentTime;
    lastMetersPerPixel = currentMetersPerPixel;
    
    return speed;
}

double SpeedEstimator::calculateVerticalPositionRatio(const Point2f& point) {
    // This function calculates where a point falls vertically within the road quadrilateral
    // Returns a value from 0 (at top of road) to 1 (at bottom of road)
    
    if (roadPoints.size() != 4) return 0.5; // Default to middle if no valid road points
    
    // Get the top and bottom y-coordinates of the road
    double topY = (roadPoints[0].y + roadPoints[1].y) / 2.0;
    double bottomY = (roadPoints[2].y + roadPoints[3].y) / 2.0;
    
    // Calculate the ratio
    double ratio = (point.y - topY) / (bottomY - topY);
    
    // Constrain to the range [0, 1]
    return std::max(0.0, std::min(1.0, ratio));
}