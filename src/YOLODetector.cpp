#include "YOLODetector.h"
#include <fstream>
#include <stdexcept>

using namespace cv;
using namespace std;

YOLODetector::YOLODetector(const string &modelConfig, const string &modelWeights, const string &classFile)
{
    net = dnn::readNetFromDarknet(modelConfig, modelWeights);
    net.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(dnn::DNN_TARGET_CPU);
    loadClassNames(classFile);
}

void YOLODetector::loadClassNames(const string &classFile)
{
    ifstream ifs(classFile);
    if (!ifs.is_open())
    {
        throw runtime_error("Failed to open class names file: " + classFile);
    }
    
    string line;
    classNames.reserve(100); // Reserve reasonable space for class names
    while (getline(ifs, line))
    {
        classNames.push_back(line);
    }
}

vector<pair<Rect, string>> YOLODetector::detectVehicles(const Mat &frame)
{
    if (frame.empty())
    {
        return {};
    }

    Mat blob;
    Mat inputFrame;
    
    if (frame.channels() == 4)
    {
        cvtColor(frame, inputFrame, COLOR_BGRA2BGR);
    }
    else
    {
        inputFrame = frame;  // No need to clone if we're not modifying it
    }

    dnn::blobFromImage(inputFrame, blob, 1 / 255.0, Size(320, 320), true, false);
    net.setInput(blob);

    vector<Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    return postprocess(inputFrame, outputs);
}

vector<pair<Rect, string>> YOLODetector::postprocess(const Mat &frame, const vector<Mat> &outputs)
{
    vector<Rect> boxes;
    vector<int> classIds;
    vector<float> confidences;

    constexpr float CONFIDENCE_THRESHOLD = 0.5;
    constexpr float NMS_THRESHOLD = 0.4;

    for (const auto &output : outputs)
    {
        for (int i = 0; i < output.rows; i++)
        {
            float confidence = output.at<float>(i, 4);
            if (confidence > CONFIDENCE_THRESHOLD)
            {
                Mat scores = output.row(i).colRange(5, output.cols);
                Point classIdPoint;
                double maxVal;
                minMaxLoc(scores, nullptr, &maxVal, nullptr, &classIdPoint);
                int classId = classIdPoint.x;

                if (classId >= classNames.size())
                    continue;
                    
                const string& class_name = classNames[classId];

                if (class_name == "car" || class_name == "truck" || class_name == "bus")
                {
                    int centerX = static_cast<int>(output.at<float>(i, 0) * frame.cols);
                    int centerY = static_cast<int>(output.at<float>(i, 1) * frame.rows);
                    int width = static_cast<int>(output.at<float>(i, 2) * frame.cols);
                    int height = static_cast<int>(output.at<float>(i, 3) * frame.rows);
                    
                    boxes.emplace_back(centerX - width / 2, centerY - height / 2, width, height);
                    classIds.push_back(classId);
                    confidences.push_back(confidence);
                }
            }
        }
    }

    // Apply Non-Maximum Suppression (NMS)
    vector<int> indices;
    dnn::NMSBoxes(boxes, confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);

    vector<pair<Rect, string>> finalDetections;
    finalDetections.reserve(indices.size());
    
    for (int idx : indices)
    {
        finalDetections.emplace_back(boxes[idx], classNames[classIds[idx]]);
    }

    return finalDetections;
}