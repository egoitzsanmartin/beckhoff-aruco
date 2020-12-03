#include <string>
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

bool readDetectorParameters(string filename, Ptr<cv::aruco::DetectorParameters>& params);
