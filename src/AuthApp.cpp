#include "AuthApp.h"
#include <windows.h>
#include <tlhelp32.h>

AuthApp::AuthApp()
{
	ZeroMemory(&pi, sizeof(pi));
}

AuthApp::~AuthApp()
{
	CloseIE();
}

void AuthApp::OpenIE()
{
	TCHAR chPath[] = TEXT("C:\\Program Files (x86)\\Internet Explorer\\iexplore.exe");

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	GetStartupInfo(&si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	// Start the child process
	if (!CreateProcess(chPath, TEXT("open http://www.baidu.com/"), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		ZeroMemory(&pi, sizeof(pi));
	}
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
		if (Process32First(handleSnap, &processEntry)) {
			BOOL isContinue = TRUE;

			//终止子进程
			do {
				if (processEntry.th32ParentProcessID == processId) {
					HANDLE hChildProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processEntry.th32ProcessID);
					if (hChildProcess) {
						TerminateProcess(hChildProcess, 0);
						CloseHandle(hChildProcess);
					}
				}
				isContinue = Process32Next(handleSnap, &processEntry);
			} while (isContinue);

			HANDLE hBaseProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
			if (hBaseProcess) {
				TerminateProcess(hBaseProcess, 0);
				CloseHandle(hBaseProcess);
			}
		}

		ZeroMemory(&pi, sizeof(pi));
	}

	return TRUE;
}