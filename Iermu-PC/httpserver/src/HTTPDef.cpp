/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#include "stdafx.h"
#include <time.h>
#include "HTTPDef.h"
#include <WS2tcpip.h>

static char month[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static char week[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

const char* g_HTTP_Content_NotFound = "404 Not Found - Q++ HTTP Server";
const char* g_HTTP_Bad_Request = "400 Bad Request - Q++ HTTP Server";
const char* g_HTTP_Bad_Method = "405 Method Not Allowed - Q++ HTTP Server";
const char* g_HTTP_Server_Error = "500 Oops, server error - Q++ HTTP Server";
const char* g_HTTP_Forbidden = "403 Forbidden - Q++ HTTP Server";
const char* g_HTTP_Server_Busy = "503 Service Unavailable, try again later - Q++ HTTP Server";

std::string format_http_date(__int64* ltime)
{
	struct tm t;
	if(ltime != NULL)
	{
		_gmtime64_s(&t, ltime);
	}
	else
	{
		//  �����ָ��,��ȡ��ǰʱ��.
		__int64 ltime_cur;
		_time64( &ltime_cur );
		_gmtime64_s(&t, &ltime_cur);
	}

	char szTime[100] = {0};

	// ��ʽ���ʼ�ʱ�� - Sun, 24 Aug 2008 22:43:45 GMT
	sprintf(szTime, "%s, %d %s %d %d:%d:%d GMT", 
		week[t.tm_wday], t.tm_mday, month[t.tm_mon], 
		t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec);

	return szTime;
}

std::string to_hex(const unsigned char* pData, int nSize)
{
	int nStrSize = nSize * 2 + 1;
	char* pStr = new char[nStrSize];
	memset(pStr, 0, nStrSize);

	int nPos = 0;
	for(int i = 0; i < nSize; ++i)
	{
		pStr[nPos] = hex[pData[i] >> 4];
		pStr[nPos + 1] = hex[pData[i] & 0x0F];
		nPos += 2;
	}

	std::string str(pStr);
	delete[] pStr;

	return str;
}

std::string decode_url(const std::string& inputStr)
{
	// ת��ΪANSI����
	const std::string &astr = inputStr;
	std::string destStr("");

	// ɨ��,�õ�һ��UTF8�ַ���
	bool isEncoded = false;
	for(std::string::size_type idx = 0; idx < astr.size(); ++idx)
	{
		char ch = astr[idx];
		if(ch == '%')
		{
			isEncoded = true;
			if( idx + 1 < astr.size() && idx + 2 < astr.size() )
			{
				char orgValue[5] = {0}, *stopPos = NULL;
				orgValue[0] = '0';
				orgValue[1] = 'x';
				orgValue[2] = astr[idx + 1];
				orgValue[3] = astr[idx + 2];
				orgValue[4] = 0;
				ch = static_cast<char> (strtol(orgValue, &stopPos, 16));

				idx += 2;
			}
			else
			{
				// ��ʽ����
				break;
			}
		}

		destStr.push_back(ch);
	}

	if(isEncoded) return UTF8toA(destStr);
	else return inputStr;
}

bool map_method(HTTP_METHOD md, char *str)
{
	switch(md)
	{
	case METHOD_HEAD: strcpy(str, "HEAD"); break;
	case METHOD_PUT: strcpy(str, "PUT"); break;
	case METHOD_POST: strcpy(str, "POST"); break;
	case METHOD_GET: strcpy(str, "GET"); break;
	case METHOD_TRACE: strcpy(str, "TRACE"); break;
	case METHOD_CONNECT: strcpy(str, "CONNECT"); break;
	case METHOD_DELETE: strcpy(str, "DELETE"); break;
	default: return false;
	}
	return true;
}


bool is_end(const byte *data, size_t len)
{
	if( len < 4 ) return false;
	else return strncmp(reinterpret_cast<const char*>(data) + len - 4, "\r\n\r\n", 4) == 0;
}

char * __cdecl stristr(const char * str1, const char * str2)
{
	char *cp = (char *)str1;
	char *s1, *s2;

	if (!*str2)
		return((char *)str1);

	while (*cp)
	{
		s1 = cp;
		s2 = (char *)str2;

		while (*s1 && *s2 && (!(*s1 - *s2) || !(*s1 - *s2 - 32) || !(*s1 - *s2 + 32))){
			s1++, s2++;
		}

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);
}

std::string get_field(const char* buf, const char* key)
{
	std::string strKey(key);
	strKey += ": ";
	std::string strValue("");

	// �ҵ��ֶεĿ�ʼ:��ʼλ�û�������һ�е���ʼλ��
	const char* pszStart = stristr(buf, strKey.c_str());
	if(pszStart == NULL || (pszStart != buf && *(pszStart - 1) != '\n')) return strValue;
	pszStart += strKey.size();

	// �ҵ��ֶν���
	const char* pszEnd = strstr(pszStart, "\r\n");
	if(pszEnd == NULL) return strValue;

	strValue.assign(pszStart, pszEnd - pszStart);
	return strValue;
}


/*
* ��ȡ�ļ�������չ��,�� html,php��.
*/
void get_file_ext(const std::string &fileName, std::string &ext)
{
	std::string::size_type st = 0, ed = 0;

	/* �ҵ����һ��'/'*/
	std::string::size_type pos = fileName.rfind('/');
	if(pos != std::string::npos)
	{
		st = pos + 1;
	}
	else
	{
		st = 0;
	}

	/* ����һ���Ļ�����,�ҵ���һ��'.'*/
	pos = fileName.find('.', st);
	if(pos != std::string::npos)
	{
		st = pos + 1;
	}
	else
	{
		st = fileName.size();
	}

	/* ֱ���ļ�����β */
	ed = fileName.size();

	if(st < ed)
	{
		ext = fileName.substr(st, ed - st);
	}
	else
	{
		ext = "";
	}
}

/*
* �Ƚ���չ�Ƿ�ƥ��,��չ���б�ĸ�ʽ��: "php,htm,html;js"
* �����չ���б�Ϊ "*" ��ʾƥ������.
*/
bool match_file_ext(const std::string &ext, const std::string &extList)
{
	if(ext.empty()) return false;
	if( extList == "*" ) return true;

	/* �Ƿ���� */
	std::string::size_type pos = extList.find(ext);
	if( pos == extList.npos )
	{
		return false;
	}

	/* �ַ�����ͷ������",;"����*/
	if(pos != 0)
	{
		if(extList.at(pos - 1) == ',' || extList.at(pos - 1) == ';')
		{
		}
		else
		{
			return false;
		}
	}

	/* �ַ�����β���ߺ��滹�� ",;" */
	if( pos + ext.size() != extList.size() )
	{
		pos += ext.size();
		if(extList.at(pos) == ',' || extList.at(pos) == ';')
		{
		}
		else
		{
			return false;
		}
	}

	return true;
}

/*
* ��� GetLastError() �������ַ���.
*/
std::string get_last_error(DWORD errCode /* = 0 */)
{
	std::string err("");
	if( errCode == 0 ) errCode = GetLastError();
	LPTSTR lpBuffer = NULL;
	if( 0 == FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
		NULL,
		errCode, 
		0,  
		(LPTSTR)&lpBuffer, 
		0, 
		NULL) )
	{
		char tmp[100] = {0};
		sprintf(tmp, "{δ�����������(%d)}", errCode);
		err = tmp;
	}
	else
	{
		err = TtoA(lpBuffer);
		LocalFree(lpBuffer);
	}
	return err;
}

/*
* �ָ��ַ���
*/
size_t split_strings(const std::string &str, str_vec_t &vec, const std::string &sp)
{
	std::string srcString(str);
	srcString += sp; // ����һ���ָ���,����ѭ������.
	std::string::size_type st = 0;
	std::string::size_type stNext = 0;
	while( (stNext = srcString.find(sp, st)) != std::string::npos )
	{
		if(stNext > st)
		{
			vec.push_back(srcString.substr(st, stNext - st));
		}

		// next
		st = stNext + sp.size();
	}
	return vec.size();
}

/*
* ��ȡ������ IP ��ַ
*/
bool get_ip_address(std::string& str)
{
	char hostName[MAX_PATH] = {0};
	if(gethostname(hostName, MAX_PATH))
	{
		return FALSE;
	}
	else
	{
		addrinfo hints, *res, *nextRes;
		memset(&hints, 0, sizeof(addrinfo));
		res = NULL;
		nextRes = NULL;
		hints.ai_family = AF_INET;

		if(getaddrinfo(hostName, NULL, &hints, &res))
		{
			return false;
		}
		else
		{
			str = "";
			nextRes = res;
			while(nextRes)
			{
				in_addr inAddr = ((sockaddr_in*)(nextRes->ai_addr))->sin_addr;
				str += inet_ntoa(inAddr);
				str += "/";

				nextRes = nextRes->ai_next;
			}
			str.pop_back();

			freeaddrinfo(res);
		}
	}

	return true;
}

std::string format_size(__int64 bytes)
{
	// �����Է������ݵĳ���
	char buf[100] = {0};
	if(bytes >= G_BYTES)
	{
		sprintf(buf, "%.2fGB",  bytes * 1.0 / G_BYTES);
	}
	else if(bytes >= M_BYTES)
	{
		sprintf(buf, "%.2fMB", bytes * 1.0 / M_BYTES);
	}
	else if(bytes >= K_BYTES)
	{
		sprintf(buf, "%.2fKB", bytes * 1.0 / K_BYTES);
	}
	else
	{
		sprintf(buf, "%lldBytes", bytes);
	}
	return std::string(buf);
}


std::string format_speed(__int64 bytes, unsigned int timeUsed)
{
	// ����ƽ������
	char buf[100] = {0};

	if(timeUsed <= 0)
	{
		strcpy(buf, "---");
	}
	else
	{
		double llSpeed = bytes * 1.0 / timeUsed * 1000;
		if(llSpeed >= G_BYTES)
		{
			sprintf(buf, "%.2fGB/s", llSpeed * 1.0 / G_BYTES);
		}
		else if(llSpeed >= M_BYTES)
		{
			sprintf(buf, "%.2fMB/s", llSpeed * 1.0 / M_BYTES);
		}
		else if(llSpeed >= K_BYTES)
		{
			sprintf(buf, "%.2fKB/s", llSpeed * 1.0 / K_BYTES);
		}
		else
		{
			sprintf(buf, "%.2fB/s", llSpeed);
		}
	}

	return std::string(buf);
}

