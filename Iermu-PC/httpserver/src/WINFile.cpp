#include "StdAfx.h"
#include "WINFile.h"
#include <io.h>
#include <direct.h>

WINFile::WINFile()
	: _h(INVALID_HANDLE_VALUE)
{
}


WINFile::~WINFile()
{
	close();
}

bool WINFile::exist(const TCHAR *fileName)
{
	return INVALID_FILE_ATTRIBUTES != GetFileAttributes(fileName);
	// && ERROR_FILE_NOT_FOUND == GetLastError())
}

bool WINFile::remove(const TCHAR *fileName)
{
	return DeleteFile(fileName) == TRUE;
}

bool WINFile::open(const TCHAR *fileName, unsigned int mode, bool tmp)
{
	if(INVALID_HANDLE_VALUE != _h) return false;
	DWORD dwCreateFlag = 0;
	DWORD dwAccess = 0;
	DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	if(mode == r)
	{
		/* ֻ�� */
		dwCreateFlag = OPEN_EXISTING;
		dwAccess = GENERIC_READ;
	}
	else if(mode == w)
	{
		/* ֻд */
		dwCreateFlag = CREATE_ALWAYS;
		dwAccess = GENERIC_WRITE;
	}
	else
	{
		/* ��д */
		dwCreateFlag = OPEN_ALWAYS;
		dwAccess = GENERIC_READ | GENERIC_WRITE;
	}

	//if(tmp)
	//{
	//	/* ��ʱ�ļ�,�Զ�ɾ�� */
	//	dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE;
	//}

	_h = CreateFile(fileName, dwAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, dwCreateFlag, dwFlagsAndAttributes, NULL);
	return _h != INVALID_HANDLE_VALUE;
}

bool WINFile::close()
{
	if(INVALID_HANDLE_VALUE == _h) return false;

	BOOL ret = CloseHandle(_h);
	if( ret )
	{
		_h = INVALID_HANDLE_VALUE;
	}
	else
	{
		assert(0);
	}

	return ret == TRUE;
}

bool WINFile::trunc()
{
	if(INVALID_HANDLE_VALUE == _h) return false;

	seek(0, s_set);
	return SetEndOfFile(_h) == TRUE;
}

unsigned long WINFile::read(void *buf, unsigned long len)
{
	if(INVALID_HANDLE_VALUE == _h) return 0;

	unsigned long bytesRd = 0;
	if(!ReadFile(_h, buf, len, &bytesRd, NULL))
	{
		assert(0);
	}

	return bytesRd;
}

unsigned long WINFile::write(const void *buf, unsigned long len)
{
	if(INVALID_HANDLE_VALUE == _h) return 0;

	unsigned long bytesWr = 0;
	if(!WriteFile(_h, buf, len, &bytesWr, NULL))
	{
		assert(0);
	}

	return bytesWr;
}

__int64 WINFile::tell()
{
	if(INVALID_HANDLE_VALUE == _h) return -1;

	LARGE_INTEGER pos;
	LARGE_INTEGER dis;
	dis.QuadPart = 0;
	pos.QuadPart = 0;
	if(!SetFilePointerEx(_h, dis, &pos, FILE_CURRENT))
	{
		assert(0);
	}

	return pos.QuadPart;
}

__int64 WINFile::seek(__int64 off, DWORD mode)
{
	if(INVALID_HANDLE_VALUE == _h) return -1;

	LARGE_INTEGER pos;
	LARGE_INTEGER dis;
	dis.QuadPart = off;
	pos.QuadPart = 0;
	if(!SetFilePointerEx(_h, dis, &pos, mode))
	{
		assert(0);
	}

	return pos.QuadPart;
}

__int64 WINFile::size()
{
	if(INVALID_HANDLE_VALUE == _h) return -1;

	LARGE_INTEGER sz;
	sz.QuadPart = 0;

	if(!GetFileSizeEx(_h, &sz))
	{
		assert(0);
	}

	return sz.QuadPart;
}

bool WINFile::eof()
{
	if(INVALID_HANDLE_VALUE == _h) return true;

	return tell() >= size();
}

bool WINFile::fileExist(const char* fileName)
{
	WIN32_FIND_DATA wfd;
	SStringT nameT = S_CA2T(fileName);
	HANDLE hHandle = ::FindFirstFile(nameT.GetBuffer(nameT.GetLength()), &wfd);
	if (hHandle == INVALID_HANDLE_VALUE)
		return false;
	else
		return (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;

}

bool WINFile::directoryExist(const char* dir)
{
	WIN32_FIND_DATA wfd;
	SStringT dirT = S_CA2T(dir);
	HANDLE hHandle = ::FindFirstFile(dirT.GetBuffer(dirT.GetLength()), &wfd);
	if (hHandle == INVALID_HANDLE_VALUE)
		return access(dir, 0) == 0; // if dir is a drive disk path like c:\,we thought is a directory too.  	
	else
		return (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool WINFile::createDirectory(const char* pathName)
{
	char path[MAX_PATH] = { 0 };
	const char* pos = pathName;
	while ((pos = strchr(pos, '\\')) != NULL)
	{
		memcpy(path, pathName, pos - pathName + 1);
		pos++;
		if (directoryExist(path))
		{
			continue;
		}
		else
		{
			int ret = _mkdir(path);
			if (ret == -1)
			{
				return false;
			}
		}
	}
	pos = pathName + strlen(pathName) - 1;
	if (*pos != '\\')
	{
		return _mkdir(pathName) == 0;
	}
	return true;
}

bool WINFile::createFileWithDirectory(const char* pathName)
{
	if (fileExist(pathName))
		return true;
	int len = strlen(pathName);
	if (len <= 0)
		return false;

	char strTmpPath[MAX_PATH] = { 0 };
	strcpy(strTmpPath, pathName);
	char* q = strTmpPath + len - 1;
	for (int i = 0; i < len - 1; i++, q--)
	{
		if (*q == '\\')
		{
			*q = '\0';
			q++;
			break;
		}
	}
	if (strlen(strTmpPath) > 0 && strlen(q) > 0)
	{
		createDirectory(strTmpPath);
		FILE* hFile = fopen(pathName, "w");
		if (hFile)
		{
			fclose(hFile);
			return true;
		}
		else
			return false;

	}
	else
	{
		return false;
	}

}