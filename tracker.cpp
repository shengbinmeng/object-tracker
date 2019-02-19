#include <iostream>
#include <getopt.h>
#include "opencv2/highgui.hpp"
#include "opencv2/tracking.hpp"
#include "opencv2/objdetect.hpp"

#define ROI_DATA_FILE "roi-data.txt"

using namespace cv;
using namespace std;

static int detectObject(Mat image, Rect2d &roi, const char* cascadeFileName) {
	CascadeClassifier objectDetector;
	if (!objectDetector.load(cascadeFileName)) {
		printf("Load cascade file failed: %s\n", cascadeFileName);
		return -1;
	}

	if (image.empty()) {
		printf("Read image failed\n");
		return -1;
	}

	Mat image_gray;
	cvtColor(image, image_gray, COLOR_BGR2GRAY);
	equalizeHist(image_gray, image_gray);

	vector<Rect> objects;
	int width = image.size().width;
	int height = image.size().height;
	int smaller = height < width ? height : width;
	int minSize = smaller * 0.2;

	objectDetector.detectMultiScale(image_gray, objects, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(minSize, minSize));

	printf("%lu object(s) detected.\n", objects.size());

	for (size_t i = 0; i < objects.size(); i++) {
		Rect object = objects[i];
		printf("%d %d %d %d\n", object.x, object.y, object.width, object.height);
	}

	if (objects.size() > 0) {
		// Give the first one.
		roi = objects[0];
	} else {
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	string trackerTypes[6] = {"Boosting", "MIL", "KCF", "TLD", "MedianFlow", "GOTURN"};
	string trackerType = trackerTypes[2];
	char* videoFileName = NULL;
	char* modelFileName = NULL;
	char opt;
	int wrongArgument = 0;
	while ((opt = getopt(argc, argv, "i:t:m:")) != -1) {
		if (optarg && optarg[0] == '-') {
			wrongArgument = 1;
			printf("Argument starting with '-' is not allowed.\n");
			break;
		}
		switch (opt) {
			case 'i':
				videoFileName = optarg;
				break;
			case 't':
				trackerType = string(optarg);
				break;
			case 'm':
				modelFileName = optarg;
				break;
			case '?':
				wrongArgument = 1;
				break;
			default:
				break;
		}
	}

	if (!videoFileName) {
		wrongArgument = 1;
	}

	if (wrongArgument) {
		printf("Wrong argument.\n");
		printf("Usage: %s -i <video_file> [-t tracker_type] [-m model_file]\n"
			  " 'tracker_type' can be: Boosting, MIL, KCF, TLD, MedianFlow, GOTURN. Default is KCF.\n"
			   " If 'model_file' is given, the program will detect an object and track it. Otherwise, you will be asked to select ROI manually.\n", argv[0]);
		return -1;
	}

	VideoCapture video(videoFileName);
	if (!video.isOpened()) {
		printf("Open video failed: %s\n", videoFileName);
		return -1;
	}

	int i;
	for (i = 0; i < 6; i++) {
		if (trackerType == trackerTypes[i]) {
			break;
		}
	}
	if (i == 6) {
		printf("Unknown tracker type. Will use KCL as default.\n");
		trackerType = trackerTypes[2];
	}
	Ptr<Tracker> tracker;
	if (trackerType == trackerTypes[0])
		tracker = TrackerBoosting::create();
	if (trackerType == trackerTypes[1])
		tracker = TrackerMIL::create();
	if (trackerType == trackerTypes[2])
		tracker = TrackerKCF::create();
	if (trackerType == trackerTypes[3])
		tracker = TrackerTLD::create();
	if (trackerType == trackerTypes[4])
		tracker = TrackerMedianFlow::create();
	if (trackerType == trackerTypes[5])
		tracker = TrackerGOTURN::create();

	Mat frame;
	Rect2d roi;
	FILE *dataFile = fopen(ROI_DATA_FILE, "w");
	if (dataFile == NULL) {
		printf("Open data file failed: %s", ROI_DATA_FILE);
	}
	int frameIndex = 0;
	if (modelFileName) {
		while (video.read(frame)) {
			// Detect an object as ROI.
			int ret = detectObject(frame, roi, modelFileName);
			if (ret < 0) {
				// No ROI for this frame.
				putText(frame, "No ROI yet", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
				fprintf(dataFile, "%d %d %d %d %d\n", frameIndex, 0, 0, 0, 0);

				imshow("tracking", frame);
				waitKey(25);
				frameIndex++;
			} else {
				break;
			}
		}
	} else {
		video.read(frame);
		roi = selectROI("tracking", frame);
	}

	if (roi.area() == 0) {
		printf("No ROI selected.\n");
		return -1;
	}

	tracker->init(frame, roi);
	fprintf(dataFile, "%d %d %d %d %d\n", frameIndex, (int)roi.x, (int)roi.y, (int)roi.width, (int)roi.height);
	
	printf("Start the tracking process from frame %d, press 'q' to quit.\n", frameIndex);
	while (1) {
		video.read(frame);
		// If the frame is empty, break immediately.
		if (frame.empty()) {
			printf("No more frame.\n");
			break;
		}
		frameIndex++;

		bool ok = tracker->update(frame, roi);
		if (ok) {
			// Draw the tracked object.
			rectangle(frame, roi, Scalar(255, 0, 0), 2, 1);
			fprintf(dataFile, "%d %d %d %d %d\n", frameIndex, (int)roi.x, (int)roi.y, (int)roi.width, (int)roi.height);
		} else {
			putText(frame, "Tracking failure", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
			fprintf(dataFile, "%d %d %d %d %d\n", frameIndex, 0, 0, 0, 0);
		}
		// Display tracker type on frame.
		putText(frame, trackerType + " tracker", Point(100, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 2);

		imshow("tracking", frame);
		// Press 'q' on keyboard to quit.
		char c = (char)waitKey(25);
		if (c == 'q') {
			break;
		}
	}

	if (dataFile) {
		fclose(dataFile);
	}
	return 0;
}
