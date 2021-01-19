#include "CaptureVideo.h"
#include "CaptureAudio.h"
#include "resource.h"

#include "xhfacelite_type.h"
#include "xhfacelite_api.h"


#define ID_COMBOBOX  10000
#define ID_COMBOBOX2 10001
#define ID_TIMER     10002

CaptureVideo g_CaptureVideo;

//CaptureAudio g_CaptureAudio;
HWND hwndImg;
HBITMAP hBitmap;
HWND hwndText;
HWND hwndCombo1;
//HWND hwndCombo2;
BSTR bstrDeviceName;

HICON g_hIconLarge;
HICON g_hIconSmall;

int g_nTimerCount = 0;

INT_PTR CALLBACK WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID CALLBACK TimerGetPicture(HWND hDlg, UINT message, UINT_PTR iTimerID, DWORD dwTimer);
VOID CALLBACK TimerSetAuthPicture(HWND hDlg, UINT message, UINT_PTR iTimerID, DWORD dwTimer);
VOID SetWindowPosCenter(HWND hDlg);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{


	g_hIconLarge = static_cast<HICON>(LoadImage(hInstance, TEXT("IDI_ICON1"), IMAGE_ICON,  //set large ico
		GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CXICON), 0));
	g_hIconSmall = static_cast<HICON>(LoadImage(hInstance, TEXT("IDI_ICON1"), IMAGE_ICON,   //set small ico
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CXSMICON), 0));

    MSG msg;
	HWND hDlg = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_DIALOG1),NULL, WndProc);
	
	//SetWindowPos(hDlg,NULL,0,0,455,600, SWP_NOMOVE);
	//SetDialogBkColor();
	ShowWindow(hDlg,iCmdShow);
	UpdateWindow(hDlg);

	TCHAR DirectoryBuffer[1024] = { '\0' };
	GetCurrentDirectory(1024, DirectoryBuffer);
	//Msg(hDlg,DirectoryBuffer);

	// 初始化人脸识别库
	int XHFInitCode = XHFInit("./models", "sparkAI_facetest_2021");
	if (XHFInitCode != XHF_OK)
	{
		Msg(hDlg, TEXT("错误"), TEXT("人脸识别库初始化错误。错误码：0x%X"), XHFFinal);
	}

	hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
	SendMessage(hwndImg, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);

	{
		TCHAR TempPath[MAX_PATH] = { 0 };
		GetTempPath(MAX_PATH, TempPath);
		StringCchCat(TempPath, MAX_PATH, TEXT("CaptureBmp\\videoauth.bmp"));

		WIN32_FIND_DATA  wfd;
		BOOL rValue = FALSE;
		HANDLE hFind = FindFirstFile(TempPath, &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			EnableWindow(GetDlgItem(hDlg, IDONESHOT), FALSE);
			Msg(hDlg, TEXT("提示"), TEXT("未发现认证照片，请点击设置按钮设置您的认证照片"));
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg, IDC_PREVIWE), FALSE);
		}
		FindClose(hFind);
	}

    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }

	CoUninitialize();
	// Exit
	DestroyIcon(g_hIconLarge);
	DestroyIcon(g_hIconSmall);	
    return (int)msg.wParam;
}

