#include <iostream>
#include "opencv2/highgui.hpp"
#include "opencv2/tracking.hpp"

using namespace cv;
using namespace std;

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
	video.read(frame);
	Rect2d roi = selectROI("tracking", frame);
	if (roi.width == 0 || roi.height == 0) {
		return -1;
	}
	tracker->init(frame, roi);
	
	printf("Start the tracking process, press 'q' to quit.\n");
	while (1) {
		video.read(frame);
		// If the frame is empty, break immediately.
		if (frame.empty()) {
			printf("No more frame.\n");
			break;
		}
		
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
