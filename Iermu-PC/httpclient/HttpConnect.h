#pragma once
#include "IHttpInterface.h"


//每次读取的字节数
#define READ_BUFFER_SIZE		4096
#define DOWNLOAD_BUFFER_SIZE	4096

#ifndef HINTERNET
#define HINTERNET LPVOID
#endif

using namespace std;

class CHttpConnect:
	public IHttpInterface
{
public:
	CHttpConnect(void);
	virtual ~CHttpConnect(void);
	string	Request(const string& strUrl, REQUEST_TYPE type, const string& strPostData="", string strHeader="");
	string	Request(const wstring& strUrl, REQUEST_TYPE type, const wstring& strPostData=L"", wstring strHeader=L"");
	bool	Download(const string& strUrl, const string& strSavePath="");
	bool	Download(const wstring& strUrl, const wstring& strSavePath=L"");
	void	SetWnd(HWND hWnd) { m_hWnd=hWnd; }
	void	SetMsg(const UINT msg) { m_uMsg=msg; }
	const	wstring&	GetErrorMsg()const { return m_strErrorMsg; }
	const	wchar_t*	GetErrorBuffer()const { return m_strErrorMsg.c_str(); }
	void	FreeInstance();
	bool    GetStatusIsOK();
protected:
	//设置出错信息
	void	SetErrorMsg(const wchar_t* pMsg);
	//从URL中解析出主域名和文件名
	//返回值结果 -1:失败 0：http  1:https
	int	ParseURL(const string& strUrl);
	int	ParseURL(const wstring& strUrl);
	//填充HTTP协议头
	void	GetHttpHeader(string& strHeader);
	void	GetHttpHeader(wstring& strHeader);
	void	GetDownloadHttpHeader(string& strHeader);
	void	GetDownloadHttpHeader(wstring& strHeader);
	//转码
	wstring StringToWstring(const string& str);
	string	WstringToString(const wstring& str);
	wstring Utf8ToUnicode(const string& strUtf8);
	//关闭句柄
	void	ReleaseHandle(HINTERNET& hInternet);
	void	Release();
private:
	HWND	m_hWnd;
	UINT	m_uMsg;
	HINTERNET	m_hSession;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRequest;
	//主域名
	string		m_strHostName;
	wstring		m_wstrHostName;
	//页面名
	string		m_strPageName;
	wstring		m_wstrPageName;
	//错误信息
	wstring		m_strErrorMsg;
	//状态码  1：200 OK  0：其他
	bool		m_ok;
};
