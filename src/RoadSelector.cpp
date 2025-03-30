#include "RoadSelector.h"
#include <iostream>

using namespace cv;
using namespace std;

// Global variables for point selection
vector<Point2f> referenceLines;
Mat displayFrame;
const string windowName = "Select Reference Lines (15m apart)";

void mouseCallback(int event, int x, int y, int flags, void* userdata) {
    Mat temp = displayFrame.clone();
    
    // Draw existing reference points and horizontal lines
    for (const auto& point : referenceLines) {            
        // Draw horizontal line through this point
        line(temp, Point(0, point.y), 
             Point(temp.cols, point.y), 
             Scalar(0, 255, 0), 1);
    }
    
    // Show distance label between lines
    if (referenceLines.size() == 2) {
        Point2f midPoint(temp.cols/2, (referenceLines[0].y + referenceLines[1].y)/2);
        putText(temp, "15m", midPoint, FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 0), 1);
    }
    
    // Handle click event
    if (event == EVENT_LBUTTONDOWN && referenceLines.size() < 2) {
        referenceLines.push_back(Point2f(x, y));
    }
    
    imshow(windowName, temp);
}

vector<Point2f> RoadSelector::selectReferenceLines(const string& videoPath) {
    referenceLines.clear();
    
    VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        cerr << "Error opening video!" << endl;
        return {};
    }

    // Get the first frame
    cap.read(displayFrame);
    if (displayFrame.empty()) {
        cerr << "Error reading first frame!" << endl;
        return {};
    }
    
    namedWindow(windowName);
    setMouseCallback(windowName, mouseCallback, nullptr);
    imshow(windowName, displayFrame);

    // Wait for both points to be selected
    while (referenceLines.size() < 2) {
        int key = waitKey(100);
        if (key == 27) { // ESC key
            referenceLines.clear();
            break;
        }
    }
    
    waitKey(0);
    destroyWindow(windowName);
    return referenceLines;
}