#include "SampleGrabberCallback.h"
#include "ImageFormatConversion.h"
#include "xhfacelite_type.h"
#include "xhfacelite_api.h"
#include <iostream>
#include <atlimage.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

SampleGrabberCallback::SampleGrabberCallback()
{
	m_bGetPicture = FALSE;
	authapp = NULL;
	//Get template path
	GetTempPath(MAX_PATH,m_chTempPath);
	StringCchCat(m_chTempPath,MAX_PATH,TEXT("CaptureBmp"));
	CreateDirectory(m_chTempPath,NULL);
}

SampleGrabberCallback::~SampleGrabberCallback()
{
	if (NULL != authapp)
	{
		delete authapp;
	}
}

ULONG STDMETHODCALLTYPE SampleGrabberCallback::AddRef()
{
	return 1;
}

ULONG STDMETHODCALLTYPE SampleGrabberCallback::Release()
{
	return 2;
}

HRESULT STDMETHODCALLTYPE SampleGrabberCallback::QueryInterface(REFIID riid,void** ppvObject)
{
	if (NULL == ppvObject) return E_POINTER;
	if (riid == __uuidof(IUnknown))
	{
		*ppvObject = static_cast<IUnknown*>(this);
		return S_OK;
	}
	if (riid == IID_ISampleGrabberCB)
	{
		*ppvObject = static_cast<ISampleGrabberCB*>(this);
		return S_OK;
	}
	return E_NOTIMPL;

}

HRESULT STDMETHODCALLTYPE SampleGrabberCallback::SampleCB(double Time, IMediaSample *pSample)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE SampleGrabberCallback::BufferCB(double Time, BYTE *pBuffer, long BufferLen)
{
	if(FALSE == m_bGetPicture)  //判断是否需要截图
		return S_FALSE;
	if(!pBuffer)
		return E_POINTER;

	SaveBitmap(pBuffer,BufferLen);

	m_bGetPicture = FALSE;
	return S_OK;
}

static char *WBSToMBS(TCHAR *uncode,char mbs[MAX_PATH * 2])
{
#ifdef _UNICODE
	DWORD num = WideCharToMultiByte(CP_ACP, 0, uncode, -1, NULL, 0, NULL, 0);
	memset(mbs, 0, MAX_PATH * 2);
	WideCharToMultiByte(CP_ACP, 0, uncode, -1, mbs, num, NULL, 0);
	return mbs;
#else
	return uncode;
#endif
}
BOOL SampleGrabberCallback::SaveBitmap(BYTE * pBuffer, long lBufferSize )
{
	TCHAR authpath[MAX_PATH] = { 0 };

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	StringCchCopy(m_chSwapStr,MAX_PATH,m_chTempPath);
	if (m_bGetAuthPicture)
	{
		StringCchPrintf(m_chDirName, MAX_PATH, TEXT("\\videoauth.bmp"));
	}
	else
	{
		StringCchPrintf(m_chDirName, MAX_PATH, TEXT("\\%02i%02ione.bmp"),sysTime.wMinute, sysTime.wSecond);
	}

	StringCchCat(authpath, MAX_PATH, m_chSwapStr);
	StringCchCat(authpath, MAX_PATH, TEXT("\\videoauth.bmp"));

	StringCchCat(m_chSwapStr,MAX_PATH,m_chDirName);
	// %temp%/CaptureBmp/*
	//MessageBox(NULL,chTempPath,TEXT("Message"),MB_OK);
	//create picture file
	HANDLE hf = CreateFile(m_chSwapStr,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,
		CREATE_ALWAYS,0,NULL);
	if(hf == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	BITMAPFILEHEADER bfh;  //Set bitmap header
	ZeroMemory(&bfh,sizeof(bfh));
	bfh.bfType = 'MB';
	bfh.bfSize = sizeof(bfh) + lBufferSize + sizeof(BITMAPFILEHEADER);
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPFILEHEADER);
	// Write the file header.
	DWORD dwWritten = 0;
	WriteFile( hf, &bfh, sizeof( bfh ), &dwWritten, NULL );    
	// Write the file Format
	BITMAPINFOHEADER bih;
	ZeroMemory(&bih,sizeof(bih));
	bih.biSize = sizeof( bih );
	bih.biWidth = m_lWidth;
	bih.biHeight = m_lHeight;
	bih.biPlanes = 1;
	bih.biBitCount = m_iBitCount;  //Specifies the number of bits per pixel (bpp)
	WriteFile( hf, &bih, sizeof( bih ), &dwWritten, NULL );
	//Write the file Data
	WriteFile( hf, pBuffer, lBufferSize, &dwWritten, NULL );
	CloseHandle( hf );

	if (m_bGetAuthPicture)
		return TRUE;

	// 人脸对比
	XHF_SESS sess;
	int FECreateCode = FECreate(&sess);
	if (FECreateCode != XHF_OK)
	{
		std::cout << FECreateCode << "FECreate error" << std::endl << std::flush;
		return FECreateCode;
	}

	char mbs_ap[MAX_PATH * 2] = {0};
	char mbs_ss[MAX_PATH * 2] = { 0 };

	WBSToMBS(authpath, mbs_ap);
	WBSToMBS(m_chSwapStr, mbs_ss);

	cv::Mat img1 = cv::imread(mbs_ap);
	cv::Mat img2 = cv::imread(mbs_ss);

	float score = -1.0f;
	int FECompareFaceCode = FECompareFace(sess, img1.data, img1.cols, img1.rows,
		img2.data, img2.cols, img2.rows, &score);
	if (FECompareFaceCode != XHF_OK)
	{
		std::cout << FECompareFaceCode << "FECompareFace error" << std::endl <<std::flush;
		//Msg(NULL, TEXT("没有检测到人脸"));
		//LockWorkStation();
	}

	if (score < 0.85)
	{
		//LockWorkStation();
		if (NULL != authapp)
		{
			authapp->CloseIE();
			delete authapp;
			authapp = NULL;
		}

		PostMessage(m_App, WM_COMMAND, ID_FACE_DETECT_FAIL, 0);
	}
	else
	{
		if (NULL != authapp)
		{
			authapp->CloseIE();
			delete authapp;
		}

		authapp = new AuthApp();
		authapp->m_App = m_App;
		authapp->OpenIE();
	}

	FEDestroy(&sess);

	return TRUE;
}
