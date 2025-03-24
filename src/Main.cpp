#include "App.h"

int main() {
    App app;
    app.run("videos/road2.mp4", "data/yolov4.cfg", "data/yolov4.weights", "data/coco.names");
    return 0;
}