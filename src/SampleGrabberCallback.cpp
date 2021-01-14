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
	//Get template path
	GetTempPath(MAX_PATH,m_chTempPath);
	StringCchCat(m_chTempPath,MAX_PATH,TEXT("CaptureBmp"));
	CreateDirectory(m_chTempPath,NULL);
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

static char *WBSToMBS(TCHAR *uncode)
{
#ifdef _UNICODE
	DWORD num = WideCharToMultiByte(CP_ACP, 0, uncode, -1, NULL, 0, NULL, 0);
	char *pbuf = NULL;
	pbuf = (char*)malloc(num * sizeof(char)) + 1;
	if (pbuf == NULL)
	{
		free(pbuf);
		return false;
	}
	memset(pbuf, 0, num * sizeof(char) + 1);
	WideCharToMultiByte(CP_ACP, 0, uncode, -1, pbuf, num, NULL, 0);
	return pbuf;
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
		StringCchPrintf(m_chDirName, MAX_PATH, TEXT("\\%02i%02i%02ione.bmp"),
			sysTime.wHour,sysTime.wMinute, sysTime.wSecond);
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
	static XHF_SESS sess = NULL;
	if (NULL == sess)
	{
		int FECreateCode = FECreate(&sess, 2);
		if (FECreateCode != XHF_OK)
		{
			std::cout << FECreateCode << "FECreate error" << std::endl << std::flush;
			return FECreateCode;
		}
	}

	char *pauthpath = WBSToMBS(authpath);
	char *pswapstr = WBSToMBS(m_chSwapStr);

	cv::Mat img1 = cv::imread(pauthpath);
	cv::Mat img2 = cv::imread(pswapstr);

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
		std::cout << FECompareFaceCode << "FECompareFace error" << std::endl << std::flush;
		//Msg(NULL, TEXT("相似度小于85%"));
		LockWorkStation();
	}

	return TRUE;
}
