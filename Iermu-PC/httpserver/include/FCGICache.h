/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once
#include "WINFile.h"
#include <string>

/*
* ��Ϊ FCGIResponder �Ļ���
* һ���̴߳� FCGI ���������յ����ݺ�д��,��һ���̴߳ӻ����ж�ȡ.
* �ڲ�ά��һ��̶����ȵ��ڴ�,������ݳ��ȳ���,��ʹ���ļ�ϵͳ.
*
*/

class FCGICache : public INoCopy
{
private:
	size_t _size;

	/*
	* �ļ�������
	*/
	WINFile *_file;
	std::string _fileName;
	long _frpos;
	long _fwpos;

	/*
	* �ڴ滺����
	* �Զ�����ֱ�����ֵ,Ȼ����д�ļ�.
	*/
	byte *_buf;
	size_t _bufSize;
	size_t _rpos;
	size_t _wpos;
	
	size_t fillBuf(); /* ����ʱ�ļ��ж�ȡ������仺���� */
public:
	FCGICache(size_t bufSize, const std::string &tmpFileName);
	~FCGICache();

	bool empty();
	size_t size();
	size_t read(void *buf, size_t len);
	size_t write(const void *buf, size_t len);
	size_t puts(const char *str);
};

