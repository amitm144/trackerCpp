#include "SpeedEstimator.h"
#include <iostream>

using namespace cv;
using namespace std;

SpeedEstimator::SpeedEstimator(double frameRate, const vector<Point2f>& referenceLines) {
    fps = frameRate;
    referenceDistance = 21.0; // 21 meters between lines
    this->referenceLines = referenceLines;

}

bool SpeedEstimator::hasVehicleCrossedLine(const Point2f& pos, const Point2f& linePoint, bool isVertical) {
    return (pos.y - linePoint.y) * (vehicles[0].lastPosition.y - linePoint.y) <= 0;
}

void SpeedEstimator::processVehicle(int id, const Point2f& position, int currentFrame) {
    // Create vehicle entry if it doesn't exist
    if (vehicles.find(id) == vehicles.end()) {
        VehicleData data;
        data.crossedFirstLine = false;
        data.crossedSecondLine = false;
        data.frameAtFirstLine = 0;
        data.frameAtSecondLine = 0;
        data.speed = 0.0;
        data.lastPosition = position; // Initialize last position
        vehicles[id] = data;
        return; // Skip first frame for this vehicle
    }
    
    // Get reference to vehicle data
    VehicleData &vehicle = vehicles[id];
    
    // Check first line crossing
    if (!vehicle.crossedFirstLine && 
        (position.y - referenceLines[0].y) * (vehicle.lastPosition.y - referenceLines[0].y) <= 0) {
        vehicle.crossedFirstLine = true;
        vehicle.frameAtFirstLine = currentFrame;
    }
    
    // Check second line crossing
    if (!vehicle.crossedSecondLine && 
        (position.y - referenceLines[1].y) * (vehicle.lastPosition.y - referenceLines[1].y) <= 0) {
        vehicle.crossedSecondLine = true;
        vehicle.frameAtSecondLine = currentFrame;
    }
    
    // Calculate speed if we have crossings for both lines
    if (vehicle.crossedFirstLine && vehicle.crossedSecondLine && vehicle.speed == 0.0) {
        int framesElapsed = abs(vehicle.frameAtSecondLine - vehicle.frameAtFirstLine);
        
        if (framesElapsed > 0) {
            double timeSeconds = framesElapsed / fps;
            vehicle.speed = (referenceDistance / timeSeconds) * 3.6;
        }
    }

    vehicle.lastPosition = position;
}


double SpeedEstimator::getSpeed(int id) const {
    auto it = vehicles.find(id);
    if (it != vehicles.end() && it->second.crossedSecondLine) {
        return it->second.speed;
    }
    return 0.0;
}

bool SpeedEstimator::hasSpeed(int id) const {
    auto it = vehicles.find(id);
    return (it != vehicles.end() && it->second.crossedSecondLine);
}