#include "CamCalib.h"
#include "MatrixVisionDevice.h"
#include "UDP_Server.h"
#include "Parameters.h"
#include "WriteFiles.h"
#include "ADS.h"
#include "exampleHelper.h"

using namespace cv;
using namespace std;

//void runProgram(shared_ptr<ThreadParameter> parameter, int n);
void endProgram();

static bool s_boTerminated = false;

int waitTime = 1;													// 1 milisegundo.
bool displayImage = true;											// Para activar o desactivar la reproducción de las imágenes.
bool writeInFile = true;			 								// Para activar o desactivar la función de esctibir las poses en ficheros.
bool saveImage = true;												// Para activar o desactivar el guardado de imagen en disco.
bool noArUco = false;												// Activar si se quiere capturar imagen sin detectar ArUcos.
float markerSize = 50;												// Tamaño en milímetros del marcador ArUco.
String imgPath = "C:/Users/Administrator/Documents/aruco/img";		// Dirección de guardado de imagen
//String imgPath = "C:/Users/Administrator/Documents/callib/img";
String imgExtension = ".bmp";										// Formato de 

class ThreadParameter
	//-----------------------------------------------------------------------------
{
	Device*             pDev_;
	int					index_;
public:
	explicit ThreadParameter(Device* pDev, int index) : pDev_(pDev), index_(index) {}
	ThreadParameter(const ThreadParameter& src) = delete;
	Device* device(void) const
	{
		return pDev_;
	}
	int index(void) const
	{
		return index_;
	}
};

void endProgram() {
	writeToStdout("Type any character to end the acquisition( the initialisation of the devices might take some time )");
	int respuesta;
	cin >> respuesta;
	// stop all threads again
	writeToStdout("Terminating live threads...");
	s_boTerminated = true;
}

void runProgram(shared_ptr<ThreadParameter> parameter, int n) {
	Device* pDev = parameter->device();
	ofstream robotFile;
	ofstream arucoFile;
	std::thread adsThread;

	initializeDevice(pDev);

	int nImage = 0;

	// Si es el primer hilo, abre un fichero para la pose del robot y comienza un hilo para gestionar la conexión de ADS.
	if (n == 0) {
		string path = "C:/Users/Administrator/Documents/aruco/poses/robotPose.txt";
		if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES) createDirectory(path);
		robotFile.open("C:/Users/Administrator/Documents/aruco/poses/robotPose.txt");
		//adsThread = std::thread(startAdsConnection, &robotFile);
	}

	// Abre un archivo para las poses de los marcadores.
	String path = "C:/Users/Administrator/Documents/aruco/poses/arucoPose";
	path += std::to_string(n);
	path += ".txt";
	if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES) createDirectory(path);
	arucoFile.open(path);

	Statistics statistics(pDev);
	FunctionInterface fi(pDev);

	TDMR_ERROR result = DMR_NO_ERROR;
	while ((result = static_cast<TDMR_ERROR>(fi.imageRequestSingle())) == DMR_NO_ERROR) {};
	if (result != DEV_NO_FREE_REQUEST_AVAILABLE)
	{
		lock_guard<mutex> lockedScope(s_mutex);
		cout << "'FunctionInterface.imageRequestSingle' returned with an unexpected result: " << result
			<< "(" << ImpactAcquireException::getErrorCodeAsString(result) << ")" << endl;
	}

	manuallyStartAcquisitionIfNeeded(pDev, fi);
	const Request* pRequest = nullptr;
	const unsigned int timeout_ms = { 500 };
	int requestNr = INVALID_ID;
	// we always have to keep at least 2 images as the display module might want to repaint the image, thus we
	// can't free it unless we have a assigned the display to a new buffer.
	int lastRequestNr = { INVALID_ID };
	unsigned int cnt = { 0 };
	// Se definen los parametros necesarios para la detección de ArUcos.
	std::vector<int> markerIds;

	std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
	cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
	cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);

	cv::Mat outputImage, rgbOutputImage, cameraMatrix, distCoeffs;
	while (!s_boTerminated) {
		// Comienza a contar el tiempo.
		auto start = std::chrono::high_resolution_clock::now();

		// El programa queda en espera a que llegue una señal de trigger.
		requestNr = fi.imageRequestWaitFor(timeout_ms);
		if (fi.isRequestNrValid(requestNr))
		{
			
			pRequest = fi.getRequest(requestNr);
			if (pRequest->isOK()) {
				outputImage = getImage(pRequest);

				camCalib(&cameraMatrix, &distCoeffs);

				// Lee el fichero con los parametros.
				bool e = readDetectorParameters("detector_params.yml", parameters);

				// Esta función es la encargada de detectar los marcadores.
				cv::aruco::detectMarkers(outputImage, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
				cv::cvtColor(outputImage, rgbOutputImage, cv::COLOR_GRAY2RGB);
				if (markerIds.size() > 0 && !noArUco) {

					if (displayImage) {
						cv::aruco::drawDetectedMarkers(rgbOutputImage, markerCorners, markerIds);
					}

					std::vector<cv::Vec3d> rvecs, tvecs;
					Mat m33(3, 3, CV_64F);

					// Esta función estima las posiciones de los marcadores y los guarda en rvecs y tvecs.
					cv::aruco::estimatePoseSingleMarkers(markerCorners, markerSize, cameraMatrix, distCoeffs, rvecs, tvecs);

					// Pasa rvecs a Rodrigues
				    // cv::Rodrigues(rvecs, m33);
					for (int i = 0; i < markerIds.size(); i++) {
						// Dibuja los ejes del marcador en la imagen.

						if (displayImage) {
							cv::aruco::drawAxis(rgbOutputImage, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 25);
						}

						//std::cout << "r(euler): " << Degree_euler << " s\n";
						//std::cout << "r: " << m33.row(i) << " s\n";
						lock_guard<mutex> lockedScope(s_mutex);
						std::cout << "t: " << tvecs[i] << " s\n";

						// Guarda las posiciones del robot y del marcador.
						if (writeInFile) {
							//getRobotPose(&robotFile);
							writeArucoPoseInFile(rvecs[i], tvecs[i], outputImage, &arucoFile, n, nImage);
						}
					}

					// Termina de contar el tiempo.
					auto finish = std::chrono::high_resolution_clock::now();

					std::chrono::duration<double> elapsed = finish - start;
					std::cout << "Capture: " << elapsed.count() << " s\n";
				}

				++cnt;
				// here we can display some statistical information every 100th image
				if (cnt % 100 == 0)
				{
					lock_guard<mutex> lockedScope(s_mutex);
					cout << "Info from " << pDev->serial.read()
						<< ": " << statistics.framesPerSecond.name() << ": " << statistics.framesPerSecond.readS()
						<< ": " << statistics.bandwidthConsumed.name() << ": " << statistics.bandwidthConsumed.readS()
						<< ", " << statistics.errorCount.name() << ": " << statistics.errorCount.readS()
						<< ", " << statistics.captureTime_s.name() << ": " << statistics.captureTime_s.readS() << endl;
				}

				// Pone la imagen en pantalla.
				if (displayImage) {

					std::string winname = "Display window ";
					winname += std::to_string(n);
					namedWindow(winname, WINDOW_NORMAL);

					// Guarda la imagen en fichero.		
					if (saveImage) {
						std::string n_string = std::string(5 - std::to_string(nImage).length(), '0') + std::to_string(nImage++);
						string path = imgPath + std::to_string(n) + "/img" + n_string + imgExtension;
						if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES) createDirectory(path);
						imwrite(path, outputImage);
					}

					imshow(winname, outputImage);

					// Sin el waitTime no aparece la imagen en ventana.
					char key = (char)cv::waitKey(waitTime);
					if (key == 27)
						break;
				}
			}
			else
			{
				writeToStdout("Error: " + pRequest->requestResult.readS());
			}
			if (fi.isRequestNrValid(lastRequestNr))
			{
				// this image has been displayed thus the buffer is no longer needed...
				fi.imageRequestUnlock(lastRequestNr);
			}
			lastRequestNr = requestNr;
			// send a new image request into the capture queue
			fi.imageRequestSingle();
		}

	}
	//arucoFile.close();

	// Espera al hilo de ADS.
	if (n == 0) 
	{
	//	adsThread.join();
	}

	// Cierra el archivo de texto
	robotFile.close();
	manuallyStopAcquisitionIfNeeded(pDev, fi);

	if (fi.isRequestNrValid(requestNr))
	{
		fi.imageRequestUnlock(requestNr);
	}
	// clear all queues
	fi.imageRequestReset(0, 0);

	writeToStdout("Device " + to_string(n) + " closed.");
}



