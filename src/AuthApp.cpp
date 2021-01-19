#include "AuthApp.h"
#include <windows.h>
#include <tlhelp32.h>
#include <Iepmapi.h>
#include "common.h"

AuthApp::AuthApp()
{
	ZeroMemory(&pi, sizeof(pi));
}

AuthApp::~AuthApp()
{
	CloseIE();
}

///<线程参数
typedef struct
{
	HWND    hwndWindow;     // 窗口句柄
	PROCESS_INFORMATION *pi;    // 进程信息
}ThreadArg;

void ThreadFunc(LPVOID lpThreadParameter)
{
	ThreadArg *pa = (ThreadArg*)lpThreadParameter;
	HANDLE hBaseProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pa->pi->dwProcessId);
	WaitForSingleObject(hBaseProcess, INFINITE);

	TerminateProcess(hBaseProcess, 0);
	CloseHandle(hBaseProcess);

	PostMessage(pa->hwndWindow, WM_COMMAND, ID_FACE_APP_QUIT, 0);

	ZeroMemory(pa->pi, sizeof(PROCESS_INFORMATION));
	free(pa);
}


void AuthApp::OpenIE()
{
	TCHAR chPath[] = TEXT("C:\\Program Files\\Internet Explorer\\iexplore.exe");
	//TCHAR chPath[] = TEXT("C:\\windows\\notepad.exe");

	IELAUNCHURLINFO launchInfo;
	launchInfo.cbSize = sizeof(IELAUNCHURLINFO);
	launchInfo.dwCreationFlags = NULL;

	HRESULT hr = IELaunchURL(TEXT("http://211.166.247.71:8080/cgw-login/success"), &pi, &launchInfo);
	if (SUCCEEDED(hr))
	{
		//WaitForInputIdle(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		DWORD processId = pi.dwProcessId;
		DWORD parentId;
		PROCESSENTRY32 processEntry = { 0 };
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		//给系统内的所有进程拍一个快照
		HANDLE handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		//遍历每个正在运行的进程
		if (Process32First(handleSnap, &processEntry))
		{
			do
			{
				// 找到父进程ID
				if (processEntry.th32ProcessID == processId)
				{
					parentId = processEntry.th32ParentProcessID;
					pi.dwProcessId = parentId;
					break;
				}
			} while (Process32Next(handleSnap, &processEntry));
		}

		DWORD ThreadID;
		ThreadArg *ta = (ThreadArg *)malloc(sizeof(ThreadArg));
		ta->pi = &pi;
		ta->hwndWindow = m_App;
		HANDLE hThread = CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)ThreadFunc,
			(void *)ta,
			0,
			&ThreadID);
		CloseHandle(hThread);
	}
}

///< 枚举窗口参数
typedef struct
{
	HWND    hwndWindow;     // 窗口句柄
	DWORD   dwProcessID;    // 进程ID
}EnumWindowsArg;

///< 枚举窗口回调函数
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	EnumWindowsArg *pArg = (EnumWindowsArg *)lParam;
	DWORD  dwProcessID = 0;
	// 通过窗口句柄取得进程ID
	::GetWindowThreadProcessId(hwnd, &dwProcessID);
	if (dwProcessID == pArg->dwProcessID)
	{
		pArg->hwndWindow = hwnd;
		// 找到了返回FALSE
		return FALSE;
	}
	// 没找到，继续找，返回TRUE
	return TRUE;
}
///< 通过进程ID获取窗口句柄
HWND AuthApp::GetWindowHwndByPID(DWORD dwProcessID)
{
	HWND hwndRet = NULL;
	EnumWindowsArg ewa;
	ewa.dwProcessID = dwProcessID;
	ewa.hwndWindow = NULL;
	EnumWindows(EnumWindowsProc, (LPARAM)&ewa);
	if (ewa.hwndWindow)
	{
		hwndRet = ewa.hwndWindow;
	}
	return hwndRet;
}

BOOL AuthApp::CloseIE()
{
	if (pi.hProcess != NULL)
	{
		DWORD processId = pi.dwProcessId;
		PROCESSENTRY32 processEntry = { 0 };
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		//给系统内的所有进程拍一个快照
		HANDLE handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		//遍历每个正在运行的进程
		if (Process32First(handleSnap, &processEntry))
		{
			BOOL isContinue = TRUE;

			//终止子进程
			do 
			{
				if (processEntry.th32ParentProcessID == processId)
				{
					HANDLE hChildProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processEntry.th32ProcessID);
					if (hChildProcess)
					{
						HWND wnd = GetWindowHwndByPID(pi.dwProcessId);
						PostMessage(wnd, WM_CLOSE, 0, 0);

						//TerminateProcess(hChildProcess, 0);
						//CloseHandle(hChildProcess);
					}
				}
				isContinue = Process32Next(handleSnap, &processEntry);
			} while (isContinue);

			HANDLE hBaseProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
			if (hBaseProcess)
			{
				TerminateProcess(hBaseProcess, 0);
				CloseHandle(hBaseProcess);
			}
		}

		ZeroMemory(&pi, sizeof(pi));
	}

	return TRUE;
}

BOOL AuthApp::AppExited()
{
	return pi.dwProcessId ? FALSE : TRUE ;
}