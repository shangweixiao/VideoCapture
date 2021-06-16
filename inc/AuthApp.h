#ifndef __AUTH_APP__
#define __AUTH_APP__
#include "windows.h"

class AuthApp {
public:
	AuthApp();
	~AuthApp();
public:
	HWND m_App;
	BOOL AppStart();
	BOOL AppExited();

private:
	void OpenIE();
	BOOL  CloseIE();
	void OpenChrome();
	BOOL  CloseChrome();
	HWND GetWindowHwndByPID(DWORD dwProcessID);

	PROCESS_INFORMATION pi;
};

#endif