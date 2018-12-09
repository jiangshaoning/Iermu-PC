#pragma once
#include "IHttpInterface.h"


//ÿ�ζ�ȡ���ֽ���
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
	//���ó�����Ϣ
	void	SetErrorMsg(const wchar_t* pMsg);
	//��URL�н��������������ļ���
	//����ֵ��� -1:ʧ�� 0��http  1:https
	int	ParseURL(const string& strUrl);
	int	ParseURL(const wstring& strUrl);
	//���HTTPЭ��ͷ
	void	GetHttpHeader(string& strHeader);
	void	GetHttpHeader(wstring& strHeader);
	void	GetDownloadHttpHeader(string& strHeader);
	void	GetDownloadHttpHeader(wstring& strHeader);
	//ת��
	wstring StringToWstring(const string& str);
	string	WstringToString(const wstring& str);
	wstring Utf8ToUnicode(const string& strUtf8);
	//�رվ��
	void	ReleaseHandle(HINTERNET& hInternet);
	void	Release();
private:
	HWND	m_hWnd;
	UINT	m_uMsg;
	HINTERNET	m_hSession;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRequest;
	//������
	string		m_strHostName;
	wstring		m_wstrHostName;
	//ҳ����
	string		m_strPageName;
	wstring		m_wstrPageName;
	//������Ϣ
	wstring		m_strErrorMsg;
	//״̬��  1��200 OK  0������
	bool		m_ok;
};
