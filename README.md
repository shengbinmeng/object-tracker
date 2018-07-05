Track an ROI using [OpenCV](http://opencv.org/).

First, run [`prepare.sh`](./prepare.sh) (see the script content for what it does). Then, simply call `make` to build the executable `track`.

Usage:

	track -i <video_file> [-t tracker_type] [-m model_file]
	
	'tracker_type' can be: Boosting, MIL, KCF, TLD, MedianFlow, GOTURN. Default is KCL.
	If 'model_file' is given, the program will detect an object and track it. Otherwise, you will be asked to select ROI manually.
	
Examples:

	track -i videos/chaplin.mp4 -m opencv/data/haarcascades/haarcascade_frontalface_alt.xml
