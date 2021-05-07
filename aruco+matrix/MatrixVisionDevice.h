#include <apps/Common/exampleHelper.h>
#include <mvIMPACT_acquire_GenICam.h>
#include <mvIMPACT_acquire.h>
//#include <mvIMPACT_acquire_helper.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <chrono>

mvIMPACT::acquire::Device* initializeDevice(mvIMPACT::acquire::Device* pDev);
cv::Mat getImage(const Request* pRequest);
int getDataType(TImageBufferPixelFormat format);
void writeToStdout(const std::string& msg);
bool isDeviceSupportedBySample(const Device* const pDev);

using namespace mvIMPACT::acquire;
using namespace std;

static mutex s_mutex;