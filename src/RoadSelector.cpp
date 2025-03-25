#include "RoadSelector.h"
#include <iostream>

using namespace cv;
using namespace std;

// Global variables for point selection
vector<Point2f> selectedPoints;
Mat displayFrame;
const string windowName = "Select Road Points";

// Point selection order labels
const vector<string> pointLabels = {
    "Top-Left", "Top-Right", "Bottom-Right", "Bottom-Left"
};

void mouseCallback(int event, int x, int y, int flags, void* userdata) {
    // Clone the original frame for drawing
    Mat temp = displayFrame.clone();
    
    // Draw existing points and lines connecting them
    for (size_t i = 0; i < selectedPoints.size(); i++) {
        // Draw point
        circle(temp, selectedPoints[i], 5, Scalar(0, 0, 255), -1);
        // Add label
        putText(temp, pointLabels[i], selectedPoints[i] + Point2f(10, 10), 
                FONT_HERSHEY_SIMPLEX,
                0.5, Scalar(0, 0, 255), 2);
        
        // Draw lines between points
        if (i > 0) {
            line(temp, selectedPoints[i-1], selectedPoints[i], Scalar(0, 255, 0), 2);
        }
        // Close the polygon if all points are selected
        if (i == 3) {
            line(temp, selectedPoints[3], selectedPoints[0], Scalar(0, 255, 0), 2);
        }
    }
    
    // Handle click event
    if (event == EVENT_LBUTTONDOWN && selectedPoints.size() < 4) {
        selectedPoints.push_back(Point2f(x, y));
        cout << "Point " << pointLabels[selectedPoints.size()-1] 
             << " selected: (" << x << ", " << y << ")" << endl;
        
        // Add the new point to the display
        circle(temp, Point2f(x, y), 5, Scalar(0, 0, 255), -1);
        putText(temp, pointLabels[selectedPoints.size()-1], 
                Point2f(x, y) + Point2f(10, 10), 
                FONT_HERSHEY_SIMPLEX, 
                0.5, Scalar(0, 0, 255), 2);
    }
    
    // Show instructions
    string instructions;
    if (selectedPoints.size() < 4) {
        instructions = "Select " + pointLabels[selectedPoints.size()] + " point of the road";
    } else {
        instructions = "All points selected. Press any key to continue.";
    }
    putText(temp, instructions, Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
    
    imshow(windowName, temp);
}

vector<Point2f> RoadSelector::selectRoadPoints(const string& videoPath) {
    selectedPoints.clear();  // Clear any previous points
    
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
    setMouseCallback(windowName, mouseCallback, NULL);

    // Display instructions
    putText(displayFrame, "Select Top-Left point of the road", 
            Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
    imshow(windowName, displayFrame);

    // Wait for all points to be selected
    while (selectedPoints.size() < 4) {
        int key = waitKey(100);
        if (key == 27) { // ESC key
            selectedPoints.clear();
            break;
        }
    }
    
    // Show final selection and wait for user confirmation
    if (selectedPoints.size() == 4) {
        // Draw the complete quadrilateral
        Mat finalView = displayFrame.clone();
        for (size_t i = 0; i < 4; i++) {
            circle(finalView, selectedPoints[i], 5, Scalar(0, 0, 255), -1);
            putText(finalView, pointLabels[i], selectedPoints[i] + Point2f(10, 10), 
                    FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
            line(finalView, selectedPoints[i], selectedPoints[(i+1)%4], Scalar(0, 255, 0), 2);
        }
        putText(finalView, "Points selected. Press any key to continue.", 
                Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
        imshow(windowName, finalView);
        waitKey(0);
    }

    destroyWindow(windowName);
    return selectedPoints;
}