INT_PTR CALLBACK WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int nGetComboxCount = 0;
	int iGetCurSel = 0;
	DWORD dwBaseUnits;
	switch(message)
	{
	case WM_INITDIALOG:
        {
			SendMessage(hDlg,WM_SETICON,FALSE,reinterpret_cast<LPARAM>(g_hIconSmall));
			SendMessage(hDlg,WM_SETICON,TRUE,reinterpret_cast<LPARAM>(g_hIconLarge));
			//////////////////////////////////////////////////////////////////////////
			SetWindowPosCenter(hDlg); //set Dialog at window center
			//////////////////////////////////////////////////////////////////////////
			g_CaptureVideo.m_App = hDlg;
			//SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 450, 600, SWP_NOMOVE);
			//g_CaptureAudio.m_App = hDlg;
			dwBaseUnits = GetDialogBaseUnits(); 
			//Msg(hDlg,TEXT("提示"),TEXT("width = %d,height = %d"), LOWORD(dwBaseUnits), HIWORD(dwBaseUnits));
			
			hwndImg = CreateWindow(TEXT("STATIC"), NULL,
				WS_VISIBLE | WS_CHILD | SS_BITMAP,
				10, //(6 * LOWORD(dwBaseUnits)) / 4, 
				0, //(2 * HIWORD(dwBaseUnits)) / 8, 
				330,
				120,
				hDlg, (HMENU)10004, NULL, NULL);

			hwndText = CreateWindow(TEXT("STATIC"), TEXT(""),
				WS_VISIBLE | WS_CHILD | SS_LEFT,
				10, //(6 * LOWORD(dwBaseUnits)) / 4, 
				123, //(2 * HIWORD(dwBaseUnits)) / 8, 
				(100 * LOWORD(dwBaseUnits)) / 4,
				(50 * HIWORD(dwBaseUnits)) / 8,
				hDlg, (HMENU)10005, NULL, NULL);
			SetWindowText(hwndText, TEXT("摄像头："));

			hwndCombo1 = CreateWindow(TEXT("COMBOBOX"), TEXT(""), 
				CBS_DROPDOWN | WS_CHILD | WS_VISIBLE, 
				70, //(6 * LOWORD(dwBaseUnits)) / 4, 
				120, //(2 * HIWORD(dwBaseUnits)) / 8, 
				(100 * LOWORD(dwBaseUnits)) / 4, 
				(50 * HIWORD(dwBaseUnits)) / 8, 
				hDlg, (HMENU)ID_COMBOBOX, NULL, NULL); 

			//Video
			g_CaptureVideo.EnumAllDevices(hwndCombo1); //Enum All Camera
			nGetComboxCount = ComboBox_GetCount(hwndCombo1);
			if (nGetComboxCount == 0)
				ComboBox_Enable(hwndCombo1,FALSE);
			else
				ComboBox_SetCurSel(hwndCombo1,0);

			if(g_CaptureVideo.m_nCaptureDeviceNumber == 0)
			{
				Msg(hDlg, TEXT("提示"), TEXT("没有发现摄像头设备"));
				EnableWindow(GetDlgItem(hDlg,IDC_PREVIWE),FALSE);
				EnableWindow(GetDlgItem(hDlg, IDONESHOT), FALSE);
			}
			else
			{
				iGetCurSel = ComboBox_GetCurSel(hwndCombo1);
				g_CaptureVideo.OpenDevice(iGetCurSel, 10, 150, 418, 330);
				EnableWindow(GetDlgItem(hDlg, IDONESHOT), TRUE);
			}
        }
		return TRUE;
	case WM_DESTROY:
        {
			g_CaptureVideo.CloseInterface();
			XHFFinal();
			PostQuitMessage(0);
        }
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
            {
				XHFFinal();
                PostQuitMessage(0);
            }
           break;
		case IDONESHOT: // 认证按钮
            {
				MessageBox(hDlg, TEXT("刷脸认证准备中...，使用过程中请保持正脸面向摄像头。"), TEXT("提示"), MB_OK);
                //g_CaptureVideo.GrabOneFrame(TRUE);
				SetTimer(hDlg,ID_TIMER,50, TimerGetPicture);
				EnableWindow(GetDlgItem(hDlg, IDC_PREVIWE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDONESHOT), FALSE);
            }
			break;
		case IDC_PREVIWE: // 设置按钮，设置初始对比图像
			{
				MessageBox(hDlg, TEXT("准备生成认证照片，请正脸面向摄像头。"),TEXT("提示"),MB_OK);
				EnableWindow(GetDlgItem(hDlg, IDC_PREVIWE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDONESHOT), TRUE);
				// 设置认证图像
				SetTimer(hDlg, ID_TIMER, 50, TimerSetAuthPicture);
			}
			break;
		case ID_FACE_DETECT_FAIL:
			g_CaptureVideo.GrabOneFrame(FALSE);
			ShowWindow(hDlg, SW_NORMAL);
			EnableWindow(GetDlgItem(hDlg, IDONESHOT), TRUE);
			KillTimer(hDlg, ID_TIMER);
			Msg(hDlg, TEXT("提示"), TEXT("人脸比对检测失败。"));
			break;
		case ID_FACE_APP_QUIT:
			g_CaptureVideo.GrabOneFrame(FALSE);
			ShowWindow(hDlg, SW_NORMAL);
			EnableWindow(GetDlgItem(hDlg, IDONESHOT), TRUE);
			KillTimer(hDlg, ID_TIMER);
			Msg(hDlg, TEXT("提示"), TEXT("浏览器已退出。"));
			break;
		case ID_FACE_DETECT_SUCCESS:
			ShowWindow(hDlg, SW_MINIMIZE);
			break;
		default:
			break;
		}
	case WM_MOVE:
		g_CaptureVideo.m_pVideoWindow->NotifyOwnerMessage((OAHWND)hDlg, message, wParam, lParam);
		break;
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORDLG:
		{
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}
	}
	return (FALSE);
}

VOID CALLBACK TimerGetPicture(HWND hDlg, UINT message, UINT_PTR iTimerID, DWORD dwTimer)
{
	g_CaptureVideo.GrabOneFrame(TRUE);
	SetTimer(hDlg, ID_TIMER, 3*1000, TimerGetPicture);
	g_nTimerCount++;
}

VOID CALLBACK TimerSetAuthPicture(HWND hDlg, UINT message, UINT_PTR iTimerID, DWORD dwTimer)
{
	g_CaptureVideo.GrabAuthFrame(TRUE);
	KillTimer(hDlg, ID_TIMER);
	Msg(hDlg, TEXT("提示"), TEXT("已生成您的认证照片，浏览器将只有您本人在摄像头前时可以正常使用，您离开后浏览器将自动关闭。\n"
		"现在您可以点击“认证”按钮开始使用。"));
}

VOID SetWindowPosCenter(HWND hDlg)
{
	int cxWindow,cyWindow;  //window Screen width and height
	RECT hDlgRect;          //Dialog Rect
	int cxDialog,cyDialog;  //Dialog Screen width and height
	int cxSetPos,cySetPos;

	GetWindowRect(hDlg,&hDlgRect);
	//
	cxDialog = hDlgRect.right - hDlgRect.left;
	cyDialog = hDlgRect.bottom - hDlgRect.top;
	//
	cxWindow = GetSystemMetrics(SM_CXSCREEN);
	cyWindow = GetSystemMetrics(SM_CYSCREEN);
	//
	cxSetPos = (cxWindow-cxDialog)/2;
	cySetPos = (cyWindow-cyDialog)/2;

	SetWindowPos(hDlg,NULL,cxSetPos,cySetPos,0,0,SWP_NOSIZE);
}