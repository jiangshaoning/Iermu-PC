/* Copyright (C) 2012 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once

#ifndef _MEMFILE_HEADER_PROTECT_
#define _MEMFILE_HEADER_PROTECT_

/*
memfile.h

�ڴ滺��,���Զ�д.
ģ�±�׼�ļ��ӿ�
�������ڶ�д�ֱ��ж�����ָ��, read() �� write() ����ִ��ʱ,����Ҫʹ�� seek() ���ƶ�.

by ������ - Que's C++ Studio
2010-12-16

*/

class memfile
{
private:
	char *_buffer;
	size_t _bufLen;

	size_t _readPos;
	size_t _writePos;
	size_t _fileSize;

	size_t _maxSize;
	size_t _memInc;
	bool _selfAlloc;

	size_t space();
	size_t reserve(size_t s);

	memfile(const memfile &other);
	memfile& operator = (const memfile &other);

public:
	memfile(size_t memInc = 1024, size_t maxSize = SIZE_T_MAX);
	memfile(const void* buf, size_t len);
	~memfile();

	size_t write(const void *buf, size_t len); // ����д����ֽ���
	size_t puts(const char* buf); // ����д����ֽ���,����������0
	size_t putc(const char ch);
	size_t seekp(long offset, int origin); // ����0 ��ʾ�ɹ�.
	size_t tellp() const;

	size_t read(void *buf, size_t size); // ���ض�ȡ�ֽ���
	char getc();
	size_t gets(char *buf, size_t size); // ���ض�ȡ���ֽ���,�������з�
	size_t seekg(long offset, int origin);
	size_t tellg() const;
	
	void* buffer();
	inline size_t bufferSize()  const { return _bufLen; }

	inline size_t fsize() const { return _fileSize; }
	void trunc(bool freeBuf = true);
	bool eof() const;
	
	bool reserve(size_t r, void **buf, size_t *len);
};

#endif