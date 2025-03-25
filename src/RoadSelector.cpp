#include "RoadSelector.h"
#include <iostream>

using namespace cv;
using namespace std;

vector<Point2f> selectedPoints;

void mouseCallback(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN && selectedPoints.size() < 4) {
        selectedPoints.push_back(Point2f(x, y));
        cout << "Point selected: (" << x << ", " << y << ")" << endl;
    }
}

vector<Point2f> RoadSelector::selectRoadPoints(const string& videoPath) {
    Mat frame;
    VideoCapture cap(videoPath);
    
    if (!cap.isOpened()) {
        cerr << "Error opening video!" << endl;
        return {};
    }

    cap.read(frame);
    namedWindow("Select Road Points");
    setMouseCallback("Select Road Points", mouseCallback, NULL);

    while (selectedPoints.size() < 4) {
        imshow("Select Road Points", frame);
        waitKey(1);
    }

    destroyWindow("Select Road Points");
    return selectedPoints;
}
