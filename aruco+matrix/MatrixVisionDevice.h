#include <apps/Common/exampleHelper.h>
#include <mvIMPACT_acquire_GenICam.h>
#include <mvIMPACT_acquire.h>
#include <mvIMPACT_acquire_helper.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <chrono>

mvIMPACT::acquire::Device* initializeDevice(mvIMPACT::acquire::Device* pDev);
cv::Mat getImage(std::shared_ptr<Request> pRequest);
int getDataType(TImageBufferPixelFormat format);
bool isDeviceSupportedBySample(const Device* const pDev);