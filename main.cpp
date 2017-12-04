#define STRICT
#ifndef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WINXP  
#endif
#ifndef WINVER
#define WINVER        _WIN32_WINNT_WINXP
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT  _WIN32_WINNT_WINXP
#endif
#include <SdkDdkver.h>

// Library Includes
#include <GuidDef.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <process.h>
#include <tchar.h>
#include <Windows.h>


#include <thread>
#include <chrono>

#include "PowerEventListener.h"
#include "Monitor.h"
#include "Camera.h"

using namespace std::chrono_literals;

bool g_isRunning;


void updateBrightness() {
	int peakH = 13, peakM = 30;
	auto peakAt = peakH * 60 + peakM;
	const float pi = std::atan(1) * 4;

	auto now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	tm local_tm = *localtime(&tt);
	int minutes = local_tm.tm_hour * 60 + local_tm.tm_min;

	float brightness = 0.5 + cosf(static_cast<float>(peakAt - minutes) / static_cast<float>(24 * 60) * 2.0f * pi) * 0.5f;

	//float b2 = cam.getBrightness();

	auto mons = Monitor::EnumerateMonitors();
	for (auto &mon : mons) {
		mon.setCurrentBrightnessFraction(brightness);
	}
}

//int main(int argc, const char* argv[])
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	PowerEventListener pwrEvents([]() {
		updateBrightness();
	});

	Camera cam;

	g_isRunning = true;
	while (g_isRunning) {
		updateBrightness();

		std::this_thread::sleep_for(15min);
	}
	return 0;
}