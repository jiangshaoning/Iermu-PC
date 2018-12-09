/* Copyright (C) 2012 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once
#include <list>
#include "HTTPDef.h"
#include "memfile.h"
#include "HTTPResponseHeader.h"
#include "HTTPRequest.h"
#include "FCGIFactory.h"
#include "FCGIRecord.h"
#include "HTTPResponder.h"
#include "FCGICache.h"

/* 
* FCGI Э��� Responder, FCGIResponder ���� HTTPRequest Ϊ����,���ƺ�FCGI���������ͨ��,������Ӧ��ת���� HTTP �ͻ���
*
*/

class FCGIResponder : public IResponder, public INoCopy
{
protected:
	/* 
	* Responder ���� 
	*/
	HTTPResponseHeader _header;
	IRequest *_request;
	IHTTPServer *_server;
	IOCPNetwork *_network;
	iocp_key_t _clientSock;
	conn_id_t _connId;
	__int64 _bytesSent;
	__int64 _bytesRecv;
	int _svrCode;

	/* 
	* Fast CGI ר�� 
	*/
	typedef struct buffer_t
	{
		byte* buf;	// ����
		size_t len;	// ��Ч���ݳ���
		size_t size; // �����С
	};
	__int64 _bytesFCGISent;
	__int64 _bytesFCGIRecv;
	FCGIFactory *_fcgiFactory;
	fcgi_conn_t *_fcgiConnection;
	bool _cacheAll;
	FCGICache *_cache;
	Lock _lock;
	
	/*
	* 3��������
	* 1. HTTPRequest -> FCGI Server
	* 2. FCGI Server -> Cache
	* 3. Cache -> HTTP Client
	*
	* ������1����Զ�����,ֻ����������1��ɺ�,������������2��������3.
	* ������2,3����ִ��.
	* ��������3���ٶȳ�������2���ٶ�ʱ,������3���ܻ���ͣ,������Ҫ������2��ͣ��������.
	*/
	int _exitCode;

	/* ������1: HTTPRequest -> FCGI Server */
	memfile *_fcgiSendBuf; /* ������1������,����ʹ�ñ䳤����,��Ϊ����Ҫ������������ */
	buffer_t *_postDataBuf; /* POST ���ݻ�����*/
	int sendPostData(); /* ��ȡ HTTPRequest �е� POST DATA */
	int sendToFCGIServer();

	/* ������2: FCGI Server -> Cache */
	int _ds2Status; /* ������2��״̬: ���,���� */
	buffer_t *_fcgiRecvBuf;
	bool _isFCGIHeaderReceived;
	bool _chunkCoding;
	bool _chunkEndSent;
	FCGIRecord *_stdoutRecord;
	memfile _fcgiResponseHeader; /* ���淢��FCGI server��HTTP��Ӧͷ���� */
	int recvFromFCGIServer();
	bool parseStdout(const byte *buf, size_t len); /* ��FCGI�յ������ǵ��÷����Ƿ��յ� FCGI_END_REQUEST */
	size_t writeToCache(const void *data, size_t len); /* �ѷ��͵� HTTP �ͻ��˵�����д�뻺��,����д����ֽ��� */

	/* ������3: Cache -> HTTP */
	int _ds3Status; /* ������3��״̬: ���,���� */
	buffer_t *_httpSendBuf;
	int sendToHTTPClient();
	bool hasData(); /* �Ƿ���������Ҫ���͵� HTTP �ͻ��� */

	/* ��� FCGI ����������,���һ�û�з��͹��κ����ݵ� HTTP �ͻ���,���� HTTP 503/500 */
	memfile* _httpErrorCache;
	int sendHttpError(int errCode, const char *msg);
	

	/*
	* �ڲ�����,��Щ����ֻ�ǹ��ߺ���,����״ֵ̬(HTTP_CONNECTION_EXITCODE),����¼״̬,���ص�,������.
	*/
	buffer_t* allocBuffer();
	void freeBuffer(buffer_t *buf);

	int initFCGIEnv();

	bool isExitPoint();
	void close(int exitCode);
protected:
	/* 
	* �����¼��ص� 
	*/
	static void IOCPCallback(iocp_key_t s, int flags, bool result, int transfered, byte* buf, size_t len, void* param);
	static void onFcgiConnectionReady(fcgi_conn_t *conn, void *param);

	void onConnection(fcgi_conn_t *conn);
	void onHTTPSend(size_t bytesTransfered, int flags);
	void onFCGISend(size_t bytesTransfered, int flags);
	void onFCGIRecv(size_t bytesTransfered, int flags);

public:
	FCGIResponder(IHTTPServer *server, IOCPNetwork *network, FCGIFactory *fcgiFactory);
	~FCGIResponder();

	/* 
	* �������� 
	*/
	inline conn_id_t getConnectionId() { return _connId; }
	inline __int64 getTotalSentBytes() { return _bytesSent; }
	inline __int64 getTotalRecvBytes() { return _bytesRecv; }
	inline int getServerCode() { return _svrCode; }

	/*
	* ������Ӧͷ
	*/
	std::string getHeader();

	/* 
	* �������� 
	*/
	int run(conn_id_t connId, iocp_key_t clientSock, IRequest *request);
	bool stop(int ec);
	bool reset();
};

