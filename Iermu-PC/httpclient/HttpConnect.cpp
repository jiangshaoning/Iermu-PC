#include "StdAfx.h"
#include "HttpConnect.h"
#include <Wininet.h>
#pragma comment(lib, "Wininet")


CHttpConnect::CHttpConnect(void)
:m_strHostName("")
,m_wstrHostName(L"")
,m_strPageName("")
,m_wstrPageName(L"")
,m_hSession(NULL)
,m_hConnect(NULL)
,m_hRequest(NULL)
,m_hWnd(NULL)
,m_uMsg(0)
{
}

CHttpConnect::~CHttpConnect(void)
{
	Release();
}

bool CHttpConnect::GetStatusIsOK()
{
	return m_ok;
}

string CHttpConnect::Request(const string& strUrl, REQUEST_TYPE type, const string& strPostData/*=""*/, string strHeader/*=""*/)
{
	m_ok = false;
	string strRet = "";
	string strRetHeader = "";

	if (strUrl.empty())
		return "";
	Release();
	m_hSession = InternetOpen(L"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.75 Safari/537.36", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	if (NULL == m_hSession)
	{
		SetErrorMsg(L"初始化HTTP接口失败");
		goto end;
	}
	int pRet = ParseURL(strUrl);
	INTERNET_PORT port = pRet > 0 ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

	int nPos = m_strHostName.find(":");
	if (string::npos != nPos)
	{
		string strPort = m_strHostName.substr(nPos + 1);
		m_strHostName = m_strHostName.substr(0, nPos);
		m_strHostName = "www.baidu.com";
		port = atoi(strPort.c_str());
	}
	m_hConnect = InternetConnectA(m_hSession, m_strHostName.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	if (NULL == m_hConnect)
	{
		SetErrorMsg(L"连接HTTP服务器失败");
		goto end;
	}
	char* pRequestType = NULL;
	if (GET == type)
		pRequestType = "GET";
	else
		pRequestType = "POST";
	DWORD flag = INTERNET_FLAG_RELOAD | INTERNET_COOKIE_THIRD_PARTY | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;
	m_hRequest = HttpOpenRequestA(m_hConnect, pRequestType, m_strPageName.c_str(), "HTTP/1.1", NULL, NULL, INTERNET_FLAG_SECURE, NULL);
	if ( NULL == m_hRequest )
	{
		SetErrorMsg(L"初始化HTTP请求失败");
		goto end;
	}
	if ( strHeader.empty() )
		GetHttpHeader(strHeader);
	BOOL bRet=FALSE;
	if (GET == type)
		bRet=HttpSendRequestA(m_hRequest, strHeader.c_str(), strHeader.size(), NULL, 0);
	else
		bRet=HttpSendRequestA(m_hRequest, strHeader.c_str(), strHeader.size(), (LPVOID)strPostData.c_str(), strPostData.size());
	if ( !bRet )
	{
		SetErrorMsg(L"发送HTTP请求失败");
		goto end;
	}
	char szBuffer[READ_BUFFER_SIZE+1]={0};
	DWORD dwReadSize=READ_BUFFER_SIZE;
	bRet=HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL);
	if ( !bRet )
	{
		SetErrorMsg(L"查询HTTP返回头失败");
		goto end;
	}
	strRetHeader=szBuffer;
	if (string::npos != strRetHeader.find("200"))
	{
		m_ok = true;
	}
	if ( string::npos != strRetHeader.find("404") )//页面不存在
	{
		SetErrorMsg(L"请求页面不存在");
		goto end;
	}

	while( true )
	{
		bRet=InternetReadFile(m_hRequest, szBuffer, READ_BUFFER_SIZE, &dwReadSize);
		if ( !bRet || (0 == dwReadSize) )
			break;
		szBuffer[dwReadSize]='\0';
		strRet.append(szBuffer);
	}
end:
	Release();
	return strRet;
}

string CHttpConnect::Request( const wstring& strUrl, REQUEST_TYPE type, const wstring& strPostData/*=L""*/, wstring strHeader/*=L""*/ )
{
	string strRet = "";
	if ( strUrl.empty() )
		return "";
	Release();
	m_hSession=InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	if ( NULL == m_hSession )
	{
		SetErrorMsg(L"初始化HTTP接口失败");
		goto end;
	}
	int pRet = ParseURL(strUrl);
	INTERNET_PORT port = pRet > 0 ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

	int nPos=m_wstrHostName.find(L":");
	if ( wstring::npos != nPos )
	{
		wstring strPort=m_wstrHostName.substr(nPos+1);
		m_wstrHostName=m_wstrHostName.substr(0, nPos);
		port=_wtoi(strPort.c_str());
	}
	m_hConnect=InternetConnectW(m_hSession, m_wstrHostName.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	if ( NULL == m_hConnect )
	{
		SetErrorMsg(L"连接HTTP服务器失败");
		goto end;
	}
	wchar_t* pRequestType=NULL;
	if ( GET == type )
		pRequestType=L"GET";
	else
		pRequestType=L"POST";
	m_hRequest=HttpOpenRequestW(m_hConnect, pRequestType, m_wstrPageName.c_str(), L"HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, NULL);
	if ( NULL == m_hRequest )
	{
		SetErrorMsg(L"初始化HTTP请求失败");
		goto end;
	}
	if ( strHeader.empty() )
		GetHttpHeader(strHeader);
	BOOL bRet=FALSE;
	if (GET == type)
		bRet=HttpSendRequestW(m_hRequest, strHeader.c_str(), strHeader.size(), NULL, 0);
	else
	{
		string str=WstringToString(strPostData);
		bRet=HttpSendRequestW(m_hRequest, strHeader.c_str(), strHeader.size(), (LPVOID)str.c_str(), str.size());
	}
	if ( !bRet )
	{
		SetErrorMsg(L"发送HTTP请求失败");
		goto end;
	}
	DWORD dwReadSize=READ_BUFFER_SIZE;
	{
		wchar_t szBuffer[READ_BUFFER_SIZE+1]={0};
		memset(szBuffer, 0, READ_BUFFER_SIZE);
		bRet=HttpQueryInfoW(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL);
		if ( !bRet )
		{
			SetErrorMsg(L"查询HTTP返回头失败");
			goto end;
		}
		wstring strRetHeader=szBuffer;
		if ( wstring::npos != strRetHeader.find(L"404") )//页面不存在
		{
			SetErrorMsg(L"请求页面不存在");
			goto end;
		}
	}

	char szBuffer[READ_BUFFER_SIZE+1]={0};
	while( true )
	{
		bRet=InternetReadFile(m_hRequest, szBuffer, READ_BUFFER_SIZE, &dwReadSize);
		if ( !bRet || (0 == dwReadSize) )
			break;
		szBuffer[dwReadSize]='\0';
		strRet.append(szBuffer);
	}
end:
	Release();
	return strRet;
}

int CHttpConnect::ParseURL( const string& strUrl)
{
	int ret = -1;
	string strTemp=strUrl;
	int nPos=strTemp.find("https://");
	int offset = 8;
	if (string::npos != nPos)
	{
		strTemp = strTemp.substr(nPos + offset, strTemp.size() - nPos - offset);
		nPos = strTemp.find('/');
		ret = 1;
	}

	if ( string::npos == nPos )//没有找到 /
	{
		nPos = strTemp.find("http://");
		int offset = 7;
		if (string::npos != nPos)
			strTemp = strTemp.substr(nPos + offset, strTemp.size() - nPos - offset);
		nPos = strTemp.find('/');
		if (string::npos == nPos)
		{
			m_strHostName = strTemp;
			return ret;
		}
		ret = 1;
	}
	m_strHostName=strTemp.substr(0, nPos);
	m_strPageName=strTemp.substr(nPos, strTemp.size()-nPos);
	return ret;
}

int CHttpConnect::ParseURL( const wstring& strUrl )
{
	int ret = -1;
	wstring strTemp=strUrl;
	int nPos = strTemp.find(L"https://");
	int offset = 8;
	if (string::npos != nPos)
	{
		strTemp = strTemp.substr(nPos + offset, strTemp.size() - nPos - offset);
		nPos = strTemp.find(L"/");
		ret = 1;
	}

	if (string::npos == nPos)//没有找到 /
	{
		nPos = strTemp.find(L"http://");
		int offset = 7;
		if (wstring::npos != nPos)
			strTemp = strTemp.substr(nPos + offset, strTemp.size() - nPos - offset);
		nPos = strTemp.find(L"/");
		if (wstring::npos == nPos)//没有找到 /
		{
			m_wstrHostName = strTemp;
			return ret;
		}
		ret = 0;
	}
	m_wstrHostName=strTemp.substr(0, nPos);
	m_wstrPageName=strTemp.substr(nPos, strTemp.size()-nPos);
	return ret;
}

void CHttpConnect::GetHttpHeader( string& strHeader )
{
	strHeader="";
	strHeader.append("Host: ");
	strHeader+=m_strHostName;
	strHeader.append("\r\nContent-Type: application/x-www-form-urlencoded\r\n");
	strHeader.append("Accept: */*\r\n");
	strHeader.append("Connection: Keep-Alive\r\n\r\n");
}

void CHttpConnect::GetHttpHeader( wstring& strHeader )
{
	strHeader=L"";
	strHeader.append(L"nReferer: ");
	strHeader+=m_wstrHostName;
	strHeader.append(L"\r\nContent-Type: application/x-www-form-urlencoded\r\n");
	strHeader.append(L"Accept: */*\r\n");
	strHeader.append(L"Connection: Keep-Alive\r\n\r\n");
}

void CHttpConnect::GetDownloadHttpHeader(string& strHeader)
{
	strHeader = "";
	strHeader.append("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n");
	strHeader.append("Accept-Encoding: gzip, deflate\r\n");
	strHeader.append("Accept-Language: zh-CN,zh;q=0.8\r\n");
	strHeader.append("Cache-Control: max-age=0\r\n");
	strHeader.append("Connection: Keep-Alive\r\n");
	strHeader.append("Host: ");
	strHeader += m_strHostName;
	strHeader.append("\r\nUpgrade-Insecure-Requests: 1\r\n");
	strHeader.append("User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Safari/537.36\r\n");
}

void CHttpConnect::GetDownloadHttpHeader(wstring& strHeader)
{
	strHeader=L"";
	strHeader.append(L"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n");
	strHeader.append(L"Accept-Encoding: gzip, deflate\r\n");
	strHeader.append(L"Accept-Language: zh-CN,zh;q=0.8\r\n");
	strHeader.append(L"Cache-Control: max-age=0\r\n");
	strHeader.append(L"Connection: Keep-Alive\r\n");
	strHeader.append(L"Host: ");
	strHeader += m_wstrHostName;	
	strHeader.append(L"\r\nUpgrade-Insecure-Requests: 1\r\n");
	strHeader.append(L"User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Safari/537.36\r\n");

}

void CHttpConnect::SetErrorMsg( const wchar_t* pMsg )
{
#ifdef _DEBUG
	m_strErrorMsg=pMsg;
	wchar_t szBuffer[20]={0};
	wsprintf(szBuffer, L",系统错误码：%u", GetLastError());
	m_strErrorMsg.append(szBuffer);
#else
	m_strErrorMsg=L"发送请求失败，请检查网络";
#endif
}

wstring CHttpConnect::StringToWstring( const string& str )
{
	int nLen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	wchar_t* wp=new wchar_t[nLen+1];
	memset(wp,0,nLen+1);
	nLen=MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), wp, nLen);
	wp[nLen]='\0';
	wstring wstr=wp;
	delete[] wp;
	return wstr;
}

wstring CHttpConnect::Utf8ToUnicode(const string& strUtf8)  
{  
	wstring wstrRet(L"");
	int nLen=MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);  
	if (nLen == ERROR_NO_UNICODE_TRANSLATION)  
		throw "Utf8ToUnicode出错：无效的UTF-8字符串。";  
	wstrRet.resize(nLen+1,'\0');
	MultiByteToWideChar(CP_UTF8,0,strUtf8.c_str(), -1,(LPWSTR)wstrRet.c_str(), nLen);
	return wstrRet;  
}

