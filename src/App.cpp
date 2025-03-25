#include "App.h"
#include "SpeedEstimator.h"
#include "RoadSelector.h"
#include <iostream>

using namespace cv;
using namespace std;

void App::run(const string &filename, const string &modelConfig,
              const string &modelWeights, const string &classFile,
              const vector<Point2f>& roadPoints)
{
    try
    {
        VideoHandler videoHandler(filename);
        YOLODetector detector(modelConfig, modelWeights, classFile);

        SpeedEstimator speedEstimator(videoHandler.getFPS(), roadPoints); // ✅ Uses road boundaries

        vector<ObjectTracker> trackerList;
        Mat frame;

        while (videoHandler.getFrame(frame))
        {
            if (frame.empty()) continue;

            vector<pair<Rect, string>> detectedObjects = detector.detectVehicles(frame);

            for (auto &obj : detectedObjects) {
                Rect box = obj.first;
                string class_name = obj.second;

                // ✅ Compute speed using the road boundaries
                double speed = speedEstimator.calculateSpeed(box);

                // ✅ Display speed on the original frame (no warping)
                putText(frame, to_string(int(speed)) + " km/h",
                        Point(box.x, box.y - 10), FONT_HERSHEY_SIMPLEX, 0.7,
                        Scalar(0, 255, 255), 2);
                
                rectangle(frame, box, Scalar(0, 255, 0), 4);
            }

            imshow("Video Feed", frame);
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
