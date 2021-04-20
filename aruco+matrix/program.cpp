#include "MatrixVisionDevice.h"

using namespace cv;
using namespace std;

void runProgram(mvIMPACT::acquire::Device* pDev);

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
	mvIMPACT::acquire::Device* pDev;
	pDev = getDeviceFromUserInput(devMgr, isDeviceSupportedBySample);

	// Inicializa la cámara.
	initializeDevice(pDev);

	// Corre el programa
	runProgram(pDev);
}

void runProgram(mvIMPACT::acquire::Device* pDev) {
	int nImage = 0;

    helper::RequestProvider requestProvider(pDev);
    requestProvider.acquisitionStart();

    while (true) {
        cv::Mat outputImage;

		// El programa queda en espera a que llegue una señal de trigger.
        std::shared_ptr<Request> pRequest = requestProvider.waitForNextRequest();

		// Recoge la imagen de la cámara
		outputImage = getImage(pRequest);
											 
		String imgPath = "C:/Users/Aita/Documents/imagenes/img";
		String imgExtension = ".bmp";
		std::string winname = "Display window ";
		namedWindow(winname, WINDOW_NORMAL);

		// Guarda la imagen.						
		imwrite((imgPath + std::to_string(nImage++) + imgExtension), outputImage);

		// Pone la imagen en pantalla.	
		imshow(winname, outputImage);
		char key = (char)cv::waitKey(waitTime);
		if (key == 27)
			break;
    }
    requestProvider.acquisitionStop();
}