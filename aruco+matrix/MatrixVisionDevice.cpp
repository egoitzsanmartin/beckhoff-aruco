#include "MatrixVisionDevice.h"

using namespace mvIMPACT::acquire;
using namespace std;
using namespace cv;

//-----------------------------------------------------------------------------
mvIMPACT::acquire::Device* initializeDevice(mvIMPACT::acquire::Device* pDev) {

    if (pDev == nullptr)
    {
        cout << "Unable to continue! Press [ENTER] to end the application" << endl;
        cin.get();
    }
    try
    {
        cout << "Initialising the device. This might take some time..." << endl << endl;
        pDev->interfaceLayout.write(dilGenICam); // This is also done 'silently' by the 'getDeviceFromUserInput' function but your application needs to do this as well so state this here clearly!
        pDev->open();
        cout << "Device open" << endl << endl;
    }
    catch (const ImpactAcquireException& e)
    {
        // this e.g. might happen if the same device is already opened in another process...
        cout << "An error occurred while opening device " << pDev->serial.read()
            << "(error code: " << e.getErrorCodeAsString() << ")." << endl
            << "Press [ENTER] to end the application..." << endl;
        cin.get();
    }

    mvIMPACT::acquire::GenICam::AcquisitionControl ac(pDev);
    ac.triggerMode.writeS("On");

    return pDev;
}


cv::Mat getImage(std::shared_ptr<Request> pRequest) {
    cv::Mat image;
    int dataType = getDataType(pRequest->imagePixelFormat.read());
    image = Mat(cv::Size(pRequest->imageWidth.read(), pRequest->imageHeight.read()), dataType, pRequest->imageData.read(), pRequest->imageLinePitch.read());

    return image;
}

bool isDeviceSupportedBySample(const Device* const pDev)
//-----------------------------------------------------------------------------
{
    if (!pDev->interfaceLayout.isValid() &&
        !pDev->acquisitionStartStopBehaviour.isValid())
    {
        return false;
    }

    vector<TDeviceInterfaceLayout> availableInterfaceLayouts;
    pDev->interfaceLayout.getTranslationDictValues(availableInterfaceLayouts);
    return find(availableInterfaceLayouts.begin(), availableInterfaceLayouts.end(), dilGenICam) != availableInterfaceLayouts.end();
}

int getDataType(mvIMPACT::acquire::TImageBufferPixelFormat format) {
	int dataType;
	switch (format) {
	case ibpfMono8:
		dataType = CV_8UC1;
		break;
	case ibpfMono10:
	case ibpfMono12:
	case ibpfMono14:
	case ibpfMono16:
		dataType = CV_16UC1;
		break;
	case ibpfMono32:
		dataType = CV_32SC1;
		break;
	case ibpfBGR888Packed:
	case ibpfRGB888Packed:
		dataType = CV_8UC3;
		break;
	case ibpfRGBx888Packed:
		dataType = CV_8UC4;
		break;
	case ibpfRGB101010Packed:
	case ibpfRGB121212Packed:
	case ibpfRGB141414Packed:
	case ibpfRGB161616Packed:
		dataType = CV_16UC3;
		break;
	}
	return dataType;
}