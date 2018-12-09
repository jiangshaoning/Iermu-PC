/*
���������ض����HTTP���ؿ⣬��ʱֻ֧������
Ҧ����
2014��10��30��18:05:13
*/
#pragma once
#include <string>
#include <map>
using std::wstring;
using std::string;
using std::map;
#include "IHttpInterface.h"
#include <WinSock2.h>


#define RECV_BUFFER_SIZE	1024*10
//����HTTP����ͷ
struct HTTP_HERDER
{
	char szType[5];				//�������ͣ�GET����POST
	char szRequest[MAX_PATH];	//��������
	char szVersion[9];			//HTTP�汾���̶�ֵ��HTTP/1.1
	char szAccept[100];			//�����ļ����ͣ�Ĭ��ֵΪ��nAccept: */*
	char szHostName[30];		//������������ַ�����磺www.baidu.com
	char szRange[11];			//�ϵ�������ʼ�㣬Range: bytes=*-
	char szConnection[11];		//���ӷ�ʽ��Ĭ�ϣ�Keep-Alive
	HTTP_HERDER()
	{
		memset(this, 0, sizeof(HTTP_HERDER));
		strcpy_s(szVersion, "HTTP/1.1");
		strcpy_s(szConnection, "Keep-Alive");
	}
	string ToString()
	{
		string strRet;
		strRet.append(szType);
		strRet.append(" ");
		strRet.append(szRequest);
		strRet.append(" ");
		strRet.append(szVersion);
		strRet.append("\r\nAccept: ");
		strRet.append(szAccept);
		strRet.append("\r\nHost: ");
		strRet.append(szHostName);
		if ( strlen(szRange)>0 )//���϶ϵ�������Ϣ
		{
			strRet.append("\r\nRange: bytes=");
			strRet.append(szRange);
			strRet.append("-");
		}
		strRet.append("\r\nConnection: ");
		strRet.append(szConnection);
		strRet.append("\r\n\r\n");
		return strRet;
	}
};


class CHttpHeader
{
public:
	CHttpHeader(const char* pHeader);
	CHttpHeader(const std::string& strHeader);
	virtual ~CHttpHeader(void);
	//********************************
	//�ⲿ�ӿ�
	//********************************
	//��ȡHTTP�汾
	const char*		GetHttpVersion()const { return m_szHttpVersion; }
	//��ȡHTTP����������ֵ
	const int		GetReturnValue()const { return m_uReturnValue; }
	//��ȡHTTP�����ַ�
	const char*		GetContent()const	  { return m_strContent.c_str(); }
	//��ȡĳһ������Ӧ��ֵ
	std::string		GetValue(const std::string& strKey);
protected:
	//����HTTPͷ�ṹ
	bool	Revolse(const std::string& strHeader);
private:
	//HTTP�������汾
	char		m_szHttpVersion[9];
	//����ֵ
	int		m_uReturnValue;
	//����˵���ַ���
	std::string	m_strContent;
	//���صļ�ֵ��
	std::map<std::string, std::string>	m_ValueMap;
};

class CHttpSocket
	: public IHttpInterface2
{
public:
	CHttpSocket();
	~CHttpSocket();
	wstring	GetIpAddr()const { return m_strIpAddr; }
	wstring GetLastError()const { return m_strLastError; }
	void	SetCallback(CDownloadCallback* pCallback);
	bool	DownloadFile(const wstring& strUrl, const wstring& strSavePath);
	//bool	HttpPost(const wstring& strUrl, const wstring& strPostData, string& strResult);
	void	FreeInstance();
protected:
	string	U2A(const wstring& str);
	wstring A2U(const string& str);
	void	ParseUrl(const wstring& strUrl, wstring& strHostName, wstring& strPage, WORD& sPort);
	bool	InitSocket(const string& strHostName, const WORD sPort);
	void	InitRequestHeader(HTTP_HERDER& header, const char* pRequest, REQUEST_TYPE type = GET, \
		const char* pRange=NULL, const char* pAccept="*/*" );
private:
	SOCKET	m_socket;
	wstring	m_strIpAddr;
	wstring m_strLastError;
	CDownloadCallback*	m_pCallback;
};