string CHttpConnect::WstringToString( const wstring& str )
{
	int nLen= WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
	char* pBuffer=new char[nLen+1];
	WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen,NULL,NULL);
	pBuffer[nLen]='\0';
	string strRet(pBuffer);
	delete[] pBuffer;
	return strRet;
}

bool CHttpConnect::Download(const wstring& strUrl, const wstring& strSavePath)
{
	bool bResult=false;
	try
	{
		if ( strUrl.empty() )
			throw L"下载地址为空";
		Release();

		m_hSession = InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if ( NULL == m_hSession ) throw L"初始化HTTP接口失败";
		int pRet = ParseURL(strUrl);
		INTERNET_PORT port = pRet > 0 ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
		m_hConnect = InternetConnectW(m_hSession, m_wstrHostName.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if ( NULL == m_hConnect ) throw L"连接HTTP服务器失败";
		m_hRequest=HttpOpenRequestW(m_hConnect, L"GET", m_wstrPageName.c_str(), L"HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, NULL);
		if ( NULL == m_hRequest ) throw L"初始化HTTP请求失败";
		wstring strHeader;
		if ( strHeader.empty() )
			GetDownloadHttpHeader(strHeader);

		BOOL bRet = FALSE;
		bRet = HttpSendRequestW(m_hRequest, strHeader.c_str(), strHeader.size(), NULL, 0);
		if ( !bRet ) throw L"发送HTTP请求失败";
		char szBuffer[DOWNLOAD_BUFFER_SIZE+1]={0};
		DWORD dwReadSize=DOWNLOAD_BUFFER_SIZE;
		bRet=HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL);
		if ( !bRet ) throw L"查询HTTP返回头失败";
		string strRetHeader=szBuffer;
		if ( string::npos != strRetHeader.find("404") ) throw L"请求文件不存在!";
		dwReadSize=DOWNLOAD_BUFFER_SIZE;
		bRet=HttpQueryInfoA(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, szBuffer, &dwReadSize, NULL);
		//if ( !bRet ) throw L"查询HTTP返回头失败";
		szBuffer[dwReadSize]='\0';
		const UINT uFileSize=atoi(szBuffer);
		int nFindPos=0;
		while( wstring::npos != (nFindPos=strSavePath.find(L"\\", nFindPos)) )
		{
			wstring strChildPath=strSavePath.substr(0, nFindPos);
			if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributes(strChildPath.c_str())) 
				CreateDirectory(strChildPath.c_str(), NULL);
			nFindPos++;
		}
		FILE* fp=NULL;
		_wfopen_s(&fp, strSavePath.c_str(), L"wb+");
		if ( NULL == fp ) throw L"创建文件失败，可能是文件正在被占用";
		UINT uWriteSize=0;
		while( true )
		{
			bRet=InternetReadFile(m_hRequest, szBuffer, DOWNLOAD_BUFFER_SIZE, &dwReadSize);
			if ( !bRet || (0 == dwReadSize) )
				break;
			szBuffer[dwReadSize]='\0';
			fwrite(szBuffer, dwReadSize, 1, fp);
			uWriteSize+=dwReadSize;
			if ( m_hWnd ) ::PostMessage(m_hWnd, m_uMsg, uWriteSize, 0);

		}
		fclose(fp);
		if ( uFileSize!=uWriteSize ) throw L"下载文件失败，请检查网络设置";
		bResult=true;
	}
	catch( const wchar_t* pError )
	{
		SetErrorMsg(pError);
	}

	Release();
	return bResult;
}

