/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#include "StdAfx.h"
#include "HTTPContent.h"
#include "HTTPDef.h"
#include <io.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

HTTPContent::HTTPContent()
	: _openType(OPEN_NONE), _contentType(""), _fileName(""), _from(0), _to(0)
{
	memset(&_fileInf, 0, sizeof(_fileInf));
}

HTTPContent::~HTTPContent()
{
	close();
}

bool HTTPContent::open(const std::string &fileName, __int64 from, __int64 to)
{
	if(OPEN_NONE != _openType) return false;
	std::string strFileName = fileName;

	_file.open(AtoT(strFileName).c_str(), WINFile::r);
	//_file = fopen(strFileName.c_str(), "rb");
	//if(NULL == _file)
	if(!_file.isopen())
	{
	}
	else
	{
		if( 0 != _stat32i64(strFileName.c_str(), &_fileInf))
		{
			assert(0);
		}

		__int64 fileSize = _fileInf.st_size;

		_openType = OPEN_FILE;
		_fileName = fileName;
			
		_from = from;
		_to = to;
			
		// lFrom Ӧ�ô���0��С���ļ��ĳ���
		if(_from > 0 && _from < fileSize)
		{
			//_fseeki64(_file, _from, SEEK_SET);
			_file.seek(_from, WINFile::s_set);
		}
		else
		{
			_from = 0;
		}
			
		// lTo Ӧ�ô��ڵ��� lFrom��С���ļ��ĳ���.
		if(_to >= _from && _to <  fileSize)
		{
		}
		else
		{
			_to = fileSize - 1;
		}
	}

	return OPEN_NONE != _openType;
}

/*
* ��ȡĿ¼�б����Ϊһ��HTML�ı���.
*/
bool HTTPContent::open(const std::string &urlStr, const std::string &filePath)
{
	if(OPEN_NONE != _openType) return false;
	assert(_memfile.fsize() == 0);

	char buffer[_MAX_PATH + 100] = {0};
	char sizeBuf[_MAX_PATH + 100] = {0};

	// ����һ��UTF8 HTML�ı���,�������ļ��б�.
	
	// 1. ���HTMLͷ,��ָ��UTF-8�����ʽ
	writeString("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/></head>");
	writeString("<body>");

	// 2. ���·��
	//(1). �����һ�� ��Ŀ¼
	writeString("<A href=\"/\">/</A>");

	//(2). ����Ŀ¼
	std::string::size_type st = 1;
	std::string::size_type stNext = 1;
	while( (stNext = urlStr.find('/', st)) != std::string::npos)
	{
		std::string strDirName =  urlStr.substr(st, stNext - st + 1);
		std::string strSubUrl = urlStr.substr(0, stNext + 1);

		writeString("&nbsp;|&nbsp;");

		writeString("<A href=\"");
		writeString(AtoUTF8(strSubUrl).c_str());
		writeString("\">");
		writeString(AtoUTF8(strDirName).c_str());
		writeString("</A>");
		
		// ��һ��Ŀ¼
		st = stNext + 1;
	}
	writeString("<br /><hr />");

	// 3. �г���ǰĿ¼�µ������ļ�
	std::string strFullName;
	strFullName = filePath;
	if(strFullName.back() != '\\') strFullName += '\\'; // ���������'\\'��β���ļ�·��,����. ע��һ��ԭ��:URL����б�ָܷ�;�ļ����Է�б�ָܷ�
	strFullName += "*";

	std::string strFiles(""); // ��ͨ�ļ�д������ַ�����.

	__finddata64_t fileinfo;
	intptr_t findRet = _findfirst64(strFullName.c_str(), &fileinfo);
	if( -1 != findRet )
	{
		do 
		{
			// ���� . �ļ�
			if( stricmp(fileinfo.name, ".") == 0 || 0 == stricmp(fileinfo.name, "..") )
			{
				continue;
			}

			// ����ϵͳ�ļ��������ļ�
			if( fileinfo.attrib & _A_SYSTEM || fileinfo.attrib & _A_HIDDEN )
			{
				continue;
			}

			// �����Ŀ¼����
			if( fileinfo.attrib & _A_SUBDIR )
			{
				// �������Ŀ¼,ֱ��д��

				// ����޸�ʱ��
				_ctime64_s( buffer, _countof(buffer), &fileinfo.time_write );
				writeString(AtoUTF8(buffer).c_str());

				// Ŀ¼����Ҫת��ΪUTF8����
				sprintf(buffer, "%s/", fileinfo.name);
				std::string fileurl = AtoUTF8(urlStr.c_str());
				std::string filename = AtoUTF8(buffer);

				writeString("&nbsp;&nbsp;");
				writeString("<A href=\"");
				writeString(fileurl.c_str());
				writeString(filename.c_str());
				writeString("\">");
				writeString(filename.c_str());
				writeString("</A>");

				// д��Ŀ¼��־
				writeString("&nbsp;&nbsp;[DIR]");

				// ����
				writeString("<br />");
			}
			else
			{
				// ��ͨ�ļ�,д�뵽һ��������ַ���string������,ѭ�����ٺϲ�.����,���е�Ŀ¼����ǰ��,�ļ��ں���
				_ctime64_s( buffer, _countof(buffer), &fileinfo.time_write );
				strFiles += AtoUTF8(buffer);

				// �ļ���ת��ΪUTF8������д��
				std::string filename = AtoUTF8(fileinfo.name);
				std::string fileurl = AtoUTF8(urlStr.c_str());

				strFiles += "&nbsp;&nbsp;";
				strFiles += "<A href=\"";
				strFiles += fileurl;
				strFiles += filename;
				strFiles += "\">";
				strFiles += filename;
				strFiles += "</A>";

				// �ļ���С
				// ע: ����Windows�� wsprintf ��֧�� %f ����,����ֻ���� sprintf ��
				double filesize = 0;
				if( fileinfo.size >= G_BYTES)
				{
					filesize = (fileinfo.size * 1.0) / G_BYTES;
					sprintf(sizeBuf, "%.2f&nbsp;GB", filesize);
				}
				else if( fileinfo.size >= M_BYTES ) // MB
				{
					filesize = (fileinfo.size * 1.0) / M_BYTES;
					sprintf(sizeBuf, "%.2f&nbsp;MB", filesize);
				}
				else if( fileinfo.size >= K_BYTES ) //KB
				{
					filesize = (fileinfo.size * 1.0) / K_BYTES;
					sprintf(sizeBuf, "%.2f&nbsp;KB", filesize);
				}
				else // Bytes
				{
					sprintf(sizeBuf, "%lld&nbsp;Bytes", fileinfo.size);
				}
			
				strFiles += "&nbsp;&nbsp;";
				strFiles += sizeBuf;

				// ����
				strFiles += "<br />";
			}
		} while ( -1 != _findnext64(findRet, &fileinfo));
		
		_findclose(findRet);
	}

	// ���ļ��ַ���д�뵽 Content ��.
	if(strFiles.size() > 0)
	{
		writeString(strFiles.c_str());
	}

	// 4. ���������־.
	writeString("</body></html>");

	_openType = OPEN_DIR;
	return true;
}

