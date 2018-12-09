#pragma once
#include <string>
using std::string;
using std::wstring;
#include <stdio.h>
#include <tchar.h>


enum REQUEST_TYPE
{
	POST,
	GET
};


////////////////////////////////////////////////////////////////////////////////////
//HTTP����ӿ���
class IHttpInterface
{
public:
	//HTTP������
	virtual string	Request(const string& strUrl, REQUEST_TYPE type, const string& strPostData="", string strHeader="")=0;
	virtual string	Request(const wstring& strUrl, REQUEST_TYPE type, const wstring& strPostData=L"", wstring strHeader=L"")=0;
	virtual bool	Download(const string& strUrl, const string& strSavePath="") = 0;
	virtual bool	Download(const wstring& strUrl,  const wstring& strSavePath = L"") = 0;
	// ������Ϣ����
	virtual void	SetWnd(HWND hWnd)=0;
	virtual void	SetMsg(const UINT msg)=0;
	//������Ϣ��ȡ
	virtual const	wstring&	GetErrorMsg()const=0;
	virtual const	wchar_t*	GetErrorBuffer()const=0;
	//ת�빦��
	virtual wstring StringToWstring(const string& str)=0;
	virtual string	WstringToString(const wstring& str)=0;
	virtual wstring Utf8ToUnicode(const string& strUtf8)=0;
	//�ͷ�
	virtual void	FreeInstance()=0;
};


///////////////////////////////////////////////////////////////////////////////////////
//HTTP socket��

enum DownloadState
{
	DS_Loading=0,
	DS_Fialed,
	DS_Finished,
};

class IHttpInterface2;
//���صĻص�
class CDownloadCallback
{
public:
	virtual void	OnDownloadCallback(IHttpInterface2* pHttpSocket, DownloadState state, int nTotalSize, int nLoadSize)=0;
	virtual bool	IsNeedStop()=0;
};

class IHttpInterface2
{
public:
	virtual wstring	GetIpAddr()const=0;
	virtual wstring GetLastError()const=0;
	virtual void	SetCallback(CDownloadCallback* pCallback)=0;
	virtual bool	DownloadFile(const wstring& strUrl, const wstring& strSavePath)=0;
	virtual void	FreeInstance()=0;
};




/////////////////////////////////////////////////////////////////////////////////
//DLL�ĵ�����������
//#define LIB_DLL
#ifdef EXPORT_LIB//������
	#ifdef LIB_DLL
		#define LIB_FUN extern "C" __declspec(dllexport)
	#else
		#define LIB_FUN
	#endif
#else//���ÿ�
	#ifdef LID_DLL
		#define LIB_FUN extern "C" __declspec(dllimport)
	#else
		#define LIB_FUN
	#endif
#endif


LIB_FUN	IHttpInterface* CreateHttpInstance();//�����ӿ�ʵ������

LIB_FUN bool UrlDownload( const wstring& strUrl, const wstring& strSavePath, OUT UINT& uLoadSize, \
					  OUT wstring& strErrorMsg, HWND hWnd=NULL, UINT uMsg=0 );//�ṩC���غ���

LIB_FUN	IHttpInterface2* CreateHttpInstance2();