bool CHttpConnect::Download(const string& strUrl, const string& strSavePath)
{

	bool bResult = false;
	try
	{
		if (strUrl.empty())
			throw L"下载地址为空";
		Release();

		m_hSession = InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if (NULL == m_hSession) throw L"初始化HTTP接口失败";
		int pRet = ParseURL(strUrl);
		m_hConnect = InternetConnectA(m_hSession, m_strHostName.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if (NULL == m_hConnect) throw L"连接HTTP服务器失败";
		m_hRequest = HttpOpenRequestA(m_hConnect, "GET", m_strPageName.c_str(), "HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, NULL);
		if (NULL == m_hRequest) throw L"初始化HTTP请求失败";
		string strHeader;
		if (strHeader.empty())
			GetDownloadHttpHeader(strHeader);

		BOOL bRet = FALSE;
		bRet = HttpSendRequestA(m_hRequest, strHeader.c_str(), strHeader.size(), NULL, 0);
		int ierror = GetLastError();
		//if (!bRet) throw L"发送HTTP请求失败";
		char szBuffer[DOWNLOAD_BUFFER_SIZE + 1] = { 0 };
		DWORD dwReadSize = DOWNLOAD_BUFFER_SIZE;
		bRet = HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL);
		if (!bRet) throw L"查询HTTP返回头失败";
		string strRetHeader = szBuffer;
		if (string::npos != strRetHeader.find("404")) throw L"请求文件不存在!";
		dwReadSize = DOWNLOAD_BUFFER_SIZE;
		bRet = HttpQueryInfoA(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, szBuffer, &dwReadSize, NULL);
		//if ( !bRet ) throw L"查询HTTP返回头失败";
		szBuffer[dwReadSize] = '\0';
		const UINT uFileSize = atoi(szBuffer);
		int nFindPos = 0;
		wstring strNewSavePath = StringToWstring(strSavePath);
		while (wstring::npos != (nFindPos = strNewSavePath.find(L"\\", nFindPos)))
		{
			wstring strChildPath = strNewSavePath.substr(0, nFindPos);
			if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributes(strChildPath.c_str()))
				CreateDirectory(strChildPath.c_str(), NULL);
			nFindPos++;
		}
		FILE* fp = NULL;
		_wfopen_s(&fp, strNewSavePath.c_str(), L"wb+");
		if (NULL == fp) throw L"创建文件失败，可能是文件正在被占用";
		UINT uWriteSize = 0;
		while (true)
		{
			bRet = InternetReadFile(m_hRequest, szBuffer, DOWNLOAD_BUFFER_SIZE, &dwReadSize);
			if (!bRet || (0 == dwReadSize))
				break;
			szBuffer[dwReadSize] = '\0';
			fwrite(szBuffer, dwReadSize, 1, fp);
			uWriteSize += dwReadSize;
			if (m_hWnd) ::PostMessage(m_hWnd, m_uMsg, uWriteSize, 0);

		}
		fclose(fp);
		if (uFileSize != uWriteSize) throw L"下载文件失败，请检查网络设置";
		bResult = true;
	}
	catch (const wchar_t* pError)
	{
		SetErrorMsg(pError);
	}
	return bResult;
}