bool HTTPContent::open(const char* buf, int len, int type)
{
	if(OPEN_NONE != _openType) return false;

	assert(_memfile.fsize() == 0);
	if( len == write(buf, len) )
	{
		_openType = type;
	}
	else
	{
		assert(0);
	}

	return OPEN_NONE != _openType;
}

void HTTPContent::close()
{
	//assert( OPEN_NONE != _openType );

	_contentType = "";
	_fileName = "";

	
	_file.close();
	_memfile.trunc();

	_openType = OPEN_NONE;
}

bool HTTPContent::isOpen()
{
	return  OPEN_NONE != _openType;
}

std::string HTTPContent::getContentTypeFromFileName(const char* pszFileName)
{
	std::string strType = "application/octet-stream";

	const char *pExt = strrchr(pszFileName, '.');
	if(pExt && strlen(pExt) < 19)
	{
		char szExt[20];
		strcpy(szExt, pExt + 1);

		if(stricmp(szExt, "jpg") == 0)
		{
			strType =  "image/jpeg";
		}
		else if(stricmp(szExt, "txt") == 0)
		{
			strType = "text/plain";
		}
		else if(stricmp(szExt, "htm") == 0)
		{
			strType = "text/html";
		}
		else if(stricmp(szExt, "html") == 0)
		{
			strType = "text/html";
		}
		else if(stricmp(szExt, "gif") == 0)
		{
			strType = "image/gif";
		}
		else if(stricmp(szExt, "png") == 0)
		{
			strType = "image/png";
		}
		else if(stricmp(szExt, "bmp") == 0)
		{
			strType = "image/x-xbitmap";
		}
		else
		{
		}
	}

	return strType;
}

size_t HTTPContent::writeString(const char* pszString)
{
	return write((void *)pszString, strlen(pszString));
}

