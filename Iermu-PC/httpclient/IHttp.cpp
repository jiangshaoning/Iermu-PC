// IHttp.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "HttpConnect.h"
#include "HttpSocket.h"
#include <Wininet.h>
#pragma comment(lib, "Wininet")
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi")



bool ParseURL( const wstring& strUrl, wstring& strHome, wstring& strPage)
{
	if ( strUrl.empty() )
		return false;
	wstring strTemp=strUrl;
	int nPos=strTemp.find(L"http://");
	if (wstring::npos != nPos )
		strTemp=strTemp.substr(nPos+7, strTemp.size()-nPos-7);
	nPos=strTemp.find('/');
	if ( wstring::npos == nPos )//没有找到 /
	{
		strHome=strTemp;
		return true;
	}
	strHome=strTemp.substr(0, nPos);
	strPage=strTemp.substr(nPos, strTemp.size()-nPos);
	return true;
}

LIB_FUN IHttpInterface* CreateHttpInstance()
{
	IHttpInterface* pInstance=new CHttpConnect;
	return pInstance;
}

LIB_FUN bool UrlDownload( const wstring& strUrl, const wstring& strSavePath, OUT UINT& uLoadSize, \
			  OUT wstring& strErrorMsg, HWND hWnd/*=NULL*/, UINT uMsg/*=0*/ )
{
 	bool bResult=false;
	HINTERNET hSession=NULL, hConnect=NULL, hRequest=NULL;
	try
	{
		if ( PathFileExists(strSavePath.c_str()) )
			::DeleteFile(strSavePath.c_str());
		hSession=InternetOpen(L"Http-connect", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if ( NULL == hSession ) throw L"初始化HTTP接口失败";
		wstring strHome, strPage;
		ParseURL(strUrl, strHome, strPage);
		int nPos=strHome.find(L":");
		int nPort=INTERNET_DEFAULT_HTTP_PORT;
		if ( nPort!=wstring::npos )
		{
			wstring strPort=strHome.substr(nPos+1);
			nPort=_ttoi(strPort.c_str());
			strHome=strHome.substr(0, nPos);
		}
		hConnect=InternetConnectW(hSession, strHome.c_str(), nPort, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if ( NULL == hConnect ) throw L"连接HTTP服务器失败";
		hRequest=HttpOpenRequestW(hConnect, L"GET", strPage.c_str(), L"HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, NULL);
		if ( NULL == hRequest ) throw L"初始化HTTP请求失败";
		wstring strHeader;
		strHeader.append(L"nReferer: ");
		strHeader+=strHome;
		strHeader.append(L"\r\nContent-Type: application/x-www-form-urlencoded\r\n");
		strHeader.append(L"Accept: */*\r\n");
		strHeader.append(L"Connection: Keep-Alive\r\n\r\n");
		BOOL bRet=HttpSendRequestW(hRequest, strHeader.c_str(), strHeader.size(), NULL, 0);
		if ( !bRet ) throw L"发送HTTP请求失败";
		char szBuffer[DOWNLOAD_BUFFER_SIZE+1]={0};
		DWORD dwReadSize=DOWNLOAD_BUFFER_SIZE;
		bRet=HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL);
		if ( !bRet ) throw L"查询HTTP返回头失败";
		string strRetHeader=szBuffer;
		if ( string::npos != strRetHeader.find("404") ) throw L"请求文件不存在!";
		dwReadSize=DOWNLOAD_BUFFER_SIZE;
		bRet=HttpQueryInfoA(hRequest, HTTP_QUERY_CONTENT_LENGTH, szBuffer, &dwReadSize, NULL);
		if ( !bRet ) throw L"查询HTTP返回头失败";
		szBuffer[dwReadSize]='\0';
		UINT uFileSize=atoi(szBuffer);
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
		int nPercent=0;
		while( true )
		{
			bRet=InternetReadFile(hRequest, szBuffer, DOWNLOAD_BUFFER_SIZE, &dwReadSize);
			if ( !bRet || (0 == dwReadSize) )
				break;
			szBuffer[dwReadSize]='\0';
			fwrite(szBuffer, dwReadSize, 1, fp);
			uLoadSize+=dwReadSize;
			if ( hWnd )
			{
				nPercent=(int)(100*(uLoadSize/(float)uFileSize));
				::PostMessage(hWnd, uMsg, nPercent, 0);
				::Sleep(20);
			}
		}
		fclose(fp);
		if ( uFileSize!=uLoadSize ) throw L"下载文件失败，请检查网络设置";
		bResult=true;
	}
	catch( const wchar_t* pError )
	{
		strErrorMsg=pError;
	}
	if ( hSession )
		InternetCloseHandle(hSession);
	if ( hConnect )
		InternetCloseHandle(hConnect);
	if ( hRequest )
		InternetCloseHandle(hRequest);
	return bResult;
}

LIB_FUN	IHttpInterface2* CreateHttpInstance2()
{
	IHttpInterface2* pHttp = new CHttpSocket;
	return pHttp;
}	