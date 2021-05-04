#include "CamCalib.h"
#include "MatrixVisionDevice.h"
#include "UDP_Server.h"
#include "Parameters.h"
#include "WriteFiles.h"
#include "ADS.h"

using namespace cv;
using namespace std;

void runProgram(mvIMPACT::acquire::Device* pDev, int n);

int waitTime = 1;													// 1 milisegundo.
bool display = true;												// Para activar o desactivar la reproducción de las imágenes.
bool writeInFile = false;											// Para activar o desactivar la función de esctibir las poses en ficheros.
bool saveImage = false;												// Para activar o desactivar el guardado de imagen en disco.
bool noArUco = false;												// Activar si se quiere capturar imagen sin detectar ArUcos.
float markerSize = 50;												// Tamaño en milímetros del marcador ArUco.
String imgPath = "C:/Users/Administrator/Documents/aruco/img";		// Dirección de guardado de imagen
String imgExtension = ".bmp";										// Formato de imagen

int main() {
	mvIMPACT::acquire::DeviceManager devMgr;

	// Cuenta la cantidad de dispositivos (cámaras) disponibles.
	if (devMgr.deviceCount() == 0)    
	{
		cout << "No device found! Unable to continue!" << endl;
		cin.get();
	}
	vector<mvIMPACT::acquire::Device*> pDevs;
	int size;
	std::list<std::thread> threads = {};

	size = getValidDevices(devMgr, pDevs);

	// Por cada cámara se inicia un dispositivo y se inicia un hilo con la función "runProgram" para cámara.
	for (int i = 0; i < size; i++) {
		threads.emplace_back(std::thread(runProgram, pDevs.at(i), i));
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
	initializeDevice(pDev);

	int nImage = 0;

	// Si es el primer hilo, abre un fichero para la pose del robot y comienza un hilo para gestionar la conexión de ADS.
	if (n == 0) {																	  
		robotFile.open("C:/Users/Administrator/Documents/aruco/poses/robotPose.txt");
	//	adsThread = std::thread(startAdsConnection, &robotFile);
	}

	// Abre un archivo para las poses de los marcadores.
	String path = "C:/Users/Administrator/Documents/aruco/poses/arucoPose";
	path += std::to_string(n);
	path += ".txt";
	arucoFile.open(path);

	FunctionInterface requestProvider(pDev);
	const unsigned int timeout_ms = { 500 };
	int requestNr = INVALID_ID;
	int lastRequestNr = { INVALID_ID };
	Request* pRequest = nullptr;

	TDMR_ERROR result = DMR_NO_ERROR;
	while ((result = static_cast<TDMR_ERROR>(requestProvider.imageRequestSingle())) == DMR_NO_ERROR) {};
	if (result != DEV_NO_FREE_REQUEST_AVAILABLE)
	{
		cout << "'FunctionInterface.imageRequestSingle' returned with an unexpected result: " << result
			<< "(" << ImpactAcquireException::getErrorCodeAsString(result) << ")" << endl;
	}

	// Se definen los parametros necesarios para la detección de ArUcos.
	std::vector<int> markerIds;

	std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
	cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
	cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);
	requestProvider.acquisitionStart();

	cv::Mat outputImage, rgbOutputImage, cameraMatrix, distCoeffs;
    while (true) {
		// Comienza a contar el tiempo.
        auto start = std::chrono::high_resolution_clock::now();

		// El programa queda en espera a que llegue una señal de trigger.
		requestNr = requestProvider.imageRequestWaitFor(timeout_ms);
		if (requestProvider.isRequestNrValid(requestNr))
		{
			pRequest = requestProvider.getRequest(requestNr);
			cout << pRequest->isOK() << endl;
			if (pRequest->isOK()) {
				outputImage = getImage(pRequest);

				camCalib(&cameraMatrix, &distCoeffs);

				// Lee el fichero con los parametros.
				bool e = readDetectorParameters("detector_params.yml", parameters);

				// Esta función es la encargada de detectar los marcadores.
				cv::aruco::detectMarkers(outputImage, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
				cv::cvtColor(outputImage, rgbOutputImage, cv::COLOR_GRAY2RGB);
				if (markerIds.size() > 0 && !noArUco) {

					if (display) {
						cv::aruco::drawDetectedMarkers(rgbOutputImage, markerCorners, markerIds);
					}

					std::vector<cv::Vec3d> rvecs, tvecs;
					Mat m33(3, 3, CV_64F);

					// Esta función estima las posiciones de los marcadores
					// y los guarda en rvecs y tvecs.
					cv::aruco::estimatePoseSingleMarkers(markerCorners, markerSize, cameraMatrix, distCoeffs, rvecs, tvecs);

					// Pasa rvecs a Rodrigues
				   // cv::Rodrigues(rvecs, m33);
					for (int i = 0; i < markerIds.size(); i++) {
						// Dibuja los ejes del marcador en la imagen.

						if (display) {
							cv::aruco::drawAxis(rgbOutputImage, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 25);
						}

						//std::cout << "r(euler): " << Degree_euler << " s\n";
						//std::cout << "r: " << m33.row(i) << " s\n";
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

					std::string winname = "Display window ";
					winname += std::to_string(n);
					namedWindow(winname, WINDOW_NORMAL);

					// Guarda la imagen en fichero.		
					if (saveImage) {
						string path = imgPath + std::to_string(n) + "/img" + std::to_string(nImage++) + imgExtension;
						imwrite(path, rgbOutputImage);
					}

					imshow(winname, rgbOutputImage);

					// Sin el waitTime no aparece la imagen en ventana.
					char key = (char)cv::waitKey(waitTime);
					if (key == 27)
						break;
				}
			}
			if (requestProvider.isRequestNrValid(lastRequestNr))
			{
				// this image has been displayed thus the buffer is no longer needed...
				requestProvider.imageRequestUnlock(lastRequestNr);
			}
			lastRequestNr = requestNr;
			// send a new image request into the capture queue
			requestProvider.imageRequestSingle();
		}
    }
	//arucoFile.close();

	// Espera al hilo de ADS.
	if (n == 0) {
		adsThread.join();
	}

	// Cierra el archivo de texto
	robotFile.close(); 
    requestProvider.acquisitionStop();
}

