#include "App.h"
#include <iostream>
#include <map>

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
        const int FRAME_SKIP = 2;

        Mat frame;
        while (videoHandler.getFrame(frame))
        {
            if (frame.empty())
                continue;
            frameCount++;

            bool processThisFrame = (frameCount % FRAME_SKIP == 0);
            if (processThisFrame)
            {
                // Detect vehicles in the frame
                vector<pair<Rect, string>> detections = detector.detectVehicles(frame);

                // Update tracked vehicles
                map<int, bool> updated;

                for (const auto &detection : detections)
                {
                    Rect box = detection.first;

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
                        VehicleTracker newVehicle;
                        newVehicle.bbox = box;
                        newVehicle.position = center;
                        newVehicle.missingFrames = 0;

                        vehicles[nextId] = newVehicle;
                        updated[nextId] = true;
                        nextId++;
                    }
                }

                // Remove vehicles not found in this frame
                vector<int> toRemove;
                for (auto &[id, vehicle] : vehicles)
                {
                    if (updated.find(id) == updated.end())
                    {
                        vehicle.missingFrames++;
                        if (vehicle.missingFrames > 5)
                        {
                            toRemove.push_back(id);
                        }
                    }
                }

                for (int id : toRemove)
                {
                    vehicles.erase(id);
                }
            }

            // Draw reference points and horizontal lines - thin style
            for (size_t i = 0; i < referenceLines.size(); i++)
            {
                line(frame, Point(0, referenceLines[i].y),
                     Point(frame.cols, referenceLines[i].y),
                     Scalar(0, 255, 0), 1);
            }

            for (const auto &[id, vehicle] : vehicles)
            {
                rectangle(frame, vehicle.bbox, Scalar(0, 255, 0), 1);
                string label;

                if (speedEstimator.hasSpeed(id))
                {
                    double speed = speedEstimator.getSpeed(id);
                    if (speed > 0)
                    {
                        label = to_string(int(speed)) + " km/h";
                    }
                    else
                    {
                        label = "-- km/h";
                    }
                }
                else
                {
                    label = "-- km/h";
                }

                putText(frame, label,
                        Point(vehicle.bbox.x + 5, vehicle.bbox.y - 5),
                        FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
            }

            imshow("Vehicle Speed Tracking", frame);
            videoHandler.writeFrame(frame);

            if (waitKey(1) == 27)
                break;
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }

    destroyAllWindows();
}