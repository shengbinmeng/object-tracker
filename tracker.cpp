#include <iostream>
#include "opencv2/highgui.hpp"
#include "opencv2/tracking.hpp"
#include "opencv2/objdetect.hpp"

#define AUTO_SELECT_ROI 1
#define MODEL_FILE "./haarcascades/haarcascade_frontalface_alt.xml"

using namespace cv;
using namespace std;

static int detectFace(Mat image, Rect2d &roi, const char* cascadeFileName) {
	CascadeClassifier faceDetector;
	if (!faceDetector.load(cascadeFileName)) {
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

	vector<Rect> faces;
	int width = image.size().width;
	int height = image.size().height;
	int smaller = height < width ? height : width;
	int minSize = smaller * 0.2;

	faceDetector.detectMultiScale(image_gray, faces, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(minSize, minSize));

	printf("%lu face(s) detected.\n", faces.size());

	// Draw a circle around every face.
	for (size_t i = 0; i < faces.size(); i++) {
		Rect face = faces[i];
		printf("%d %d %d %d\n", face.x, face.y, face.width, face.height);
	}

	if (faces.size() > 0) {
		// Give the first one.
		roi = faces[0];
	} else {
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	string trackerTypes[6] = {"Boosting", "MIL", "KCF", "TLD", "MedianFlow", "GOTURN"};
	if (argc < 2) {
		printf("Usage: %s <video_file> [tracker_type]\n"
			  " [tracker_type] can be: Boosting, MIL, KCF, TLD, MedianFlow, GOTURN. Default is KCL.\n", argv[0]);
		return -1;
	}
	char* videoFileName = argv[1];
	VideoCapture video(videoFileName);
	if (!video.isOpened()) {
		printf("Open video failed: %s\n", videoFileName);
		return -1;
	}
	string trackerType = trackerTypes[2];
	if (argc > 2) {
		trackerType = string(argv[2]);
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
	int frameIndex = 0;
	int autoSelect = AUTO_SELECT_ROI;
	if (autoSelect) {
		while (video.read(frame)) {
			// Detect a face as ROI.
			int ret = detectFace(frame, roi, MODEL_FILE);
			if (ret < 0) {
				// No ROI for this frame.
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
		return -1;
	}

	tracker->init(frame, roi);
	
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
		} else {
			putText(frame, "Tracking failure detected", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
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
	return 0;
}
