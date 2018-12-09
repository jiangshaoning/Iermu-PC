/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once

/*
* HTTPContent
* ������װ HTTP ��Ӧ�����е����ݲ���,�п��ܰ������¼�����������:
* 1. һ���ı�,�� HTTP 500 �ȴ�����ʾ.
* 2. һ��Ŀ¼�ı���,���Ŀ¼ʱ�õ����ļ��б�.
* 3. һ��ֻ���ļ��Ĳ��ֻ���ȫ��.
*
*/

#include "HTTPDef.h"
#include "memfile.h"

#define OPEN_NONE 0
#define OPEN_FILE 1
#define OPEN_TEXT 2
#define OPEN_BINARY 3
#define OPEN_HTML 4
#define OPEN_DIR 5


class HTTPContent : public INoCopy
{
public:
	HTTPContent();
	virtual ~HTTPContent();

protected:
	int _openType;
	std::string _contentType;

	std::string _fileName;
	WINFile _file;
	struct _stat32i64 _fileInf;

	memfile _memfile;
	
	__int64 _from;
	__int64 _to;

	std::string getContentTypeFromFileName(const char* fileName);
	bool writable();

public:
	bool open(const std::string &fileName, __int64 from = 0, __int64 to = 0); /* ��һ��ֻ���ļ� */
	bool open(const std::string &urlStr, const std::string &filePath); /* ��һ��Ŀ¼ */
	bool open(const char* buf, int len, int type); /* ��һ�� mem buffer */
	void close();

	std::string contentType();
	__int64 contentLength();
	std::string lastModified();
	std::string etag();
	std::string contentRange();
	
	bool isFile();
	bool isOpen();
	bool eof();
	
	size_t read(void* buf, size_t len);
	size_t write(const void* buf, size_t len);
	size_t writeString(const char* str);
};
