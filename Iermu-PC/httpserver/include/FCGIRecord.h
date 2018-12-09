/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once
#include <list>
#include "memfile.h"
#include "fastcgi.h"
/*
* ��װFastCGI Record, FastCGI Record �� FastCGI Э��Ļ���ͨѶ��λ.
* һ�� Record �Ľṹ����:
	typedef struct {
		unsigned char version;
		unsigned char type;
		unsigned char requestIdB1;
		unsigned char requestIdB0;
		unsigned char contentLengthB1;
		unsigned char contentLengthB0;
		unsigned char paddingLength;
		unsigned char reserved;

		unsigned char contentData[contentLength];
		unsigned char paddingData[paddingLength];
	} FCGI_Record;
* ǰ8���ֽ��� FCGI_Header �ṹ, ����������� FCGI_Header ָ�������ݳ��� contentLength �� ���볤�� paddingLength ָ��.
* ������Ϊ����һ�α䳤���ڴ滺��.
* һЩ�ض����͵� Record contentData ���ض���Ԥ����ṹ,�� FCGI_BeginRequestBody, FCGI_EndRequestBody �� FCGI_UnknownTypeBody.
*
* class FCGIRecord ��Ŀ���Ƿ�װFastCGI record�Է����̷���. ���� reader �� writer �ֱ����ڽ��������� FastCGI record.
*/

/* 
* Name - Value pair 
*/
typedef struct 
{ 
	const unsigned char* data; 
	size_t len; 
}nv_t;
typedef std::pair<nv_t, nv_t> nvpair_t;
typedef std::list<nvpair_t> nvlist_t;

class FCGIRecord : public INoCopy
{
private:
	memfile _buffer; /* record ԭʼ���� */
	
public:
	FCGIRecord();
	~FCGIRecord();

	static size_t toNumber2(const unsigned char* src);
	static size_t toNumber14(const unsigned char* src, size_t *bytes);
	static void toBytes(void* dest, size_t number, size_t bytes);

	size_t write(const void* buf, size_t len); /* ���Էֶ��д��һ��������record,����ֵС�� len �����Ѿ����յ�һ��������¼ */
	size_t read(void* buf, size_t len); /* ���Ϊ��,���Էֶε�������ֱ������0��ʾ������. */
	size_t writeHeader(const void *buf, size_t len);
	void reset();
	bool check(); /* ����Ƿ���һ�������� record */
	const void* buffer();
	size_t size();
	
	bool setHeader(unsigned short requestId, int type);
	bool setBeginRequestBody(unsigned short role, bool keepConn); /* FCGI_BEGIN_REQUEST */
	bool setEndRequestBody(unsigned int appStatus, unsigned char protocolStatus); /* FCGI_END_REQUEST */
	bool setUnknownTypeBody(); /* FCGI_UNKNOWN_TYPE */
	bool addNameValuePair(nv_t n, nv_t v); /* FCGI_PARAMS,FCGI_GET_VALUES,FCGI_GET_VALUES_RESULT */
	bool addNameValuePair(const char* n, const char* v);
	bool addBodyData(unsigned char* buf, size_t len); /* ͨ��,FCGI_STDIN,FCGI_STDOUT,FCGI_STDERR,FCGI_DATA*/
	bool setEnd(); /* ��� */

	unsigned char getType();
	bool getHeader(FCGI_Header &header);
	size_t getContentLength(const FCGI_Header &header);
	bool getBeginRequestBody(unsigned short &role, bool &keepConn); /* FCGI_BEGIN_REQUEST */
	bool getEndRequestBody(unsigned int &appStatus, unsigned char &protocolStatus); /* FCGI_END_REQUEST */
	size_t getNameValuePairCount();
	bool getNameValuePair(int index, nv_t &n, nv_t &v); /* FCGI_PARAMS,FCGI_GET_VALUES,FCGI_GET_VALUES_RESULT */
	const void* getBodyData(); /* ͨ��,FCGI_STDIN,FCGI_STDOUT,FCGI_STDERR,FCGI_DATA*/
	size_t getBodyLength();
};

class FCGIRecordWriter : public INoCopy
{
private:
	memfile &_buf;
	size_t _headerPos;

	size_t write(const void *buf, size_t len);
public:
	FCGIRecordWriter(memfile &buf);
	~FCGIRecordWriter();

	size_t writeHeader(unsigned short requestId, int type);
	size_t writeBeginRequestBody(unsigned short role, bool keepConn); /* FCGI_BEGIN_REQUEST */
	size_t writeEndRequestBody(unsigned int appStatus, unsigned char protocolStatus); /* FCGI_END_REQUEST */
	size_t writeUnknownTypeBody(unsigned char t); /* FCGI_UNKNOWN_TYPE */
	size_t writeNameValuePair(const char* name, const char* val);
	size_t writeBodyData(const unsigned char* buf, size_t len); /* ͨ��,FCGI_STDIN,FCGI_STDOUT,FCGI_STDERR,FCGI_DATA*/
	size_t writeEnd();
};

class FCGIRecordReader : public INoCopy
{
private:
	const void* _buffer;
	size_t _len;
	size_t _pos;

	size_t read(void* dest, size_t len);
	size_t putback(size_t len);
	size_t space();
public:
	FCGIRecordReader(const void *buf, size_t len);
	~FCGIRecordReader();

	size_t pos();
	size_t readHeader(unsigned char &t, unsigned short &requestId, unsigned short &contentLength);
	size_t readHeader(FCGI_Header &header);
	size_t readBeginRequestBody(unsigned short &role, bool &keepConn); /* FCGI_BEGIN_REQUEST */
	size_t readEndRequestBody(unsigned int &appStatus, unsigned char &protocolStatus); /* FCGI_END_REQUEST */
	size_t readNameValuePair(const char* &n, size_t &nlen, const char* &val, size_t &vlen); /* FCGI_PARAMS,FCGI_GET_VALUES,FCGI_GET_VALUES_RESULT */
	size_t readBodyData(void* buf, size_t len); /* ͨ��,FCGI_STDIN,FCGI_STDOUT,FCGI_STDERR,FCGI_DATA*/
};