/*
新增可以重定向的HTTP下载库，暂时只支持下载
姚佳宁
2014年10月30日18:05:13
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
//定义HTTP请求头
struct HTTP_HERDER
{
	char szType[5];				//请求类型，GET或者POST
	char szRequest[MAX_PATH];	//请求内容
	char szVersion[9];			//HTTP版本，固定值：HTTP/1.1
	char szAccept[100];			//请求文件类型，默认值为：nAccept: */*
	char szHostName[30];		//服务器域名地址，例如：www.baidu.com
	char szRange[11];			//断点续传起始点，Range: bytes=*-
	char szConnection[11];		//连接方式，默认：Keep-Alive
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
		if ( strlen(szRange)>0 )//加上断点续传信息
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
	//外部接口
	//********************************
	//获取HTTP版本
	const char*		GetHttpVersion()const { return m_szHttpVersion; }
	//获取HTTP服务器返回值
	const int		GetReturnValue()const { return m_uReturnValue; }
	//获取HTTP返回字符
	const char*		GetContent()const	  { return m_strContent.c_str(); }
	//获取某一个键对应的值
	std::string		GetValue(const std::string& strKey);
protected:
	//解析HTTP头结构
	bool	Revolse(const std::string& strHeader);
private:
	//HTTP服务器版本
	char		m_szHttpVersion[9];
	//返回值
	int		m_uReturnValue;
	//返回说明字符串
	std::string	m_strContent;
	//返回的键值对
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