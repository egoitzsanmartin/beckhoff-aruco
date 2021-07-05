#include "WriteFiles.h"


void writeArucoPoseInFile(Vec3d rvecs, Vec3d tvecs, Mat outputImage, std::ofstream* allOutfile, int n, int nImage) {
	String imgPath = "C:/Users/Administrator/Documents/aruco/img";
	String imgExtension = ".bmp";
	imgPath += std::to_string(n);
   /* std::ofstream outfile;
    outfile.open(posePath + std::to_string(nImage) + poseExtension);
    outfile << tvecs[0] << "\t" << tvecs[1] << "\t" << tvecs[2] << "\t" << rvecs[0] << "\t" << rvecs[1] << "\t" << rvecs[2] << "\n" << std::endl;
    outfile.close();*/
    *allOutfile << tvecs[0] << ';' << tvecs[1] << ';' << tvecs[2] << ';' << rvecs[0] << ';' << rvecs[1] << ';' << rvecs[2] << std::endl;

    //imwrite((imgPath + "/img" + std::to_string(nImage) + imgExtension), outputImage);
    //nImage++;
}

void writeRobotPoseInFile(char* buf, std::ofstream* allOutfile) {

	std::string result = buf;
	tinyxml2::XMLDocument doc;
	doc.Parse(buf);

	const char* X = doc.FirstChildElement("Rob")->FirstChildElement("X")->GetText();
	const char* Y = doc.FirstChildElement("Rob")->FirstChildElement("Y")->GetText();
	const char* Z = doc.FirstChildElement("Rob")->FirstChildElement("Z")->GetText();
	const char* A = doc.FirstChildElement("Rob")->FirstChildElement("A")->GetText();
	const char* B = doc.FirstChildElement("Rob")->FirstChildElement("B")->GetText();
	const char* C = doc.FirstChildElement("Rob")->FirstChildElement("C")->GetText();

	printf("X: %s\n", X);
	printf("Y: %s\n", Y);
	printf("Z: %s\n", Z);
	printf("A: %s\n", A);
	printf("B: %s\n", B);
	printf("C: %s\n", C);

	result = X;
	result.append(";");
	result.append(Y);
	result.append(";");
	result.append(Z);
	result.append(";");
	result.append(A);
	result.append(";");
	result.append(B);
	result.append(";");
	result.append(C);
	result.append(";");

	*allOutfile << buf << std::endl;

	/*
	printf("Z: %s\n", Z);
	printf("A: %s\n", A);
	printf("B: %s\n", B);
	printf("C: %s\n", C);*/


	//return doc.ErrorID();
	//getValuesFromXML(buf, newBuf, toGetSize.size());
	
}

void getValuesFromXML(char buf[], char destBuf[], size_t size) {
	int resultPos = 0;

	for (int i = 0; i < size; i++) {
		if (buf[i] == '\"' && buf[i + 1] != 'K') {
			int j = i + 1;
			while (buf[j] != '\"') {
				destBuf[resultPos++] = buf[j];
			}
			destBuf[resultPos++] = ';';
		}
	}
}

void createDirectory(std::string path) {
	size_t pos_it = 0;
	size_t pos = 0;
	std::string delimiter = "/";
	std::string token;
	const char* dir;
	std::string iterator = path;

	while ((pos_it = iterator.find(delimiter)) != std::string::npos) {
		pos += pos_it + 1;
		token = path.substr(0, pos);
		if (GetFileAttributesA(token.c_str()) == INVALID_FILE_ATTRIBUTES) {
			dir = token.c_str();
			_mkdir(dir);
		}
		iterator.erase(0, pos_it + delimiter.length());
	}

}