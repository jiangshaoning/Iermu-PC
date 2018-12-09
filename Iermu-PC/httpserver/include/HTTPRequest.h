/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once

#include "HTTPDef.h"
#include "HTTPContent.h"
#include "memfile.h"
#include "IOCPNetwork.h"

/*
* HTTP Э���е�"����"���ĵķ�װ
*
* HTTPRequest ����ʵ���� IRequest �ӿ�,��ʼ���к�ӿͻ��˶�ȡ���� HTTP������,��������ͷ�� POST ����
* 
*/

class HTTPRequest : public IRequest, public INoCopy
{
protected:
	HighResolutionTimer _hrt;
	memfile _header;
	memfile *_postData;
	WINFile *_postFile;
	std::string _postFileName;
	bool _isHeaderRecved;
	std::string _boundary;
	int _ignoreLength; //��������ͷ��boundary�����Ĳ��֣���������Ҫ���ļ�
	IHTTPServer *_server;
	IOCPNetwork *_network;
	iocp_key_t _clientSock;
	byte* _sockBuf;
	size_t _sockBufLen;
	conn_id_t _connId;
	size_t _bytesRecv;
	size_t _contentLength;
	__int64 _startTime;
	int _headDeviation;

	void deleteSocketBuf();
	void close(int exitCode);
	size_t push(const byte* data, size_t len); // �׽����յ����ݺ�,���뵽 HTTP Request ʵ����.
	void onRecv(int flags, size_t bytesTransfered);
	static void IOCPCallback(iocp_key_t s, int flags, bool result, int transfered, byte* buf, size_t len, void* param);

public:
	HTTPRequest(IHTTPServer *server, IOCPNetwork *network);
	virtual ~HTTPRequest();

	inline conn_id_t getConnectionId() { return _connId; }
	inline size_t getTotalRecvBytes() { return _bytesRecv; }
	HTTP_METHOD method(); // ����HTTP ����
	std::string uri(bool decode); // ���ؿͻ�������Ķ���(�Ѿ�����UTF8����,���Է��ؿ��ַ���)
	std::string field(const char* key); // ��������ͷ�е�һ���ֶ�(HTTPͷ��ֻ��ANSI�ַ�,���Է���string).
	bool range(__int64 &from, __int64 &to);
	bool keepAlive();
	size_t contentLength(); /* ����ͷ�е� Content-Length �ֶε�ֵ */
	__int64 startTime() { return _startTime; }

	std::string getHeader();

	bool isValid();
	bool isBoundary();
	size_t headerSize();
	size_t size();
	size_t read(byte* buf, size_t len);
	bool eof();
	
	int run(conn_id_t connId, iocp_key_t clientSock, size_t timeout);
	bool stop(int ec);
	bool reset();
};
