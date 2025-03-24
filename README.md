# Installation Instructions

To set up the necessary data files, please follow these steps:

1. Create a folder named `data` and navigate into it:
   ```sh
   mkdir -p yolo_models && cd data
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
