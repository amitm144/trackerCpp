#ifndef APP_H
#define APP_H

#include "VideoHandler.h"
#include "ObjectTracker.h"
#include "YOLODetector.h"
#include <string>

class App {
public:
    void run(const std::string& filename, const std::string& modelConfig, 
             const std::string& modelWeights, const std::string& classFile);
};

#endif