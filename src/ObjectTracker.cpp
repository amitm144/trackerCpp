#include "ObjectTracker.h"

using namespace cv;
using namespace std;

ObjectTracker::ObjectTracker(double iouThreshold, int maxMissingFrames)
    : nextId(0), frameCount(0), iouThreshold(iouThreshold), maxMissingFrames(maxMissingFrames)
{
}

void ObjectTracker::update(const vector<pair<Rect, string>>& detections)
{
    map<int, bool> updated;
    for (const auto &detection : detections)
    {
        const Rect& box = detection.first;
        const string& label = detection.second;

        // Find closest match based on IOU
        int bestId = -1;
        double bestIOU = iouThreshold;

        for (const auto &[id, object] : trackedObjects)
        {
            if (updated.find(id) != updated.end())
                continue;

            Rect intersection = box & object.bbox;
            if (intersection.empty())
                continue;

            double iou = intersection.area() /
                         (double)(box.area() + object.bbox.area() - intersection.area());

            if (iou > bestIOU)
            {
                bestIOU = iou;
                bestId = id;
            }
        }

        // Calculate center position
        Point2f center(box.x + box.width / 2.0f, box.y + box.height / 2.0f);

        // Update existing or create new object
        if (bestId >= 0)
        {
            trackedObjects[bestId].bbox = box;
            trackedObjects[bestId].position = center;
            trackedObjects[bestId].missingFrames = 0;
            trackedObjects[bestId].classLabel = label;
            updated[bestId] = true;
        }
        else
        {
            TrackedObject newObject{box, center, 0, label};
            trackedObjects[nextId] = newObject;
            updated[nextId] = true;
            nextId++;
        }
    }

    // Remove objects that haven't been seen for a while
    for (auto it = trackedObjects.begin(); it != trackedObjects.end();)
    {
        if (updated.find(it->first) == updated.end())
        {
            it->second.missingFrames++;
            if (it->second.missingFrames > maxMissingFrames)
            {
                it = trackedObjects.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
}

void ObjectTracker::drawBoundingBoxes(Mat &frame, bool showLabels)
{
    for (const auto &[id, object] : trackedObjects)
    {
        rectangle(frame, object.bbox, Scalar(0, 255, 0), 1);
        
        if (showLabels)
        {
            string label = object.classLabel + " ID:" + to_string(id);
            putText(frame, label,
                    Point(object.bbox.x + 5, object.bbox.y - 5),
                    FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
        }
    }
}

bool ObjectTracker::getObjectPosition(int id, Point2f &position) const
{
    auto it = trackedObjects.find(id);
    if (it != trackedObjects.end())
    {
        position = it->second.position;
        return true;
    }
    return false;
}

const map<int, ObjectTracker::TrackedObject>& ObjectTracker::getAllObjects() const
{
    return trackedObjects;
}

int ObjectTracker::getFrameCount() const
{
    return frameCount;
}

void ObjectTracker::incrementFrameCount()
{
    frameCount++;
}