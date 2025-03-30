# Track and Calculate Vehicles Speed


https://github.com/user-attachments/assets/94866aa2-a34a-4c60-b7db-e50dea28c6ed


## Installation Instructions

To set up the necessary data files, please follow these steps:

1. Create a folder named `data` and navigate into it:
```sh
mkdir -p data && cd data
```

2. Download the following files using the provided commands:
```sh
wget -O coco.names https://raw.githubusercontent.com/pjreddie/darknet/master/data/coco.names
wget -O yolov4.cfg https://raw.githubusercontent.com/AlexeyAB/darknet/master/cfg/yolov4.cfg
wget -O yolov4.weights https://github.com/AlexeyAB/darknet/releases/download/yolov4/yolov4.weights
```

After downloading, your project directory should look like this:
```
trackerCpp/
│
├── data/
│   ├── coco.names
│   ├── yolov4.cfg
│   └── yolov4.weights
│
└── ...
```

## Test Video

For testing the application, use this video:
https://www.youtube.com/watch?v=wqctLW0Hb_0&t=1s

1. Download the video
2. Rename it to `road.mp4`
3. Create a `videos` folder and place the file there:
```sh
mkdir -p videos
# Place road.mp4 in the videos folder
```

Your updated project structure will look like:
```
trackerCpp/
│
├── data/
│   ├── coco.names
│   ├── yolov4.cfg
│   └── yolov4.weights
│
├── videos/
│   └── road.mp4
│
└── ...
```


## Build Configuration

This project uses the following VSCode tasks configuration for building with OpenCV:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Build C++ with OpenCV",
      "type": "shell",
      "command": "sh",
      "args": [
        "-c",
        "export PKG_CONFIG_PATH=$(brew --prefix opencv)/lib/pkgconfig:$PKG_CONFIG_PATH && g++ -std=c++17 -Iinclude -o tracking_app src/*.cpp $(pkg-config --cflags --libs opencv4)"
      ],
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "problemMatcher": ["$gcc"]
    }
  ]
}
```

## How Speed Calculation Works

The application calculates vehicle speed using the following approach:

1. The program requires traffic videos where the road lines are vertical in the frame.
2. Users must select two road lines that section the driving lane.
3. Based on standard road markings:
   - Average road line length is approximately 3 meters
   - The gap between lines is typically 4.5 meters the line length
   - For calculation purposes, the distance between selected lines is estimated at 15 meters (average for US and European roads)
![צילום מסך 2025-03-30 ב-13 04 43](https://github.com/user-attachments/assets/2cc106e3-42ac-46d4-986e-922a861a5d32)

### Calculation Method:
1. The tracker detects when a vehicle crosses the first line (start time)
2. It then detects when the same vehicle crosses the second line (end time)
3. The time difference is calculated (in seconds) using the video's frame rate: `time = (end_frame - start_frame) / fps`
4. Speed is calculated as: `speed = distance / time`
   - Where distance = 15 meters
   - Result is in meters per second, which can be converted to km/h or mph

This method provides a reasonable estimate of vehicle speed for traffic monitoring applications.
