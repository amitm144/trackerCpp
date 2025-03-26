#include "App.h"
#include <iostream>
#include <map>
#include <algorithm>

using namespace cv;
using namespace std;

void App::run(const string &filename, const string &modelConfig,
              const string &modelWeights, const string &classFile,
              const vector<Point2f> &referenceLines)
{
    try
    {
        // Initialize video handling
        VideoHandler videoHandler(filename);
        double frameRate = videoHandler.getFPS();

        // Initialize detector and speed estimator
        YOLODetector detector(modelConfig, modelWeights, classFile);
        SpeedEstimator speedEstimator(frameRate, referenceLines);

        // Vehicle tracking data structure
        struct VehicleTracker
        {
            Rect bbox;
            Point2f position;
            int missingFrames;
        };

        map<int, VehicleTracker> vehicles;
        int nextId = 0;
        int frameCount = 0;
        constexpr int FRAME_SKIP = 2;

        Mat frame;
        while (videoHandler.getFrame(frame))
        {
            if (frame.empty())
                continue;
                
            frameCount++;

            // Only process every FRAME_SKIP frames
            if (frameCount % FRAME_SKIP == 0)
            {
                // Detect vehicles in the frame
                vector<pair<Rect, string>> detections = detector.detectVehicles(frame);

                // Update tracked vehicles
                map<int, bool> updated;

                for (const auto &detection : detections)
                {
                    const Rect& box = detection.first;

                    // Find closest match based on IOU
                    int bestId = -1;
                    double bestIOU = 0.3;

                    for (const auto &[id, vehicle] : vehicles)
                    {
                        if (updated.find(id) != updated.end())
                            continue;

                        Rect intersection = box & vehicle.bbox;
                        if (intersection.empty())
                            continue;

                        double iou = intersection.area() /
                                     (double)(box.area() + vehicle.bbox.area() - intersection.area());

                        if (iou > bestIOU)
                        {
                            bestIOU = iou;
                            bestId = id;
                        }
                    }

                    // Calculate center position
                    Point2f center(box.x + box.width / 2.0f, box.y + box.height / 2.0f);

                    // Update existing or create new
                    if (bestId >= 0)
                    {
                        vehicles[bestId].bbox = box;
                        vehicles[bestId].position = center;
                        vehicles[bestId].missingFrames = 0;
                        updated[bestId] = true;

                        // Process for speed calculation
                        speedEstimator.processVehicle(bestId, center, frameCount);
                    }
                    else
                    {
                        VehicleTracker newVehicle{box, center, 0};
                        vehicles[nextId] = newVehicle;
                        updated[nextId] = true;
                        nextId++;
                    }
                }

                // Remove vehicles not found in this frame
                for (auto it = vehicles.begin(); it != vehicles.end();)
                {
                    if (updated.find(it->first) == updated.end())
                    {
                        it->second.missingFrames++;
                        if (it->second.missingFrames > 5)
                        {
                            it = vehicles.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            // Draw reference lines
            for (const auto& linePoint : referenceLines)
            {
                line(frame, Point(0, linePoint.y),
                     Point(frame.cols, linePoint.y),
                     Scalar(0, 255, 0), 1);
            }

            // Draw vehicle bounding boxes and speeds
            for (const auto &[id, vehicle] : vehicles)
            {
                rectangle(frame, vehicle.bbox, Scalar(0, 255, 0), 1);
                
                string label = speedEstimator.hasSpeed(id) ? 
                    to_string(int(speedEstimator.getSpeed(id))) + " km/h" : "-- km/h";

                putText(frame, label,
                        Point(vehicle.bbox.x + 5, vehicle.bbox.y - 5),
                        FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
            }

            imshow("Vehicle Speed Tracking", frame);
            videoHandler.writeFrame(frame);

            if (waitKey(1) == 27)  // ESC key
                break;
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }

    destroyAllWindows();
}