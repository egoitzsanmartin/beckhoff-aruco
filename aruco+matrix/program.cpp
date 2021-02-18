#include "CamCalib.h"
#include "MatrixVisionDevice.h"
#include "UDP_Server.h"
#include "Parameters.h"
#include "WriteFiles.h"
#include "ADS.h"

using namespace cv;
using namespace std;

void runProgram(mvIMPACT::acquire::Device* pDev, int n);

int waitTime = 1;					// 1 milisegundo.
bool display = true;				// Para activar o desactivar la reproducción de las imágenes.
bool writeInFile = true;			// Para activar o desactivar la función de esctibir las poses en ficheros.
bool noArUco = true;				// Activar si se quiiere capturar imagen sin detectar ArUcos.
float markerSize = 50;				// Tamaño en milímetros del marcador ArUco.

int main() {
	mvIMPACT::acquire::DeviceManager devMgr;

	// Cuenta la cantidad de dispositivos (cámaras) disponibles.
	if (devMgr.deviceCount() == 0)    
	{
		cout << "No device found! Unable to continue!" << endl;
		cin.get();
	}
	mvIMPACT::acquire::Device* pDev[6];
	std::list<std::thread> threads = {};

	// Por cada cámara se inicia un dispositivo y se inicia un hilo con la función "runProgram" para cámara.
	for (int i = 0; i < devMgr.deviceCount(); i++) {
		pDev[i] = getDeviceFromUserInput(devMgr, isDeviceSupportedBySample);
		initializeDevice(pDev[i]);
		threads.emplace_back(std::thread(runProgram, pDev[i], i)); 
	}
	// Espera a que todos los hilos de los dispositivos terminen.
	for (auto& t : threads) {
		t.join();				 
	}
}

void runProgram(mvIMPACT::acquire::Device* pDev, int n) {
	ofstream robotFile;
	ofstream arucoFile;
	std::thread adsThread;

	int nImage = 0;

	// Si es el primer hilo, abre un fichero para la pose del robot y comienza un hilo para gestionar la conexión de ADS.
	if (n == 0) {																	  
		robotFile.open("C:/Users/Administrator/Documents/aruco/poses/robotPose.txt");
		adsThread = std::thread(startAdsConnection, &robotFile);
	}

	// Abre un archivo para las poses de los marcadores.
	String path = "C:/Users/Administrator/Documents/aruco/poses/arucoPose";
	path += std::to_string(n);
	path += ".txt";
	arucoFile.open(path);

    helper::RequestProvider requestProvider(pDev);
	std::vector<int> markerIds;

	// Se definen los parametros necesarios para la detección de ArUcos.
	std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;										 
	cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
	cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);
    requestProvider.acquisitionStart();

    while (true) {
        cv::Mat outputImage, cameraMatrix, distCoeffs;

		// Comienza a contar el tiempo.
        auto start = std::chrono::high_resolution_clock::now();

		// El programa queda en espera a que llegue una señal de trigger.
        std::shared_ptr<Request> pRequest = requestProvider.waitForNextRequest(); 
		outputImage = getImage(pRequest);

        camCalib(&cameraMatrix, &distCoeffs);

		// Lee el fichero con los parametros.
        bool e = readDetectorParameters("detector_params.yml", parameters);

		// Esta función es la encargada de detectar los marcadores.
        cv::aruco::detectMarkers(outputImage, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);  

        if (markerIds.size() > 0) {
            if (display) {
                cv::aruco::drawDetectedMarkers(outputImage, markerCorners, markerIds);
            }

            std::vector<cv::Vec3d> rvecs, tvecs;
            Mat m33(3, 3, CV_64F), Degree_euler;

			// Esta función estima las posiciones de los marcadores
			// y los guarda en rvecs y tvecs.
            cv::aruco::estimatePoseSingleMarkers(markerCorners, markerSize, cameraMatrix, distCoeffs, rvecs, tvecs); 

            cv::Rodrigues(rvecs, m33);

            for (int i = 0; i < markerIds.size(); i++) {
				// Dibuja los ejes del marcador en la imagen.
				if (display) {
					cv::aruco::drawAxis(outputImage, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 25);  
				}

                //std::cout << "r(euler): " << Degree_euler << " s\n";
                std::cout << "r: " << m33 << " s\n";
                std::cout << "t: " << tvecs[i] << " s\n";

				// Guarda las posiciones del robot y del marcador.
                if (writeInFile) {				
                    getRobotPose(&robotFile);
                    writeArucoPoseInFile(rvecs[i], tvecs[i], outputImage, &arucoFile, n, nImage++);
                }
            }

			// Termina de contar el tiempo.
            auto finish = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> elapsed = finish - start;
            std::cout << "Capture: " << elapsed.count() << " s\n";
        }

		// Pone la imagen en pantalla.
        if (display) {														 
			String imgPath = "C:/Users/Administrator/Documents/aruco/img";
			String imgExtension = ".bmp";

			std::string winname = "Display window ";
			winname += std::to_string(n);
            namedWindow(winname, WINDOW_NORMAL);

			// Guarda la image
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

	// Espera al hilo de ADS
	if (n == 0) {
		adsThread.join();
	}

	// Cierra el archivo de texto
	robotFile.close(); 
    requestProvider.acquisitionStop();
}

