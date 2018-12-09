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
//HTTP请求接口类
class IHttpInterface
{
public:
	//HTTP请求功能
	virtual string	Request(const string& strUrl, REQUEST_TYPE type, const string& strPostData="", string strHeader="")=0;
	virtual string	Request(const wstring& strUrl, REQUEST_TYPE type, const wstring& strPostData=L"", wstring strHeader=L"")=0;
	virtual bool	Download(const string& strUrl, const string& strSavePath="") = 0;
	virtual bool	Download(const wstring& strUrl,  const wstring& strSavePath = L"") = 0;
	// 下载消息设置
	virtual void	SetWnd(HWND hWnd)=0;
	virtual void	SetMsg(const UINT msg)=0;
	//出错信息获取
	virtual const	wstring&	GetErrorMsg()const=0;
	virtual const	wchar_t*	GetErrorBuffer()const=0;
	//转码功能
	virtual wstring StringToWstring(const string& str)=0;
	virtual string	WstringToString(const wstring& str)=0;
	virtual wstring Utf8ToUnicode(const string& strUtf8)=0;
	//释放
	virtual void	FreeInstance()=0;
};


///////////////////////////////////////////////////////////////////////////////////////
//HTTP socket类

enum DownloadState
{
	DS_Loading=0,
	DS_Fialed,
	DS_Finished,
};

class IHttpInterface2;
//下载的回调
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
//DLL的导出函数声明
//#define LIB_DLL
#ifdef EXPORT_LIB//导出库
	#ifdef LIB_DLL
		#define LIB_FUN extern "C" __declspec(dllexport)
	#else
		#define LIB_FUN
	#endif
#else//引用库
	#ifdef LID_DLL
		#define LIB_FUN extern "C" __declspec(dllimport)
	#else
		#define LIB_FUN
	#endif
#endif


LIB_FUN	IHttpInterface* CreateHttpInstance();//创建接口实例对象

LIB_FUN bool UrlDownload( const wstring& strUrl, const wstring& strSavePath, OUT UINT& uLoadSize, \
					  OUT wstring& strErrorMsg, HWND hWnd=NULL, UINT uMsg=0 );//提供C下载函数

LIB_FUN	IHttpInterface2* CreateHttpInstance2();