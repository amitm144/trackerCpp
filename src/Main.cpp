#include "App.h"
#include "RoadSelector.h"
#include <vector>

int main() {
    // Only select reference lines (21m apart)
    std::vector<cv::Point2f> referenceLines = RoadSelector::selectReferenceLines("videos/road.mp4");
    
    App app;
    app.run("videos/road.mp4", "data/yolov4.cfg", "data/yolov4.weights", "data/coco.names", referenceLines);
    return 0;
}