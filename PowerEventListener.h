#pragma once

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "kernel32.lib")



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

#include <GuidDef.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <process.h>
#include <tchar.h>
#include <Windows.h>

#include <stdexcept>


#define   WINDOW_CLASS_NAME   _T("HiddenWndClass")
#define   WINDOW_MENU_NAME    _T("MainMenu")

typedef struct thread_parameters
{
  HINSTANCE hinst; 
  HWND      hwnd;
} THREAD_PARAMS;


// DLL Datastructures
#ifndef PBT_POWERSETTINGCHANGE
#define PBT_POWERSETTINGCHANGE          0x8013
typedef struct {
    GUID PowerSetting;
    DWORD DataLength;
    UCHAR Data[1];
} POWERBROADCAST_SETTING, *PPOWERBROADCAST_SETTING;


#endif // PBT_POWERSETTINGCHANGE


class PowerEventListener {
private:
	THREAD_PARAMS  thread_params;
	HANDLE          window_thread;
	uint32_t          window_thread_id;

	std::function<void(void)> handler;

	// DLL Function Signatures
	typedef LPVOID(WINAPI *REGISTERPOWERSETTINGNOTIFICATION)(HANDLE, LPCGUID, DWORD);

	static PowerEventListener *instance(PowerEventListener *setInstances = nullptr) {
		static PowerEventListener *instance = nullptr;
		if (setInstances && instance)
			throw std::logic_error("single instance only!");
		if (setInstances) instance = setInstances;
		return instance;
	}

public:
	explicit PowerEventListener(std::function<void(void)> &&handler)
		:handler(handler)
		, thread_params{ 0 }
		, window_thread{0}
		, window_thread_id{ 0 }
	{
		instance(this);

		// Create a thread to handle creating a window so that we can receive broadcast messages
		// NOTE: We are using _beginthreadex rather than CreateThread because we call the CRT in our app.
		//       See: http://msdn.microsoft.com/en-us/library/kdzttdcb.aspx
		window_thread = (HANDLE)_beginthreadex(NULL, 0, &s_hidden_window_thread, (void*)this, 0, &window_thread_id);

		if (window_thread == NULL)
		{
			throw std::runtime_error("failed creating thread");
		}
	}


private:
	static unsigned __stdcall s_hidden_window_thread(void* p_args)
	{
		if (p_args == NULL)
			throw std::runtime_error("p_args NULL");
		static_cast<PowerEventListener*>(p_args)->hidden_window_thread();
	}

	unsigned __stdcall hidden_window_thread()
	{
		MSG            message;
		BOOL           message_return_code;
		WNDCLASS       window_class;
		int32_t          return_code = 0;

		// Register the window class for the main window. 
		window_class.style = 0;
		window_class.lpfnWndProc = (WNDPROC)window_message_handler;
		window_class.cbClsExtra = 0;
		window_class.cbWndExtra = 0;
		window_class.hInstance = NULL;
		window_class.hIcon = LoadIcon((HINSTANCE)NULL, IDI_APPLICATION);
		window_class.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
		window_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		window_class.lpszMenuName = WINDOW_MENU_NAME;
		window_class.lpszClassName = WINDOW_CLASS_NAME;

		if (!RegisterClass(&window_class))
			throw std::runtime_error("RegisterClass failed");

		thread_params.hinst = NULL;  // save instance handle 

		// Create the main window. 
		// http://msdn.microsoft.com/en-us/library/ms632679(v=vs.85).aspx
		thread_params.hwnd = CreateWindow(WINDOW_CLASS_NAME, "", WS_OVERLAPPEDWINDOW,
			-1, -1, 0, 0, (HWND)NULL, (HMENU)NULL, thread_params.hinst, (LPVOID)NULL);
		// If the main window cannot be created, terminate 
		// the application. 
		if (!thread_params.hwnd)
		{
			throw std::runtime_error("CreateWindow failed");
		}

		// Register for Power Events
		register_for_powersettingnotifications(thread_params.hwnd);
		

		// Start the message loop. 
		while ((message_return_code = GetMessage(&message, NULL, 0, 0)) != 0)
		{
			if (message_return_code != FALSE)
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		return 0;
	}

	void register_for_powersettingnotifications(HWND hWnd)
	{
		LPVOID                            h_battery_percentage_notify;
		LPVOID                            h_monitor_power_notify;
		LPVOID                            h_power_source_notify;
		HMODULE                           hm_user32lib;
		REGISTERPOWERSETTINGNOTIFICATION  register_power_setting_notification;

		if (hWnd == NULL)
			throw std::runtime_error("hWnd NULL");

		hm_user32lib = LoadLibrary(_T("User32.dll"));
		if (hm_user32lib == NULL)
			throw std::runtime_error("error loading user32.dll");

		register_power_setting_notification = (REGISTERPOWERSETTINGNOTIFICATION)GetProcAddress(hm_user32lib, "RegisterPowerSettingNotification");
		if (register_power_setting_notification == NULL)
			throw std::runtime_error("error locating RegisterPowerSettingNotification");

		if ((h_power_source_notify = register_power_setting_notification(hWnd,
			&GUID_ACDC_POWER_SOURCE,
			DEVICE_NOTIFY_WINDOW_HANDLE)) == NULL)
		{
throw std::runtime_error("FAILURE_REGISTER_POWER_NOTIFY");
		}
		if ((h_battery_percentage_notify = register_power_setting_notification(hWnd,
			&GUID_BATTERY_PERCENTAGE_REMAINING,
			DEVICE_NOTIFY_WINDOW_HANDLE)) == NULL)
		{
			throw std::runtime_error("FAILURE_REGISTER_POWER_NOTIFY");
		}
		if ((h_monitor_power_notify = register_power_setting_notification(hWnd,
			&GUID_MONITOR_POWER_ON,
			DEVICE_NOTIFY_WINDOW_HANDLE)) == NULL)
		{
			throw std::runtime_error("FAILURE_REGISTER_POWER_NOTIFY");
		}

		FreeLibrary(hm_user32lib);
	}



	static LRESULT CALLBACK window_message_handler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT return_code = 0;

		switch (message)
		{
		case WM_POWERBROADCAST:
			instance()->handler();
			break;
		case WM_DESTROY:
			PostQuitMessage(ERROR_SUCCESS);
			break;
		default:
			return_code = DefWindowProc(hWnd, message, wParam, lParam);
		}

		return return_code;
	}

};