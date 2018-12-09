#include "stdafx.h"
#include "HttpSocket.h"
#pragma comment (lib, "ws2_32")
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi")

/////////////////////////////////////////////////////////////////////////
//class CHttpHeader
CHttpHeader::CHttpHeader( const char* pHeader )
:m_uReturnValue(0)
{
	Revolse(std::string(pHeader));
}

CHttpHeader::CHttpHeader( const std::string& strHeader )
:m_uReturnValue(0)
{
	Revolse(strHeader);
}

CHttpHeader::~CHttpHeader(void)
{
}

std::string CHttpHeader::GetValue( const std::string& strKey )
{
	std::string strResult;
	std::map<std::string, std::string>::const_iterator itor;
	itor=m_ValueMap.find(strKey);
	if ( itor != m_ValueMap.end() )
		strResult=itor->second;
	return strResult;
}

bool CHttpHeader::Revolse( const std::string& strHeader )
{
	int nStartPos=0, nFindPos=0, nLineIndex=0;
	std::string strLine, strKey, strValue;
	do
	{
		nFindPos=strHeader.find("\r\n", nStartPos);
		if ( -1 == nFindPos )
			strLine=strHeader.substr(nStartPos, strHeader.size()-nStartPos);
		else
		{
			strLine=strHeader.substr(nStartPos, nFindPos-nStartPos);
			nStartPos=nFindPos+2;
		}
		if ( 0 == nLineIndex )//第一行
		{
			strncpy_s(m_szHttpVersion, strLine.c_str(), 8);
			m_szHttpVersion[8]='\0';
			if ( strcmp(m_szHttpVersion, "HTTP/1.1") != 0 )
				return false;
			int nSpace1=strLine.find(" ");
			int nSpace2=strLine.find(" ", nSpace1+1);
			m_uReturnValue=atoi(strLine.substr(nSpace1+1, nSpace2-nSpace1-1).c_str());
			m_strContent=strLine.substr(nSpace2+1, strLine.size()-nSpace2-1);
			nLineIndex++;
			continue;
		}
		int nSplit=strLine.find(": ");
		strKey=strLine.substr(0, nSplit);
		strValue=strLine.substr(nSplit+2, strLine.size()-nSplit-2);
		std::pair<std::string ,std::string> data;
		data.first=strKey;
		data.second=strValue;
		m_ValueMap.insert(data);
		nLineIndex++;
	}
	while(nFindPos!=-1);
	return true;
}

/////////////////////////////////////////////////////////////////////////


CHttpSocket::CHttpSocket()
:m_socket(INVALID_SOCKET)
,m_pCallback(NULL)
{
	WSADATA data;
	WSAStartup(0x0202, &data);
}

CHttpSocket::~CHttpSocket()
{
	if ( INVALID_SOCKET != m_socket )
		closesocket(m_socket);
	WSACleanup();
}

void CHttpSocket::ParseUrl( const wstring& strUrl, wstring& strHostName, wstring& strPage, WORD& sPort )
{
	sPort = 80;
	wstring strTemp=strUrl;
	int nPos=strTemp.find(L"http://");
	if (wstring::npos != nPos )
		strTemp=strTemp.substr(nPos+7, strTemp.size()-nPos-7);
	nPos=strTemp.find('/');
	if ( wstring::npos == nPos )//没有找到 /
		strHostName=strTemp;
	else
		strHostName = strTemp.substr(0, nPos);
	int nPos1 = strHostName.find(':');
	if ( nPos1 != wstring::npos )
	{
		wstring strPort = strTemp.substr(nPos1+1, strHostName.size()-nPos1-1);
		strHostName = strHostName.substr(0, nPos1);
		sPort = (u_short)_wtoi(strPort.c_str());
	}
	if ( wstring::npos == nPos )
		return ;
	strPage=strTemp.substr(nPos, strTemp.size()-nPos);
}

