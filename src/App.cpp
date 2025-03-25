#include "App.h"
#include <iostream>
#include <map>
#include <chrono>

using namespace cv;
using namespace std;
using namespace std::chrono;

void App::run(const string &filename, const string &modelConfig,
              const string &modelWeights, const string &classFile,
              const vector<Point2f>& roadPoints)
{
    try
    {
        // Initialize video handling
        VideoHandler videoHandler(filename);
        double frameRate = videoHandler.getFPS();
        cout << "Video frame rate: " << frameRate << " fps" << endl;
        
        // Detection on every frame, but speed calculation every 10 frames
        const int speedCalculationInterval = 10;
        
        YOLODetector detector(modelConfig, modelWeights, classFile);
        
        // Display the road quadrilateral once at startup
        Mat firstFrame;
        if (videoHandler.getFrame(firstFrame) && !firstFrame.empty()) {
            Mat roadDisplay = firstFrame.clone();
            
            // Draw the road area
            vector<Point> roadPoly;
            for (const auto& pt : roadPoints) {
                roadPoly.push_back(Point(pt.x, pt.y));
            }
            
            // Fill the road area with semi-transparent overlay
            Mat overlay = roadDisplay.clone();
            fillPoly(overlay, vector<vector<Point>>{roadPoly}, Scalar(0, 200, 0, 128));
            addWeighted(overlay, 0.3, roadDisplay, 0.7, 0, roadDisplay);
            
            // Draw the points and lines
            for (size_t i = 0; i < roadPoints.size(); i++) {
                circle(roadDisplay, roadPoints[i], 5, Scalar(0, 0, 255), -1);
                line(roadDisplay, 
                     roadPoints[i], 
                     roadPoints[(i+1) % roadPoints.size()], 
                     Scalar(0, 255, 0), 2);
                
                // Add labels
                string label = "P" + to_string(i+1);
                putText(roadDisplay, label, 
                        roadPoints[i] + Point2f(10, 10), 
                        FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
            }
            
            // Display road calibration
            putText(roadDisplay, "Road Calibration Points", 
                    Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 2);
            putText(roadDisplay, "Press any key to continue", 
                    Point(10, 60), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);
            
            imshow("Road Calibration", roadDisplay);
            waitKey(0);
            destroyWindow("Road Calibration");
            
            // Reset the video to start
            videoHandler = VideoHandler(filename);
        }
        
        // Create a structure to hold vehicle data
        struct VehicleData {
            Rect bbox;
            double speed;
            Point2f position;
            high_resolution_clock::time_point lastSpeedUpdateTime;
            Point2f speedReferencePosition;
            int framesSinceLastSpeedCalculation;
            bool hasValidSpeed;
        };
        
        map<int, VehicleData> trackedVehicles; // ID -> vehicle data
        int nextVehicleId = 0;
        
        Mat frame;
        int frameCount = 0;
        high_resolution_clock::time_point processingStart = high_resolution_clock::now();
        
        while (videoHandler.getFrame(frame))
        {
            if (frame.empty()) continue;
            frameCount++;
            
            high_resolution_clock::time_point currentFrameTime = high_resolution_clock::now();
            
            // Detect vehicles in every frame for smooth tracking
            vector<pair<Rect, string>> detectedObjects = detector.detectVehicles(frame);
            
            // Update tracking with the new detections
            map<int, bool> vehicleUpdated;
            
            for (const auto& detection : detectedObjects) {
                Rect box = detection.first;
                string className = detection.second;
                
                // Find best match based on IOU
                int bestMatchId = -1;
                double bestIOU = 0.3; // IOU threshold
                
                for (const auto& [id, vehicle] : trackedVehicles) {
                    // Skip already updated vehicles
                    if (vehicleUpdated.find(id) != vehicleUpdated.end()) continue;
                    
                    // Calculate IOU
                    Rect trackedBox = vehicle.bbox;
                    Rect intersection = box & trackedBox;
                    if (intersection.empty()) continue;
                    
                    double intersectArea = intersection.area();
                    double unionArea = box.area() + trackedBox.area() - intersectArea;
                    double iou = intersectArea / unionArea;
                    
                    if (iou > bestIOU) {
                        bestIOU = iou;
                        bestMatchId = id;
                    }
                }
                
                // Calculate center position of the bounding box
                Point2f center(box.x + box.width/2.0, box.y + box.height/2.0);
                
                // Update existing vehicle or create new one
                if (bestMatchId >= 0) {
                    VehicleData& vehicle = trackedVehicles[bestMatchId];
                    
                    // Update vehicle position for tracking
                    vehicle.bbox = box;
                    vehicle.position = center;
                    
                    // Only calculate speed every speedCalculationInterval frames
                    vehicle.framesSinceLastSpeedCalculation++;
                    
                    if (vehicle.framesSinceLastSpeedCalculation >= speedCalculationInterval) {
                        // Reset the counter
                        vehicle.framesSinceLastSpeedCalculation = 0;
                        
                        // Calculate time difference since last speed update
                        if (vehicle.lastSpeedUpdateTime.time_since_epoch().count() > 0) {
                            auto timeDiff = duration_cast<duration<double>>(
                                currentFrameTime - vehicle.lastSpeedUpdateTime);
                            double secondsElapsed = timeDiff.count();
                            
                            if (secondsElapsed > 0.1) { // Ensure enough time has passed
                                // Calculate displacement in pixels
                                double pixelDistance = norm(center - vehicle.speedReferencePosition);
                                
                                // Get road position for scaling (0 = top, 1 = bottom)
                                double roadHeight = norm(roadPoints[0] - roadPoints[3]);
                                double yPosition = (center.y - roadPoints[0].y) / roadHeight;
                                yPosition = max(0.0, min(1.0, yPosition));
                                
                                // Calculate meters per pixel (adjust for perspective)
                                double topWidth = norm(roadPoints[1] - roadPoints[0]);
                                double bottomWidth = norm(roadPoints[2] - roadPoints[3]);
                                double currentWidth = topWidth * (1 - yPosition) + bottomWidth * yPosition;
                                
                                // Convert using road width
                                double roadWidthMeters = 3.5; // Standard lane width
                                double metersPerPixel = roadWidthMeters / currentWidth;
                                
                                // Convert pixels to meters
                                double distanceMeters = pixelDistance * metersPerPixel;
                                
                                // Calculate speed in km/h with actual time difference
                                double speedKmh = (distanceMeters / secondsElapsed) * 3.6;
                                
                                // Apply a scaling factor - adjust as needed based on ground truth
                                speedKmh *= 15.0;
                                
                                // Apply smoothing with previous speed
                                if (vehicle.hasValidSpeed) {
                                    vehicle.speed = 0.3 * vehicle.speed + 0.7 * speedKmh;
                                } else {
                                    vehicle.speed = speedKmh;
                                    vehicle.hasValidSpeed = true;
                                }
                                
                                // Apply limits for realism
                                vehicle.speed = max(0.0, min(180.0, vehicle.speed));
                            }
                        }
                        
                        // Update reference position and time for next speed calculation
                        vehicle.speedReferencePosition = center;
                        vehicle.lastSpeedUpdateTime = currentFrameTime;
                    }
                    
                    vehicleUpdated[bestMatchId] = true;
                } else {
                    // This is a new vehicle
                    VehicleData newVehicle;
                    newVehicle.bbox = box;
                    newVehicle.speed = 0.0;
                    newVehicle.position = center;
                    newVehicle.speedReferencePosition = center;
                    newVehicle.lastSpeedUpdateTime = currentFrameTime;
                    newVehicle.framesSinceLastSpeedCalculation = 0;
                    newVehicle.hasValidSpeed = false;
                    
                    trackedVehicles[nextVehicleId] = newVehicle;
                    vehicleUpdated[nextVehicleId] = true;
                    nextVehicleId++;
                }
            }
            
            // Remove unmatched vehicles after several frames of absence
            const int maxMissingFrames = 5;
            static map<int, int> missingFrameCount;
            
            vector<int> idsToRemove;
            for (const auto& [id, _] : trackedVehicles) {
                if (vehicleUpdated.find(id) == vehicleUpdated.end()) {
                    // Vehicle wasn't found in this frame
                    missingFrameCount[id]++;
                    if (missingFrameCount[id] > maxMissingFrames) {
                        idsToRemove.push_back(id);
                    }
                } else {
                    // Reset counter for vehicles that were found
                    missingFrameCount[id] = 0;
                }
            }
            
            for (int id : idsToRemove) {
                trackedVehicles.erase(id);
                missingFrameCount.erase(id);
            }
            
            // Draw the road boundaries
            for (size_t i = 0; i < roadPoints.size(); i++) {
                circle(frame, roadPoints[i], 3, Scalar(0, 0, 255), -1);
                line(frame, 
                     roadPoints[i], 
                     roadPoints[(i+1) % roadPoints.size()], 
                     Scalar(0, 255, 0), 1);
            }
            
            // Draw tracked vehicles and speeds
            for (const auto& [id, vehicle] : trackedVehicles) {
                rectangle(frame, vehicle.bbox, Scalar(0, 255, 0), 2);
                
                // Draw ID and speed - now always showing speed in km/h
                string label = "ID:" + to_string(id);
                if (vehicle.hasValidSpeed) {
                    label += " " + to_string(int(vehicle.speed)) + " km/h";
                } else {
                    label += " -- km/h"; // Display placeholder until we calculate a speed
                }
                
                // Position the text for better visibility
                int textY = max(30, vehicle.bbox.y - 10);
                putText(frame, label, 
                        Point(vehicle.bbox.x, textY), 
                        FONT_HERSHEY_SIMPLEX, 0.5, 
                        Scalar(0, 255, 255), 2);
            }
            
            // Add frame counter
            string frameInfo = "Frame: " + to_string(frameCount);
            if (frameCount % speedCalculationInterval == 0) {
                frameInfo += " [Speed Update]";
            }
            putText(frame, frameInfo, Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
            
            // Display and write frame
            imshow("Vehicle Speed Tracking", frame);
            videoHandler.writeFrame(frame);
            
            // Check for exit key
            if (waitKey(1) == 27) break;
        }
        
        high_resolution_clock::time_point processingEnd = high_resolution_clock::now();
        duration<double> processingTime = duration_cast<duration<double>>(processingEnd - processingStart);
        
        cout << "Processed " << frameCount << " frames in " << processingTime.count() 
             << " seconds (" << frameCount / processingTime.count() << " FPS)" << endl;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }

    destroyAllWindows();
}