#ifndef __SAMPLEGRABBERCALLBACK_H__
#define __SAMPLEGRABBERCALLBACK_H__
#include "common.h"
#include "AuthApp.h"

class SampleGrabberCallback : public ISampleGrabberCB
{
public:
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void** ppvObject);

	HRESULT STDMETHODCALLTYPE SampleCB(double Time, IMediaSample *pSample);
	HRESULT STDMETHODCALLTYPE BufferCB(double Time, BYTE *pBuffer, long BufferLen);
	
	SampleGrabberCallback();
	~SampleGrabberCallback();

	BOOL SaveBitmap(BYTE * pBuffer, long lBufferSize ); //����bitmapͼƬ
public:
	BOOL m_bGetPicture;  // is get a picture
	BOOL m_bGetAuthPicture;
	long m_lWidth;       //�洢ͼƬ�Ŀ���
	long m_lHeight;		 //�洢ͼƬ�ĳ���
	int  m_iBitCount;    //the number of bits per pixel (bpp)
	TCHAR m_chTempPath[MAX_PATH];
	TCHAR m_chSwapStr[MAX_PATH];
	TCHAR m_chDirName[MAX_PATH];
	AuthApp  *authapp = NULL;
	HWND m_App;
};

#endif //__SAMPLEGRABBERCALLBACK_H__