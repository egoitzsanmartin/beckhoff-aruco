#include "CamCalib.h"
#include "MatrixVisionDevice.h"
#include "UDP_Server.h"
#include "Parameters.h"
#include "WriteFiles.h"
#include "ADS.h"

using namespace cv;
using namespace std;

void runProgram(mvIMPACT::acquire::Device* pDev, int n);

int waitTime = 1; // 1 milisecond
bool display = true;  // to activate/deactivate display window
bool writeInFile = true;  // to activate/deactivate writing parameters in files
bool noArUco = true;
float markerSize = 50;

int main() {
	mvIMPACT::acquire::DeviceManager devMgr;

	if (devMgr.deviceCount() == 0)
	{
		cout << "No device found! Unable to continue!" << endl;
		cin.get(); //wait for any key press
	}
	mvIMPACT::acquire::Device* pDev[6];
	std::list<std::thread> threads = {};

	for (int i = 0; i < devMgr.deviceCount(); i++) {
		pDev[i] = getDeviceFromUserInput(devMgr, isDeviceSupportedBySample);
		initializeDevice(pDev[i]);
		threads.emplace_back(std::thread(runProgram, pDev[i], i));
	}

	for (auto& t : threads) {
		t.join();
	}
}

void runProgram(mvIMPACT::acquire::Device* pDev, int n) {
	ofstream robotFile;
	ofstream arucoFile;
	std::thread adsThread;

	int nImage = 0;
	if (n == 0) {
		robotFile.open("C:/Users/Administrator/Documents/aruco/poses/robotPose.txt");
		adsThread = std::thread(startAdsConnection, &robotFile);
	}
	String path = "C:/Users/Administrator/Documents/aruco/poses/arucoPose";
	path += std::to_string(n);
	path += ".txt";
	arucoFile.open(path);
    helper::RequestProvider requestProvider(pDev);
	std::vector<int> markerIds;

	std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
	cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
	cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);
    requestProvider.acquisitionStart();

    while (true) {
        cv::Mat outputImage, cameraMatrix, distCoeffs;
        auto start = std::chrono::high_resolution_clock::now();
        std::shared_ptr<Request> pRequest = requestProvider.waitForNextRequest();
        outputImage = getImage(pRequest);

        camCalib(&cameraMatrix, &distCoeffs);
        
        bool e = readDetectorParameters("detector_params.yml", parameters);

        cv::aruco::detectMarkers(outputImage, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);

        if (markerIds.size() > 0) {
            if (display) {
                cv::aruco::drawDetectedMarkers(outputImage, markerCorners, markerIds);
            }

            std::vector<cv::Vec3d> rvecs, tvecs;
            Mat m33(3, 3, CV_64F), Degree_euler;

            cv::aruco::estimatePoseSingleMarkers(markerCorners, markerSize, cameraMatrix, distCoeffs, rvecs, tvecs);

            cv::Rodrigues(rvecs, m33);
            // Degree_euler = m33 * 180 / CV_PI;

            for (int i = 0; i < markerIds.size(); i++) {
				if (display) {
					cv::aruco::drawAxis(outputImage, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 25);
				}

                //std::cout << "r(euler): " << Degree_euler << " s\n";
                std::cout << "r: " << m33 << " s\n";
                std::cout << "t: " << tvecs[i] << " s\n";

                if (writeInFile) {
                    getRobotPose(&robotFile);
                    writeArucoPoseInFile(rvecs[i], tvecs[i], outputImage, &arucoFile, n, nImage++);
                }
            }

            auto finish = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = finish - start;
            std::cout << "Capture: " << elapsed.count() << " s\n";
        }

        if (display) {
			String imgPath = "C:/Users/Administrator/Documents/aruco/img";
			String imgExtension = ".bmp";

			std::string winname = "Display window ";
			winname += std::to_string(n);
            namedWindow(winname, WINDOW_NORMAL);
			if (noArUco) {
				imwrite((imgPath + std::to_string(n) + "/img" + std::to_string(nImage++) + imgExtension), outputImage);
			}
            imshow(winname, outputImage);
            char key = (char)cv::waitKey(waitTime);
            if (key == 27)
                break;
        }
    }
	//arucoFile.close();
	if (n == 0) {
		adsThread.join();
	}
	robotFile.close();
    requestProvider.acquisitionStop();
}

