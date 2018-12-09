/* Copyright (C) 2012 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once
#include "HTTPDef.h"
#include "memfile.h"
#include "HTTPResponseHeader.h"
#include "HTTPRequest.h"

/*
* HTTPResponder ���ǽӿ� IResponder ��ʵ��,��װ�� HTTP Э�����Ӧ����.
* HTTPResponder ���� IRequest HTTP ������Ϊ����,���ɶ�Ӧ����Ӧ����,�������ݷ��͵� HTTP �ͻ���.
* 
*/

class HTTPContent;
class HTTPResponder : public IResponder, public INoCopy
{
protected:
	HTTPResponseHeader _header;
	IRequest *_request;
	IHTTPServer *_server;
	IOCPNetwork *_network;
	iocp_key_t _clientSock;
	conn_id_t _connId;
	__int64 _bytesSent;
	__int64 _bytesRecv;
	int _svrCode;
	HTTPContent* _content;
	byte* _sockBuf;
	size_t _sockBufLen;

	static void IOCPCallback(iocp_key_t s, int flags, bool result, int transfered, byte* buf, size_t len, void* param);
	void onSend(size_t bytesTransfered, int flags);
	
	bool makeResponseHeader(int svrCode);
	bool sendToClient();
	void close(int ct);

public:
	HTTPResponder(IHTTPServer *server, IOCPNetwork *network);
	virtual ~HTTPResponder();

	inline conn_id_t getConnectionId() { return _connId; }
	inline __int64 getTotalSentBytes() { return _bytesSent; }
	inline __int64 getTotalRecvBytes() { return _bytesRecv; }
	inline int getServerCode() { return _svrCode; }
	
	/*
	* ������Ӧͷ
	*/
	std::string getHeader();

	int run(conn_id_t connId, iocp_key_t clientSock, IRequest *request);
	bool stop(int ec);
	bool reset();
};