size_t HTTPContent::write(const void* buf, size_t len)
{
	if(_file.isopen())
	{
		/* Ŀǰû���õ���д�� HTTPContent */
		assert(0);
		return _file.write(buf, len);
	}
	else
	{
		return _memfile.write(buf, len);
	}
}

std::string HTTPContent::contentType()
{
	std::string strType("application/octet-stream");

	if(_openType == OPEN_FILE)
	{
		strType = getContentTypeFromFileName(_fileName.c_str());
	}
	else if(_openType == OPEN_TEXT)
	{
		strType = "text/plain";
	}
	else if(_openType == OPEN_HTML)
	{
		strType = "text/html";
	}
	else if(_openType == OPEN_DIR)
	{
		strType = "text/html";
	}
	else
	{
	}

	return strType;
}


__int64 HTTPContent::contentLength()
{
	__int64 nLength = 0;

	if(_openType == OPEN_FILE)
	{
		nLength = _to - _from + 1;
	}
	else
	{
		nLength = _memfile.fsize();
	}

	return nLength;
}

std::string HTTPContent::lastModified()
{
	__int64 ltime;

	if(_openType == OPEN_FILE)
	{
		ltime = _fileInf.st_mtime;
	}
	else
	{
		_time64( &ltime );
	}

	return format_http_date(&ltime);
}

std::string HTTPContent::contentRange()
{
	char szRanges[300] = {0};
	if(_openType == OPEN_FILE)
	{
		sprintf(szRanges, "bytes %lld-%lld/%lld", _from, _to, _fileInf.st_size);
	}
	else
	{
		
	}
	return szRanges;
}

bool HTTPContent::isFile()
{
	return _file.isopen();
}

bool HTTPContent::eof()
{
	if(_file.isopen())
	{
		//if(feof(_file))
		if(_file.eof())
		{
			return true;
		}
		else
		{
			return _file.tell() >= _to;
			//return _ftelli64(_file) >= _to;
		}
	}
	else
	{
		return _memfile.eof();
	}
}

size_t HTTPContent::read(void* buf, size_t len)
{
	assert(len);

	if(_file.isopen())
	{
		int nRet = 0;
		//__int64 lCurPos = _ftelli64(_file);  // ftell()���ص��ǵ�ǰָ���λ��,ָ���һ��δ�����ֽ�
		__int64 lCurPos = _file.tell();
		__int64 lLeftSize = _to - lCurPos + 1; // �ļ���ʣ����ֽ���

		if(len > lLeftSize)
		{
			len = static_cast<size_t>(lLeftSize);// �˴��ǰ�ȫ��
		}
		//return fread(buf, 1, len, _file); 
		return _file.read(buf, len);
	}
	else
	{
		return _memfile.read(buf, len);
	}
}

std::string HTTPContent::etag()
{
	std::string strETag("");
	if(OPEN_FILE == _openType)
	{
		char szLength[201] = {0};
		//_ltoa_s((_to - _from + 1), szLength, 200, 10);
		_i64toa(_fileInf.st_size, szLength, 10);

		// ������ļ�, �����ļ���С���޸����ڴ���. [ETag: ec5ee54c00000000:754998030] �޸�ʱ���HEXֵ:�ļ�����
		// ȷ��ͬһ����Դ�� ETag ��ͬһ��ֵ.
		// ��ʹ�ͻ��������ֻ�������Դ��һ����.
		// �ϵ������ͻ��˸��� ETag ��ֵȷ�����صļ��������ǲ���ͬһ���ļ�.
		strETag = to_hex((const unsigned char*)(&_fileInf.st_mtime), sizeof(_fileInf.st_mtime));
		strETag += ":";
		strETag += szLength;
	}
	else
	{
		char szLength[201] = {0};
		 _ltoa_s(_memfile.fsize(), szLength, 200, 10); // �ڴ�����û��Ҫ�� __int64

		// ������ڴ�����, ���ݴ�С��ȡ���ɸ��ֽڵ�16�����ַ�����.
		unsigned char szValue[ETAG_BYTE_SIZE + 1];

		for(int i = 0; i < ETAG_BYTE_SIZE; ++i)
		{
			int nUnit = _memfile.fsize() / ETAG_BYTE_SIZE;
			szValue[i] = reinterpret_cast<const char*>(_memfile.buffer())[nUnit * i];
		}

		strETag = to_hex(szValue, ETAG_BYTE_SIZE);
		strETag += ":";
		strETag += szLength;
	}

	return strETag;
}


/*
int HTTPContent::Seek(int nOffset);
*/