bool CHttpSocket::InitSocket( const string& strHostName, const WORD sPort )
{
	bool bResult = false;
	try
	{
		HOSTENT* pHostent=gethostbyname(strHostName.c_str());
		if ( NULL == pHostent )
			throw _T("获取域名对应的地址失败");
		char szIP[16]={0};
		sprintf_s(szIP, "%d.%d.%d.%d",
			pHostent->h_addr_list[0][0]&0x00ff,
			pHostent->h_addr_list[0][1]&0x00ff,
			pHostent->h_addr_list[0][2]&0x00ff,
			pHostent->h_addr_list[0][3]&0x00ff);
		m_strIpAddr = A2U(szIP);
		if ( INVALID_SOCKET != m_socket )
			closesocket(m_socket);
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if ( INVALID_SOCKET == m_socket )
			throw _T("创建套接字失败");
		int nSec=1000*10;//10秒内没有数据则说明网络断开
		setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nSec, sizeof(int));
		sockaddr_in		addrServer;
		addrServer.sin_family=AF_INET;
		addrServer.sin_port=htons(sPort);
		addrServer.sin_addr.S_un.S_addr=inet_addr(szIP);
		if ( SOCKET_ERROR == connect(m_socket, (SOCKADDR*)&addrServer, sizeof(addrServer)) )
			throw _T("连接服务器失败");
		bResult=true;
	}
	catch(const wchar_t* pException)
	{
		m_strLastError = pException;
	}
	catch(...)
	{

	}
	return bResult;
}

bool CHttpSocket::DownloadFile( const wstring& strUrl, const wstring& strSavePath )
{
	bool bResult = false;
	try
	{
		wstring strHostName, strPage;
		u_short uPort=80;
		ParseUrl(strUrl, strHostName, strPage, uPort);
		string str = U2A(strHostName);
		if ( ! InitSocket(str, uPort) )
			throw L"";
		HTTP_HERDER header;
		strcpy_s(header.szHostName, str.c_str());
__request:
		InitRequestHeader(header, U2A(strPage).c_str());
		std::string strSend = header.ToString();
		int nRet=send(m_socket, strSend.c_str(), strSend.size(), 0);
		if ( SOCKET_ERROR == nRet )
			throw _T("发送请求失败");
		int		nRecvSize=0, nFileSize=0, nLoadSize=0;
		char	szRecvBuffer[RECV_BUFFER_SIZE+1]={0};
		bool	bFilter=false;//HTTP返回头是否已经被过滤掉
		if ( PathFileExists(strSavePath.c_str()) )
			DeleteFile(strSavePath.c_str());
		do
		{
			if ( m_pCallback && m_pCallback->IsNeedStop() )
				throw _T("用户取消下载");
			nRecvSize = recv(m_socket, szRecvBuffer, RECV_BUFFER_SIZE, 0);
			if ( SOCKET_ERROR == nRecvSize )
				throw _T("接收失败，请检查网络");
			if ( nRecvSize>0 )
			{
				szRecvBuffer[nRecvSize]='\0';
				if ( !bFilter )
				{
					std::string str(szRecvBuffer);
					int nPos=str.find("\r\n\r\n");
					if ( -1 == nPos )
						continue;
					std::string strHeader;
					strHeader.append(szRecvBuffer, nPos);
					CHttpHeader header(strHeader);
					int nHttpValue = header.GetReturnValue();
					if ( 404 == nHttpValue )//文件不存在
					{
						throw _T("下载文件不存在");
					}
					if ( nHttpValue>300 && nHttpValue<400 )//重定向
					{
						wstring strReLoadUrl = A2U(header.GetValue("location"));
						if ( strReLoadUrl.find(L"http://") != 0 )
						{
							strPage = strReLoadUrl;
							goto __request;
						}
						if ( INVALID_SOCKET != m_socket )
						{
							closesocket(m_socket);
							m_socket = INVALID_SOCKET;
						}
						return DownloadFile(strReLoadUrl, strSavePath);
					}
					nFileSize=atoi(header.GetValue("Content-Length").c_str());
					nRecvSize-=(nPos+4);
					FILE* fp = NULL;
					_wfopen_s(&fp, strSavePath.c_str(), L"ab+");
					if ( NULL == fp )
						throw L"文件指针为空，写入文件失败";
					fwrite(szRecvBuffer+nPos+4, nRecvSize, 1, fp);
					fclose(fp);
					nLoadSize+=nRecvSize;
					if ( m_pCallback )
						m_pCallback->OnDownloadCallback(this, DS_Loading, nFileSize, nLoadSize);
					if ( nFileSize == nLoadSize )
					{
						if ( m_pCallback )
							m_pCallback->OnDownloadCallback(this, DS_Finished, nFileSize, nLoadSize);
						bResult = true;
						break;
					}
					bFilter=true;
					nRecvSize = 1;
					continue;
				}
				FILE* fp = NULL;
				_wfopen_s(&fp, strSavePath.c_str(), L"ab+");
				if ( NULL == fp )
					throw L"文件指针为空，写入文件失败";
				fwrite(szRecvBuffer, nRecvSize, 1, fp);
				fclose(fp);
				nLoadSize+=nRecvSize;
				if ( m_pCallback )
					m_pCallback->OnDownloadCallback(this, DS_Loading, nFileSize, nLoadSize);
				if ( nLoadSize>=nFileSize )
				{
					bResult = true;
					if ( m_pCallback )
						m_pCallback->OnDownloadCallback(this, DS_Finished, nFileSize, nLoadSize);
					break;
				}
			}
			Sleep(10);
		}
		while( nRecvSize>0 );
	}
	catch(const wchar_t* pException)
	{
		if ( wcslen(pException) != 0 )
			m_strLastError = pException;
		if ( m_pCallback )
			m_pCallback->OnDownloadCallback(this, DS_Fialed, 0, 0);
	}
	catch(...)
	{

	}
	if ( !bResult )
		DeleteFile(strSavePath.c_str());
	return bResult;
}

