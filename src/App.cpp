#include "App.h"
#include "ObjectTracker.h"
#include <iostream>

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

        // Initialize object tracker with IOU threshold of 0.3 and max missing frames of 5
        ObjectTracker tracker(0.3, 5);
        
        int frameCount = 0;
        constexpr int FRAME_SKIP = 2;

        Mat frame;
        while (videoHandler.getFrame(frame))
        {
            if (frame.empty())
                continue;
                
            frameCount++;
            tracker.incrementFrameCount();

            if (frameCount % FRAME_SKIP == 0)
            {
                // Detect vehicles in the frame
                vector<pair<Rect, string>> detections = detector.detectVehicles(frame);

                // Update tracked objects with new detections
                tracker.update(detections);
                
                // Process tracked objects for speed estimation
                const auto& trackedObjects = tracker.getAllObjects();
                for (const auto& [id, _] : trackedObjects)
                {
                    Point2f position;
                    if (tracker.getObjectPosition(id, position))
                    {
                        speedEstimator.processVehicle(id, position, frameCount);
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

            // Draw tracked objects
            const auto& trackedObjects = tracker.getAllObjects();
            for (const auto& [id, _] : trackedObjects)
            {
                Point2f position;
                if (tracker.getObjectPosition(id, position))
                {
                    string speedLabel = speedEstimator.hasSpeed(id) ? 
                        to_string(int(speedEstimator.getSpeed(id))) + " km/h" : "-- km/h";
                    
                    putText(frame, speedLabel,
                            Point(position.x - 30, position.y - 15),
                            FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
                }
            }
            
            // Draw bounding boxes for all tracked objects
            tracker.drawBoundingBoxes(frame, false);  // false = don't show class labels (just boxes)

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