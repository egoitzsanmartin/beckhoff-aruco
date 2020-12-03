#include "CamCalib.h"


void camCalib(cv::Mat *cameraMatrix, cv::Mat *distCoeffs) {

	*cameraMatrix = (cv::Mat_<double>(3, 3) << 2433.08656844317, 0.0, 1239.12812477619, //col1
		0.0, 2414.80610479345, 1049.52571086599, //col2 
		 0.0, 0.0, 1.0); //col3 

	*distCoeffs = (cv::Mat_<double>(5, 1) << -0.102651707555197, 0.150679127177187, 0.0, 0.0, 0.0);

}