struct __http_data_t
{
	HANDLE hEvt;
	HINTERNET hUrl;
};

static void CALLBACK InternetStatusCallback(
	_In_  HINTERNET hInternet,
	_In_  DWORD_PTR dwContext,
	_In_  DWORD dwInternetStatus,
	_In_  LPVOID lpvStatusInformation,
	_In_  DWORD dwStatusInformationLength
	)
{
	__http_data_t* data = (__http_data_t*)dwContext;
	////
	//        printf("**dwInternetStatus=%d\n", dwInternetStatus);
	if (dwInternetStatus == INTERNET_STATUS_REQUEST_COMPLETE){
		INTERNET_ASYNC_RESULT* res = (INTERNET_ASYNC_RESULT*)lpvStatusInformation;
		data->hUrl = (HINTERNET)res->dwResult;
		SetEvent(data->hEvt);
	}
	else if (dwInternetStatus == INTERNET_STATUS_HANDLE_CREATED){
		INTERNET_ASYNC_RESULT* res = (INTERNET_ASYNC_RESULT*)lpvStatusInformation;
		data->hUrl = (HINTERNET)res->dwResult;
	}
}

void CHttpConnect::FreeInstance()
{
	delete this;
}

void CHttpConnect::ReleaseHandle( HINTERNET& hInternet )
{
	if (hInternet) 
	{ 
		InternetCloseHandle(hInternet); 
		hInternet=NULL; 
	}
}

void CHttpConnect::Release()
{
	if (m_hRequest)
	{
		ReleaseHandle(m_hRequest);
		m_hRequest = NULL;
	}
	if (m_hRequest)
	{
		ReleaseHandle(m_hConnect);
		m_hRequest = NULL;
	}
	if (m_hRequest)
	{
		ReleaseHandle(m_hSession);
		m_hSession = NULL;
	}
}