string CHttpSocket::U2A(const wstring& str)
{
	string strDes;
	if ( str.empty() )
		goto __end;
	int nLen=::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
	if ( 0==nLen )
		goto __end;
	char* pBuffer=new char[nLen+1];
	memset(pBuffer, 0, nLen+1);
	::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen, NULL, NULL);
	pBuffer[nLen]='\0';
	strDes.append(pBuffer);
	delete[] pBuffer;
__end:
	return strDes;
}

wstring CHttpSocket::A2U(const string& str)
{
	wstring strDes;
	if ( str.empty() )
		goto __end;
	int nLen=::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	if ( 0==nLen )
		goto __end;
	wchar_t* pBuffer=new wchar_t[nLen+1];
	memset(pBuffer, 0, nLen+1);
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen);
	pBuffer[nLen]='\0';
	strDes.append(pBuffer);
	delete[] pBuffer;
__end:
	return strDes;
}

void CHttpSocket::InitRequestHeader( HTTP_HERDER& header, const char* pRequest, REQUEST_TYPE type/*=get*/, \
							  const char* pRange/*=NULL*/, const char* pAccept/*="* / *"*/ )
{
	memset(header.szType, 0, 5);
	if (GET == type)
		strcpy_s(header.szType, "GET");
	else
		strcpy_s(header.szType, "POST");
	memset(header.szRequest, 0, MAX_PATH);
	strcpy_s(header.szRequest, pRequest);
	memset(header.szAccept, 0, 100);
	strcpy_s(header.szAccept, pAccept);
	memset(header.szRange, 0, 11);
	if ( pRange )
		strcpy_s(header.szRange, pRange);
}

void CHttpSocket::SetCallback( CDownloadCallback* pCallback )
{
	m_pCallback = pCallback;
}

void CHttpSocket::FreeInstance()
{
	delete this;
}

// bool CHttpSocket::HttpPost( const wstring& strUrl, const wstring& strPostData, string& strResult )
// {
// 	wstring strHostName, strPage;
// 	u_short uPort=80;
// 	ParseUrl(strUrl, strHostName, strPage, uPort);
// 	string str = U2A(strHostName);
// 	if ( ! InitSocket(str, uPort) )
// 		return false;
// 	strPage +=strPostData;
// 	HTTP_HERDER header;
// 	strcpy_s(header.szHostName, str.c_str());
// 	InitRequestHeader(header, U2A(strPage).c_str(), post);
// 	std::string strSend = header.ToString();
// 	int nRet=send(m_socket, strSend.c_str(), strSend.size(), 0);
// 	if ( SOCKET_ERROR == nRet )
// 		throw _T("发送请求失败");
// 	char szBuffer[RECV_BUFFER_SIZE+1]={0};
// 	string strResult;
// 	while( true )
// 	{
// 		int nRecvSize = recv(m_socket, szBuffer, RECV_BUFFER_SIZE, 0);
// 		if ( SOCKET_ERROR == nRecvSize )
// 			return false;
// 		if ( nRecvSize == 0 )
// 			break;
// 		
// 	}
// }

