#ifndef __AUTH_APP__
#define __AUTH_APP__
#include "windows.h"

class AuthApp {
public:
	AuthApp();
	~AuthApp();

	void OpenIE();
	BOOL  CloseIE();
	HWND GetWindowHwndByPID(DWORD dwProcessID);

	HWND m_App;
	PROCESS_INFORMATION pi;
};

#endif