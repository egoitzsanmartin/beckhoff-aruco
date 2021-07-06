#include <string>
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <direct.h>
#include <windows.h>
#include <fstream>
#include <string>
#include "tinyxml2.h"

using namespace cv;
using namespace tinyxml2;

void writeArucoPoseInFile(cv::Vec3d rvecs, Vec3d tvecs, Mat outputImage, std::ofstream* allOutfile, int n, int nImage);
void getValuesFromXML(char buf[], char destBuf[], size_t size);
void writeRobotPoseInFile(char* buf, std::ofstream* allOutfile);
void createDirectory(std::string path);