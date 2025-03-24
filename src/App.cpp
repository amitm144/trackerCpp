#include "App.h"
#include <iostream>

using namespace cv;
using namespace std;

void App::run(const string &filename, const string &modelConfig,
              const string &modelWeights, const string &classFile)
{
    try
    {
        VideoHandler videoHandler(filename);
        YOLODetector detector(modelConfig, modelWeights, classFile);

        int frameCount = 0;
        const int PROCESS_EVERY_N_FRAMES = 5;

        const int MAX_TRACKED_OBJECTS = 100;
        vector<ObjectTracker> trackerList;

        Mat frame;
        bool isTracking = false;

        while (videoHandler.getFrame(frame))
        {
            if (frame.empty())
                continue;

            frameCount++;
            if (frameCount % PROCESS_EVERY_N_FRAMES != 0)
            {
                videoHandler.writeFrame(frame); // Just save the frame without processing
                continue;
            }
            // Detect vehicles in current frame
            vector<pair<Rect, string>> detectedObjects = detector.detectVehicles(frame);

            // Define colors for each vehicle type
            std::map<std::string, Scalar> vehicleColors = {
                {"car", Scalar(0, 255, 0)},   // Green
                {"truck", Scalar(255, 0, 0)}, // Blue
                {"bus", Scalar(255, 0, 0)},   // Blue
            };

            for (const auto &obj : detectedObjects)
            {
                Rect box = obj.first;
                string class_name = obj.second;
                Scalar color = vehicleColors.count(class_name) ? vehicleColors[class_name] : Scalar(255, 255, 255);
                rectangle(frame, box, color, 4);
            }

            // Update all existing trackers
            for (auto &tracker : trackerList)
            {
                tracker.update(frame);
                tracker.drawBoundingBox(frame);
            }

            imshow("Video feed", frame);
            videoHandler.writeFrame(frame);

            if (waitKey(25) >= 0)
                break;
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }

    destroyAllWindows();
}