int main(int argc, char* argv[]) 
{
	DeviceManager devMgr;
	const unsigned int devCnt = devMgr.deviceCount();

	// Cuenta la cantidad de dispositivos (cámaras) disponibles.
	if (devCnt == 0)
	{
		cout << "No MATRIX VISION device found! Unable to continue!" << endl;
		return 1;
	}

	map<shared_ptr<ThreadParameter>, shared_ptr<thread>> threads;

	// Por cada cámara se inicia un dispositivo y se inicia un hilo con la función "runProgram" para cámara.
	cout << "Pulse [1] si quiere utilizar todas las cámaras" << endl;
	cout << "Pulse [2] si quiere utilizar una sola cámara" << endl;

	int respuesta;
	cin >> respuesta;
	if (respuesta == 1) {
		for (unsigned int i = 0; i < devCnt; i++)
		{
			shared_ptr<ThreadParameter> pParameter = make_shared<ThreadParameter>(devMgr[i], i);
			threads[pParameter] = make_shared<thread>(runProgram, pParameter, i);
			writeToStdout(devMgr[i]->family.read() + "(" + devMgr[i]->serial.read() + ")");
		}
		thread stop(endProgram);
		for (auto& t : threads) {
			if (t.second->joinable()) 
			{
				t.second->join();
			}
		}
		stop.join();
	}
	else if (respuesta == 2) {
		Device* pDev = getDeviceFromUserInput(devMgr);
		shared_ptr<ThreadParameter> pParameter = make_shared<ThreadParameter>(pDev, 0);
		threads[pParameter] = make_shared<thread>(runProgram, pParameter, 0);
		writeToStdout(pDev->family.read() + "(" + pDev->serial.read() + ")");

		thread stop(endProgram);
		for (auto& t : threads) {
			if (t.second->joinable()) 
			{
				t.second->join();
			}
		}
		stop.join();
	}

}

