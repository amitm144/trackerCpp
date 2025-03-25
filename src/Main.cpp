#include "App.h"
#include "RoadSelector.h"
#include <vector>

int main() {
    App app;
    std::vector<cv::Point2f> roadPoints = RoadSelector::selectRoadPoints("videos/road.mp4");
    app.run("videos/road.mp4", "data/yolov4.cfg", "data/yolov4.weights", "data/coco.names", roadPoints);
    return 0;
}
