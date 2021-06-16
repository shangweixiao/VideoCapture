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

BOOL AuthApp::AppStart()
{
	OpenIE();
	return TRUE;
}

#define INVLAID_PROCESS_ID (-1)
///<�̲߳���
typedef struct
{
	HWND    hwndApp;     // �����򴰿ھ��
	PROCESS_INFORMATION *pi;    //������Ϣ
}ThreadArg;
DWORD GetProcessParentId(DWORD dwProcessID)
{
	DWORD parentId = INVLAID_PROCESS_ID;
	PROCESSENTRY32 processEntry = { 0 };
	processEntry.dwSize = sizeof(PROCESSENTRY32);
	//��ϵͳ�ڵ����н�����һ������
	HANDLE handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	//����ÿ���������еĽ���
	if (Process32First(handleSnap, &processEntry))
	{
		do
		{
			// �ҵ�������ID
			if (processEntry.th32ProcessID == dwProcessID)
			{
				parentId = processEntry.th32ParentProcessID;
				break;
			}
		} while (Process32Next(handleSnap, &processEntry));
	}

	CloseHandle(handleSnap);
	return parentId;
}

void ThreadFunc(LPVOID lpThreadParameter)
{
	ThreadArg *pa = (ThreadArg*)lpThreadParameter;
	DWORD ParentId = GetProcessParentId(pa->pi->dwProcessId); // IE��Chrome���Ƕ��ǩ������������ظ�����
	if (ParentId != INVLAID_PROCESS_ID)
	{
		HANDLE hBaseProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ParentId);
		WaitForSingleObject(hBaseProcess, INFINITE);

		TerminateProcess(hBaseProcess, 0);
		CloseHandle(hBaseProcess);
	}

	PostMessage(pa->hwndApp, WM_COMMAND, ID_FACE_APP_QUIT, 0);
	ZeroMemory(pa->pi, sizeof(PROCESS_INFORMATION));

	free(pa);
}

void AuthApp::OpenIE()
{
	IELAUNCHURLINFO launchInfo;
	launchInfo.cbSize = sizeof(IELAUNCHURLINFO);
	launchInfo.dwCreationFlags = NULL;

	HRESULT hr = IELaunchURL(TEXT("http://192.168.251.212:8080/cgw-login/"), &pi, &launchInfo);
	//HRESULT hr = IELaunchURL(TEXT("https://www.baidu.com/"), &pi, &launchInfo);
	if (SUCCEEDED(hr))
	{
		//WaitForInputIdle(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		DWORD ThreadID;
		ThreadArg *ta = (ThreadArg *)malloc(sizeof(ThreadArg));
		ta->pi = &pi;
		ta->hwndApp = m_App;
		HANDLE hThread = CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)ThreadFunc,
			(void *)ta,
			0,
			&ThreadID);
		CloseHandle(hThread);
	}
}

///< ö�ٴ��ڲ���
typedef struct
{
	HWND    hwndWindow;     // ���ھ��
	DWORD   dwProcessID;    // ����ID
}EnumWindowsArg;

///< ö�ٴ��ڻص�����
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	EnumWindowsArg *pArg = (EnumWindowsArg *)lParam;
	DWORD  dwProcessID = 0;
	// ͨ�����ھ��ȡ�ý���ID
	::GetWindowThreadProcessId(hwnd, &dwProcessID);
	if (dwProcessID == pArg->dwProcessID)
	{
		pArg->hwndWindow = hwnd;
		// �ҵ��˷���FALSE
		return FALSE;
	}
	// û�ҵ��������ң�����TRUE
	return TRUE;
}
///< ͨ������ID��ȡ���ھ��
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
		DWORD ParentId = GetProcessParentId(pi.dwProcessId);
		PROCESSENTRY32 processEntry = { 0 };
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		//��ϵͳ�ڵ����н�����һ������
		HANDLE handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		//����ÿ���������еĽ���
		if (Process32First(handleSnap, &processEntry))
		{
			BOOL isContinue = TRUE;

			//��ֹ�ӽ���
			do 
			{
				if (processEntry.th32ParentProcessID == ParentId)
				{
					HANDLE hChildProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processEntry.th32ProcessID);
					if (hChildProcess)
					{
						HWND wnd = GetWindowHwndByPID(processEntry.th32ProcessID);
						PostMessage(wnd, WM_CLOSE, 0, 0);

						//TerminateProcess(hChildProcess, 0);
						//CloseHandle(hChildProcess);
					}
				}
				isContinue = Process32Next(handleSnap, &processEntry);
			} while (isContinue);

			HANDLE hBaseProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ParentId);
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

void AuthApp::OpenChrome()
{
	BOOL bRet;
	TCHAR url[] = TEXT(" -- http://192.168.251.212:8080/cgw-login/");
	TCHAR chromex86[] = TEXT("C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe");
	TCHAR chromex64[] = TEXT("C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe");
	TCHAR chromeuser[MAX_PATH] = {0};
	DWORD ret = GetEnvironmentVariable(TEXT("LOCALAPPDATA"), chromeuser, sizeof(chromeuser));
	if (ret)
	{
		wcscat(chromeuser,TEXT("\\Google\\Chrome\\Application\\chrome.exe"));
	}

	STARTUPINFO si;
	ZeroMemory(&si,sizeof(si));
	si.cb = sizeof(si);

	bRet = CreateProcess(chromex86, url, NULL, NULL,FALSE, 0, NULL, NULL, &si, &pi);
	if (!bRet)
	{
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		bRet = CreateProcess(chromex64, url, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}

	if (!bRet)
	{
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		bRet = CreateProcess(chromeuser, url, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}

	if (bRet)
	{
		DWORD ThreadID;
		ThreadArg *ta = (ThreadArg *)malloc(sizeof(ThreadArg));
		ta->pi = &pi;
		ta->hwndApp = m_App;
		HANDLE hThread = CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)ThreadFunc,
			(void *)ta,
			0,
			&ThreadID);
		CloseHandle(hThread);
	}
}

BOOL AuthApp::CloseChrome()
{
	return CloseIE();
}

BOOL AuthApp::AppExited()
{
	return pi.dwProcessId ? FALSE : TRUE ;
}