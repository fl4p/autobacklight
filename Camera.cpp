#include <thread>
#include <chrono>
#include "escapi.h"
#include "Camera.h"

using namespace std::chrono_literals;

int Camera::numDevices = 0;

Camera::Camera()
{
	if (numDevices == 0) {
		numDevices = setupESCAPI();
	}
	deviceNo = 0;
}

Camera::~Camera()
{
}

float Camera::getBrightness()
{
	SimpleCapParams capture;
	capture.mHeight = 16;
	capture.mWidth = 16;
	std::vector<int> buf(capture.mHeight * capture.mWidth);
	capture.mTargetBuf = buf.data();

	initCapture(deviceNo, &capture);
	doCapture(deviceNo);
	while (isCaptureDone(deviceNo) == 0)
	{
		std::this_thread::sleep_for(1ms);
	}
	deinitCapture(deviceNo);
	
	// normalize image energy

	// compute noise energy

	// estimate room brightness from noise

	uint32_t rS = 0, gS = 0, bS = 0;
	for (auto cl : buf) {
		rS += (cl >> 4 & 0xff);
		gS += (cl >> 2 & 0xff);
		bS += (cl >> 0 & 0xff);
	}

	uint32_t lumSum = (rS + rS + rS + bS + gS + gS + gS + gS) >> 3;

	return static_cast<float>(lumSum / buf.size()) / 255.0f;
}
