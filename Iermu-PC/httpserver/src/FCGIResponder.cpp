#include "StdAfx.h"
#include "FCGIResponder.h"

#define FCGI_INIT_TMPBUF_SIZE 2048

#define DSS_NONE 0 /* ������״̬: δ���� */
#define DSS_RUNING 0x01 /* ������״̬: �������� */
#define DSS_ABORTED 0x02 /* ������״̬: �ѳ���ֹͣ */
#define DSS_COMPLETE 0x04 /* ������״̬: ����� */
#define DSS_PAUSED 0x08 /* ������״̬: ��ͣ */

FCGIResponder::FCGIResponder(IHTTPServer *server, IOCPNetwork *network, FCGIFactory *fcgiFactory)
	: _request(NULL), _bytesRecv(0), _bytesSent(0), _isFCGIHeaderReceived(false),
	_server(server), _network(network), _clientSock(IOCP_NULLKEY), _connId(NULL), _svrCode(SC_UNKNOWN),
	_fcgiFactory(fcgiFactory), _bytesFCGISent(0), _bytesFCGIRecv(0), _fcgiConnection(NULL), _cacheAll(false),
	_fcgiRecvBuf(NULL), _fcgiSendBuf(NULL), _postDataBuf(NULL), _stdoutRecord(NULL), _chunkCoding(false), _chunkEndSent(false),
	_httpSendBuf(NULL), _cache(NULL), _httpErrorCache(NULL), _exitCode(CT_UNKNOWN)
{

}

FCGIResponder::~FCGIResponder()
{

}

FCGIResponder::buffer_t* FCGIResponder::allocBuffer()
{
	buffer_t *buf = new buffer_t;
	buf->buf = new byte[FCGI_BUFFER_SIZE];
	buf->len = 0;
	buf->size = FCGI_BUFFER_SIZE;
	return buf;
}

void FCGIResponder::freeBuffer(buffer_t *buf)
{
	if( buf )
	{
		if( buf->buf ) delete []buf->buf;
		delete buf;
	}
}

/*
* ��ں���
*/
int FCGIResponder::run(conn_id_t connId, iocp_key_t clientSock, IRequest *request)
{
	/* ������� */
	_connId = connId;
	_clientSock = clientSock;
	_request = request;
	
	/* ������ */
	assert(_fcgiFactory);
	fcgi_conn_t *conn = NULL;
	int ret = CT_SUCESS;

	/* ��ȡ FCGI ���� */
	if( _fcgiFactory->getConnection(conn, onFcgiConnectionReady, this))
	{
		if(conn != NULL)
		{
			/* ���һ�����õ����� */
			_fcgiConnection = conn;
			
			/* ׼�����ӵ�FCGI������(Զ��)���߷���FCGI_BEGIN_REQUEST(����) */
			ret = initFCGIEnv();
		}
		else
		{
			/* �ȴ��ص� */
			ret = CT_SUCESS;
		}
	}
	else
	{
		ret = CT_INTERNAL_ERROR;
	}

	if(CT_SUCESS != ret)
	{
		reset();
	}
	return ret;
}

bool FCGIResponder::stop(int ec)
{
	return false;
}

bool FCGIResponder::reset()
{
	/*
	* ������Դ
	*/
	if(_fcgiConnection)
	{
		_fcgiFactory->releaseConnection(_fcgiConnection, false);
		_fcgiConnection = NULL;
	}

	_connId = INVALID_CONNID;

	freeBuffer(_fcgiRecvBuf);
	freeBuffer(_httpSendBuf);
	freeBuffer(_postDataBuf);
	if( _fcgiSendBuf ) delete _fcgiSendBuf;
	_postDataBuf = NULL;
	_fcgiSendBuf = NULL;
	_fcgiRecvBuf = NULL;
	_httpSendBuf = NULL;

	if(_stdoutRecord)
	{
		delete _stdoutRecord;
		_stdoutRecord = NULL;
	}

	if(_cache)
	{
		delete _cache;
		_cache = NULL;
	}

	if(_httpErrorCache)
	{
		delete _httpErrorCache;
		_httpErrorCache = NULL;
	}

	_bytesFCGIRecv = 0;
	_bytesFCGISent = 0;
	_bytesRecv = 0;
	_bytesSent = 0;

	return true;
}

/*
* ��ʼ�� FCGI �����л���
*/
int FCGIResponder::initFCGIEnv()
{
	/*
	* ��ʼ���ڲ�״̬
	* ���仺��
	* ����������1 [HTTP -> FCGI] ������
	*/
	assert(_cache == NULL && _fcgiSendBuf == NULL);
	_cacheAll = _fcgiConnection->cacheAll;
	if(_cache == NULL)
	{
		_cache = new FCGICache(FCGI_CACHE_SIZE, _server->tmpFileName());
	}
	if(_fcgiSendBuf == NULL)
	{
		_fcgiSendBuf = new memfile();
	}
	
	/* 
	* ׼������ 
	*/
	std::string tmp;
	char numberBuf[50] = {0};

	/* ����uri��Ϣ */
	std::string uri = _request->uri(false);
	std::string uriPath(""), uriQueryString("");
	std::string uriServerPath;
	std::string::size_type pos = uri.find('?');
	if(pos == std::string::npos)
	{
		uriPath = uri;
	}
	else
	{
		uriPath = uri.substr(0, pos);
		uriQueryString = uri.substr(pos + 1, uri.size() - pos - 1);
	}

	if(!_server->mapServerFilePath(uriPath, uriServerPath))
	{
		assert(0);
		return CT_INTERNAL_ERROR;
	}

	/* ��ȡ���ӵ�ַ */
	sockaddr_in svrAddr, clientAddr;
	memset(&svrAddr, 0, sizeof(sockaddr_in));
	memset(&clientAddr, 0, sizeof(sockaddr_in));
	int nameLen = sizeof(sockaddr_in);
	if(SOCKET_ERROR == getsockname(_network->getSocket(_clientSock), (sockaddr*)&svrAddr, &nameLen))
	{
		assert(0);
	}
	nameLen = sizeof(sockaddr_in);
	if(SOCKET_ERROR == getpeername(_network->getSocket(_clientSock), (sockaddr*)&clientAddr, &nameLen))
	{
		assert(0);
	}

	/* HTTP���� */
	char method[50] = {0};
	map_method(_request->method(), method);

	/* SERVER_NAME - HOST */
	std::string hostName = _request->field("Host");
	if( hostName.size() <= 0)
	{
		hostName = _server->ip();
	}
	else
	{
		std::string::size_type pos = hostName.find(':');
		if(pos != std::string::npos)
		{
			hostName = hostName.substr(0, pos);
		}
	}

	/* 
	* ׼�������� 
	*/
	FCGIRecordWriter writer(*_fcgiSendBuf);

	/* ���� FCGI_BEGIN_REQUEST */
	writer.writeHeader(_fcgiConnection->requestId, FCGI_BEGIN_REQUEST);
	writer.writeBeginRequestBody(FCGI_RESPONDER, true);
	writer.writeEnd();

	/* ���Ͳ��� */
	writer.writeHeader(_fcgiConnection->requestId, FCGI_PARAMS);
	writer.writeNameValuePair("HTTPS", "off");
	writer.writeNameValuePair("REDIRECT_STATUS", "200");
	writer.writeNameValuePair("SERVER_PROTOCOL", "HTTP/1.1");
	writer.writeNameValuePair("GATEWAY_INTERFACE", "CGI/1.1");
	writer.writeNameValuePair("SERVER_SOFTWARE", SERVER_SOFTWARE);
	writer.writeNameValuePair("SERVER_NAME", hostName.c_str());
	writer.writeNameValuePair("SERVER_ADDR", inet_ntoa(svrAddr.sin_addr));
	writer.writeNameValuePair("SERVER_PORT", itoa(ntohs(svrAddr.sin_port), numberBuf, 10));
	writer.writeNameValuePair("REMOTE_ADDR", inet_ntoa(clientAddr.sin_addr));
	writer.writeNameValuePair("REMOTE_PORT", itoa(ntohs(clientAddr.sin_port), numberBuf, 10));
	writer.writeNameValuePair("REQUEST_METHOD", method);
	writer.writeNameValuePair("REQUEST_URI", uri.c_str());
	if(uriQueryString.size() > 0) writer.writeNameValuePair("QUERY_STRING",uriQueryString.c_str());

	writer.writeNameValuePair("DOCUMENT_ROOT", _server->docRoot().c_str());
	writer.writeNameValuePair("SCRIPT_NAME", uriPath.c_str());
	writer.writeNameValuePair("SCRIPT_FILENAME", uriServerPath.c_str());
	//writer.writeNameValuePair("PATH_INFO", uriServerPath.c_str());
	//writer.writeNameValuePair("PATH_TRANSLATED", uriServerPath.c_str());

	writer.writeNameValuePair("HTTP_HOST", _request->field("Host").c_str());
	writer.writeNameValuePair("HTTP_USER_AGENT", _request->field("User-Agent").c_str());
	writer.writeNameValuePair("HTTP_ACCEPT", _request->field("Accept").c_str());
	writer.writeNameValuePair("HTTP_ACCEPT_LANGUAGE", _request->field("Accept-Language").c_str());
	writer.writeNameValuePair("HTTP_ACCEPT_ENCODING", _request->field("Accept-Encoding").c_str());

	tmp = _request->field("Cookie");
	if(tmp.size() > 0)
	{
		writer.writeNameValuePair("HTTP_COOKIE", tmp.c_str());
	}
	
	tmp = _request->field("Referer");
	if(tmp.size() > 0)
	{
		writer.writeNameValuePair("HTTP_REFERER", tmp.c_str());
	}

	tmp = _request->field("Content-Type");
	if(tmp.size() > 0)
	{
		writer.writeNameValuePair("CONTENT_TYPE", tmp.c_str());
	}

	tmp = _request->field("Content-Length");
	if(tmp.size() > 0)
	{
		writer.writeNameValuePair("CONTENT_LENGTH", tmp.c_str());
	}
	writer.writeEnd();

	/* �ռ�¼��ʾ���� */
	writer.writeHeader(_fcgiConnection->requestId, FCGI_PARAMS);
	writer.writeEnd();

	/* ��� HTTPRequest ���󲻰��� POST ����,ֱ�ӷ���һ��������׼ */
	if(_request->contentLength() == 0)
	{
		writer.writeHeader(_fcgiConnection->requestId, FCGI_STDIN);
		writer.writeEnd();
	}
	
	/*
	* �Ѳ������͵� FCGI ������
	*/
	return sendToFCGIServer();
}

void FCGIResponder::IOCPCallback(iocp_key_t s, int flags, bool result, int transfered, byte* buf, size_t len, void* param)
{
	FCGIResponder *instPtr = reinterpret_cast<FCGIResponder*>(param);
	
	if( s == instPtr->_clientSock )
	{
		assert(flags & IOCP_SEND);
		instPtr->onHTTPSend(transfered, flags);
	}
	else
	{
		if( flags & IOCP_RECV )
		{
			instPtr->onFCGIRecv(transfered, flags);
		}
		else
		{
			instPtr->onFCGISend(transfered, flags);
		}
	}
}

int FCGIResponder::sendToFCGIServer()
{
	assert(_fcgiSendBuf);
	if( _fcgiConnection->comm == IOCP_NULLKEY) return CT_UNKNOWN;

	if(IOCP_PENDING != _network->send(_fcgiConnection->comm, 
		reinterpret_cast<const byte*>(_fcgiSendBuf->buffer()) + _fcgiSendBuf->tellg(), 
		_fcgiSendBuf->fsize() - _fcgiSendBuf->tellg(), 
		_server->sendTimeout(), IOCPCallback, this))
	{
		DWORD lastErrorCode = _network->getLastError();
		return CT_FCGI_SEND_FAILED;
	}
	else
	{
		return CT_SUCESS;
	}
}

/*
* �ӻ����ж�ȡ���ݲ��ҷ��͵� HTTP �ͻ���
*/
int FCGIResponder::sendToHTTPClient()
{
	assert(_httpSendBuf);
	int exitCode = CT_SUCESS;

	/* ��ȡ��Ӧͷ */
	_httpSendBuf->len += _header.read(_httpSendBuf->buf + _httpSendBuf->len, _httpSendBuf->size - _httpSendBuf->len);

	/* ��ȡ���� */
	_httpSendBuf->len += _cache->read(_httpSendBuf->buf + _httpSendBuf->len, _httpSendBuf->size - _httpSendBuf->len);

	/* ���͵��ͻ��� */
	if(_httpSendBuf->len > 0)
	{
		int netIoRet = _network->send(_clientSock, _httpSendBuf->buf, _httpSendBuf->len, _server->sendTimeout(), IOCPCallback, this);
		if( IOCP_PENDING != netIoRet )
		{
			exitCode = (netIoRet == IOCP_SESSIONTIMEO ? CT_SESSION_TIMEO : CT_CLIENTCLOSED);
		}
		else
		{
			exitCode = CT_SUCESS;
		}
	}
	else
	{
		assert(0);
	}
	return exitCode;
}

int FCGIResponder::recvFromFCGIServer()
{
	assert(_fcgiRecvBuf && _fcgiRecvBuf->buf);

	if( IOCP_PENDING != _network->recv(_fcgiConnection->comm, _fcgiRecvBuf->buf, _fcgiRecvBuf->size, 
		_server->recvTimeout(), IOCPCallback, this))
	{
		return CT_FCGI_RECV_FAILED;
	}
	else
	{
	}
	return CT_SUCESS;
}

void FCGIResponder::onConnection(fcgi_conn_t *conn)
{
	if(conn)
	{
		/* �����һ�����õ����� */
		_fcgiConnection = conn;

		/* ׼�����ӵ�FCGI������(Զ��)���߷���FCGI_BEGIN_REQUEST(����) */
		int res = initFCGIEnv();
		if(CT_SUCESS != res)
		{
			close(res);
		}
	}
	else
	{
		/* �޷�������� ���ʹ��� 503  */
		int res = sendHttpError(SC_SERVERBUSY, g_HTTP_Server_Busy);
		if(CT_SUCESS != res) close(res);
	}
}

void FCGIResponder::onFcgiConnectionReady(fcgi_conn_t *conn, void *param)
{
	FCGIResponder *instPtr = reinterpret_cast<FCGIResponder*>(param);
	instPtr->onConnection(conn);
}

/*
* ���ٽ�˳�����,����Ѿ�����������3ֹͣ����,�Ͳ����ٷ����κ����Ա,��Ϊ��һ���߳̿��ܻ�ɾ����ʵ��.
* ֻ��ʹ�ú����ڵľֲ�����(ջ�ڷ���).
*/
void FCGIResponder::onHTTPSend(size_t bytesTransfered, int flags)
{
	/* ���� HTTP Error */
	if(_httpErrorCache != NULL)
	{
		if(0 == bytesTransfered)
		{
			close((flags & IOCP_WRITETIMEO) ? CT_SEND_TIMEO : CT_CLIENTCLOSED);
		}
		else
		{
			/* ״̬�ص� */
			_server->onResponderDataSent(this, bytesTransfered);
			_bytesSent += bytesTransfered;

			/* �ѻ�������δ���͵������Ƶ���ͷ */
			_httpSendBuf->len -= bytesTransfered;
			if( _httpSendBuf->len > 0 )
			{
				memmove(_httpSendBuf->buf, _httpSendBuf->buf + bytesTransfered, _httpSendBuf->len);
			}

			/* �ӻ����ж�ȡ��һ������ */
			_httpSendBuf->len += _httpErrorCache->read(_httpSendBuf->buf + _httpSendBuf->len, _httpSendBuf->size - _httpSendBuf->len);

			/* ���͵� HTTP �ͻ��� */
			if( _httpSendBuf->len > 0)
			{
				if( IOCP_PENDING != _network->send(_clientSock, _httpSendBuf->buf, _httpSendBuf->len, _server->sendTimeout(), IOCPCallback, this))
				{
					close(CT_CLIENTCLOSED);
				}
			}
			else
			{
				close(CT_SENDCOMPLETE);
			}
		}
		return;
	}

	/*
	* ������һ�� IO �����Ľ��
	*/
	if(bytesTransfered == 0)
	{
		
	}
	else
	{
		/* ״̬�ص� */
		_server->onResponderDataSent(this, bytesTransfered);

		_bytesSent += bytesTransfered;

		/* ����һ��û�з���������Ƶ���������ͷ */
		_httpSendBuf->len -= bytesTransfered;
		if(_httpSendBuf->len > 0)
		{
			memmove(_httpSendBuf->buf, _httpSendBuf->buf + bytesTransfered, _httpSendBuf->len);
		}
	}

	/*
	* ����������һ�β���֮ǰ��һ���˳���
	*/
	bool isClose = false;
	_lock.lock();

	/* ��¼��һ�β����Ľ�� */
	if(0 == bytesTransfered)
	{
		_ds3Status = DSS_ABORTED;
		_exitCode = (flags & IOCP_WRITETIMEO) ? CT_SEND_TIMEO : CT_CLIENTCLOSED;
	}

	/*
	* ��ʼ��һ�� IO ����
	*/
	/* �Ƿ�������� */
	if(_ds3Status != DSS_ABORTED)
	{
		if(_ds2Status != DSS_ABORTED)
		{
			if(_httpSendBuf->len > 0 || hasData())
			{
				int sendRes = sendToHTTPClient();
				if(CT_SUCESS != sendRes)
				{
					_ds3Status = DSS_ABORTED;
					_exitCode = sendRes;
				}
			}
			else if(_ds2Status == DSS_COMPLETE)
			{
				_ds3Status = DSS_COMPLETE;
				_exitCode = CT_SENDCOMPLETE;
			}
			else
			{
				_ds3Status = DSS_PAUSED;
			}
		}
		else
		{
			/* ������2�������������,������3����������һ��IO����������Ϊ��ͣ */
			_ds3Status = DSS_PAUSED;
		}
	}

	/* �Ƿ��˳� */
	isClose = isExitPoint();
	_lock.unlock();

	if(isClose)
	{
		close(_exitCode);
	}
}
bool FCGIResponder::isExitPoint()
{
	/*
	* ʲô������Ӧ���˳�
	*/

	/* �˳�����1: ������2�����˴���,��ôֻҪ������3û����ִ�� IO ������Ӧ���˳� */
	if(_ds2Status == DSS_ABORTED && _ds3Status != DSS_RUNING) return true;
	
	/* �˳�����2: ������2���,Ӧ�õȴ�������3����������������ɲ��˳� */
	if(_ds2Status == DSS_COMPLETE && (_ds3Status == DSS_ABORTED || _ds3Status == DSS_COMPLETE)) return true;
	
	return false;
}

void FCGIResponder::onFCGIRecv(size_t bytesTransfered, int flags)
{
	/* ������յ������� */
	bool requestEnd = false;
	if(bytesTransfered == 0)
	{
	}
	else
	{
		_bytesFCGIRecv += bytesTransfered;
		_fcgiRecvBuf->len = bytesTransfered;
		requestEnd = parseStdout(_fcgiRecvBuf->buf, _fcgiRecvBuf->len);
	}

	/* 
	* ������һ�β���֮ǰ,����˳���
	*/
	bool isClose = false;
	_lock.lock();

	/* ��¼��һ�β����Ľ�� */
	if(0 == bytesTransfered)
	{
		_ds2Status = DSS_ABORTED;
		_exitCode = flags & IOCP_READTIMEO ? CT_FCGI_RECV_TIMEO : CT_FCGI_RECV_FAILED;
	}
	if(requestEnd)
	{
		/* �����ͷ� FCGI ���� */
		_ds2Status = DSS_COMPLETE;
		_fcgiFactory->releaseConnection(_fcgiConnection, true);
		_fcgiConnection = NULL;

		/* ���յ��������ݺ�,�����ݵĳ���д����Ӧͷ */
		if(_cacheAll)
		{
			std::string val("");
			if(_header.getField("Content-Length",val))
			{
				// FCGI �������Ѿ�ָ���� Content-Length 
				assert(atoi(val.c_str()) == _cache->size());
			}
			else
			{
				char tmpSizeBuf[20];
				sprintf(tmpSizeBuf, "%d", _cache->size());
				_header.add("Content-Length", tmpSizeBuf);
				_header.format();
			}
		}
	}

	/*
	* ��ʼִ��һ��һ�� IO ����
	*/
	/* �Ƿ������ FCGI ��������������(����: ������2û�����������Ҳû���յ� FCGI_END_REQUEST ����������3û�з����������) */
	if(_ds2Status != DSS_ABORTED && _ds2Status != DSS_COMPLETE && _ds3Status != DSS_ABORTED)
	{
		if(CT_SUCESS != recvFromFCGIServer())
		{
			_ds2Status = DSS_ABORTED;
			_exitCode = CT_FCGI_RECV_FAILED;
		}
	}

	/* �Ƿ���Ҫ����������3(����: ������2û�з����������,������������,������3��״̬Ϊ��ͣ */
	if(_ds2Status != DSS_ABORTED && _ds3Status == DSS_PAUSED)
	{
		if(((_cacheAll && requestEnd) || !_cacheAll) && hasData())
		{
			int sendRes = sendToHTTPClient();
			if(CT_SUCESS != sendRes)
			{
				_ds3Status = DSS_ABORTED;
				_exitCode = sendRes;
			}
			else
			{
				_ds3Status = DSS_RUNING;
			}
		}
	}

	/* �Ƿ��˳� */
	isClose = isExitPoint();
	_lock.unlock();

	if(isClose) close(_exitCode);
}

void FCGIResponder::onFCGISend(size_t bytesTransfered, int flags)
{
	if(bytesTransfered == 0)
	{
		close((flags & IOCP_WRITETIMEO) ? CT_FCGI_SEND_TIMEO : CT_FCGI_SEND_FAILED);
		return;
	}

	/*
	* 1. ͳ����Ϣ
	* 2. ����������ڻ�������û��һ�η���,������仺����
	* 3. ������Ͷ����л���record,������仺����
	*/
	_bytesFCGISent += bytesTransfered;
	_fcgiSendBuf->seekg(bytesTransfered, SEEK_CUR);

	if( !_fcgiSendBuf->eof() )
	{
		/* �������ڻ�������,��������,ֱ��ȫ���������. */
		int res = sendToFCGIServer();
		if(res != CT_SUCESS) close(res);
	}
	else
	{
		/* �������ݷ������,���û����� */
		_fcgiSendBuf->trunc(false);

		if(!_request->eof())
		{
			/* �������� POST ���� */
			int res = sendPostData();
			if( res != CT_SUCESS ) close(res);
		}
		else
		{
			/* ɾ��������1�Ļ�����,������Ҫ�� */
			delete _fcgiSendBuf;
			_fcgiSendBuf = NULL;
			freeBuffer(_postDataBuf);
			_postDataBuf = NULL;

			/* POST �����Ѿ��������,׼���������� FCGI ����������Ӧ(������2) */
			_fcgiRecvBuf = allocBuffer();
			_httpSendBuf = allocBuffer();
			_stdoutRecord = new FCGIRecord();

			/* ������2,3��ʼ����,��ʱ������Ҫ��ͬ������ */
			_ds2Status = DSS_RUNING;
			_ds3Status = DSS_PAUSED; /* һ��ʼ cache ��û������,����������3��״̬Ϊ��ͣ */
			
			/* ������2���յ�һ��STDOUT���� */
			int res = recvFromFCGIServer();
			if( res != CT_SUCESS )
			{
				close(res);
			}
		}
	}
}

int FCGIResponder::sendPostData()
{
	/* ȷ�ϻ�����Ϊ�� */
	assert(_fcgiSendBuf->tellg() == 0);
	if(_postDataBuf == NULL)
	{
		_postDataBuf = allocBuffer();
	}

	/* ��ȡһ�� POST DATA */
	_postDataBuf->len = _request->read(_postDataBuf->buf, _postDataBuf->size);
	if(_postDataBuf->len > 0)
	{
		/* �� POST DATA ���Ϊ���� FCGI Э���Record ��д�� FCGI Send Buffer */
		FCGIRecordWriter writer(*_fcgiSendBuf);
		writer.writeHeader(_fcgiConnection->requestId, FCGI_STDIN);
		writer.writeBodyData(_postDataBuf->buf, _postDataBuf->len);
		writer.writeEnd();
	}

	/* POST DATA �Ƿ��ȡ��� */
	if(_postDataBuf->len < _postDataBuf->size)
	{
		/* POST DATA ��ȡ���,׷��һ���� STDIN ��ʾ���� */
		FCGIRecordWriter writer(*_fcgiSendBuf);
		writer.writeHeader(_fcgiConnection->requestId, FCGI_STDIN);
		writer.writeEnd();
	}

	/* �Ѵ��������ݷ��͵� FCGI ������ */
	if(_fcgiSendBuf->fsize() > 0)
	{
		return sendToFCGIServer();
	}
	else
	{
		/* �����е���������? */
		assert(0);
		return CT_UNKNOWN;
	}
}

/*
* �ѽ����� FCGI ������������д�뻺��
* д��֮ǰ��Ҫ��һЩ����
* 1. ����Ӧͷ����д�� _fcgiResponseHeader ֱ����Ӧͷ����.
* 2. ���� FCGI ��Ӧͷ���ֵ���������һ����׼�� HTTP 1.1 ��Ӧͷ��д�뻺����.
* 3. �����ݲ���д�뻺����.
*/
size_t FCGIResponder::writeToCache(const void *buf, size_t len)
{
	const byte* data = reinterpret_cast<const byte*>(buf);
	size_t bytesWritten = 0;
	size_t bytesParsed = 0;

	if(len == 0)
	{
		/* д�� chunked ����Ľ�β�� */
		if( _chunkCoding && !_chunkEndSent)
		{
			_chunkEndSent = true;

			_lock.lock();
			bytesWritten += _cache->puts("0\r\n\r\n");
			_lock.unlock();
		}
		else
		{
			assert(0);
		}
	}
	else
	{
		/* ������ FCGI ����������ɵ� HTTP ��Ӧͷ�Ա����� */
		if(!_isFCGIHeaderReceived)
		{
			/* ���� HTTP ��Ӧͷֱ�����յ�������������Ϊ��־�Ľ�β */
			while(bytesParsed < len)
			{
				_fcgiResponseHeader.write(data + bytesParsed, 1);
				++bytesParsed;
				if(is_end(reinterpret_cast<const byte*>(_fcgiResponseHeader.buffer()), _fcgiResponseHeader.fsize()))
				{
					_isFCGIHeaderReceived = true;
					break;
				}
			}

			/* ������ FCGI ������̲����� HTTP ��Ӧͷ */
			if(_isFCGIHeaderReceived)
			{
				/* ����һ�� HTTP ��Ӧͷ */
				_svrCode = SC_OK;
				_header.setResponseCode(_svrCode);
				_header.addDefaultFields();
				_header.add(reinterpret_cast<const char*>(_fcgiResponseHeader.buffer()));
				
				/* ������ FCGI ���̲�������Ӧͷ�ļ���������: Status, Content-Length, Transfer-Encoding */
				std::string tmp;
				if(_header.getField("Status", tmp))
				{
					// FCGI Status ��ָ���µ���Ӧ��
					_svrCode = atoi(tmp.c_str());
					if( _svrCode == 0) _svrCode = SC_OK;
					_header.setResponseCode(_svrCode);
					_header.remove("Status");
				}

				int contentLen = 0;
				if(_header.getField("Content-Length", tmp))
				{
					// FCGI ����ָ�������ݵĳ���
					contentLen = atoi(tmp.c_str());
				}
				
				if(!_header.getField("Transfer-Encoding", tmp) && contentLen == 0 && !_cacheAll)
				{
					// FCGI ������û��ָ������,����û��ָ�� Transfer-Encoding ��ʹ�� chunked ����.
					_chunkCoding = true;
					_header.add("Transfer-Encoding", "chunked");
				}

				/* �Ƿ񱣳����� */
				if(_request->keepAlive())
				{
					_header.add("Connection", "keep-alive");
				}
				else
				{
					_header.add("Connection", "close");
				}

				/* ��ʽ����Ӧͷ׼����� */
				if(_cacheAll)
				{
				}
				else
				{
					_header.format();
				}

				///* HTTP ��Ӧͷд�뻺�� */
				//byte tmpBuf[1024];
				//size_t tmpLen = 0;
				//while((tmpLen = _header.read(tmpBuf, 1024)) != 0)
				//{
				//	bytesWritten += _cache->write(tmpBuf, tmpLen);
				//}
			}
		}

		/* ����������д�뻺�� */
		if(bytesParsed < len)
		{
			_lock.lock();

			/* д�ֿ�chunked ����ͷ: 16�����ַ�����ʾ�����ݳ��� + CRLF */
			if( _chunkCoding )
			{
				char chunkSize[200] = {0};
				sprintf(chunkSize, "%x\r\n", len - bytesParsed);
				bytesWritten += _cache->puts(chunkSize);
			}

			/* д������ */
			bytesWritten += _cache->write(data + bytesParsed, len - bytesParsed);

			/* ���β CRLF */
			if( _chunkCoding )
			{
				bytesWritten += _cache->puts("\r\n");
			}

			_lock.unlock();
		}
	}
	return bytesWritten;
}

/*
* �������� FCGI ������̵�������,���� FCGI Э��� record ����.
*/
bool FCGIResponder::parseStdout(const byte *buf, size_t len)
{
	bool requestEnd = false;
	size_t parsedLen = 0;
	while( parsedLen < len )
	{
		parsedLen += _stdoutRecord->write(buf + parsedLen, len - parsedLen);
		if( _stdoutRecord->check() )
		{
			FCGI_Header header;
			_stdoutRecord->getHeader(header);

			/* ���� FCGI Record Type ���ദ�� */
			if( FCGI_STDOUT == header.type )
			{
				if(_stdoutRecord->getContentLength(header) > 0)
				{
					writeToCache(_stdoutRecord->getBodyData(), _stdoutRecord->getContentLength(header));
				}
				else
				{
					/* ����FCGIЭ��,Ӧ����һ������Ϊ0�� FCGI_STDOUT ��ʾ���ݽ���,����û�յ���...��֪��Ϊʲô */
					assert(0);
				}
			}
			else if( FCGI_END_REQUEST == header.type)
			{
				unsigned int appStatus = 0;
				byte protocolStatus = 0;
				if(_stdoutRecord->getEndRequestBody(appStatus, protocolStatus))
				{
					assert(protocolStatus == FCGI_REQUEST_COMPLETE);
					requestEnd = true;
					if(_chunkCoding)
					{
						/* ����һ������ chunk */
						writeToCache(NULL, 0);
					}
				}
			}
			else if( FCGI_STDERR == header.type)
			{
				/* д��־ */
				std::string err;
				err.assign(reinterpret_cast<const char*>(_stdoutRecord->getBodyData()), _stdoutRecord->getBodyLength());
				//LOGGER_CWARNING(theLogger, _T("%s\r\n"), AtoT(err).c_str());
			}
			else
			{
				assert(0);
				/* ����֮ */
			}

			/*
			* ������һ��record
			*/
			_stdoutRecord->reset();
		}
	}

	return requestEnd;
}

int FCGIResponder::sendHttpError(int errCode, const char *msg)
{	
	assert(_httpErrorCache == NULL);

	char tmp[1024] = {0};
	if(NULL == _httpErrorCache) _httpErrorCache = new memfile();
	if(NULL == _httpSendBuf) _httpSendBuf = allocBuffer();

	/* ������Ӧͷ */
	_svrCode = errCode;

	_header.setResponseCode(_svrCode);
	_header.addDefaultFields();
	_header.add("Content-Type", "text/plain");
	sprintf(tmp, "%d", strlen(msg));
	_header.add("Content-Length", tmp);
	if(_request->keepAlive())
	{
		_header.add("Connection", "keep-alive");
	}
	else
	{
		_header.add("Connection", "close");
	}
	_header.format();

	/* ����Ӧͷ�ʹ�����Ϣд�뻺�� */
	size_t rd = 0;
	while( (rd = _header.read(reinterpret_cast<byte*>(tmp), 1024)) > 0)
	{
		_httpErrorCache->write(tmp, rd);
	}
	_httpErrorCache->puts(msg);

	/* ���͵�һ�����ݵ� HTTP �ͻ��� */
	_httpSendBuf->len = _httpErrorCache->read(_httpSendBuf->buf, _httpSendBuf->size);
	if( IOCP_PENDING != _network->send(_clientSock, _httpSendBuf->buf, _httpSendBuf->len, _server->sendTimeout(), IOCPCallback, this))
	{
		return CT_CLIENTCLOSED;
	}
	else
	{
		return CT_SUCESS;
	}
}

void FCGIResponder::close(int exitCode)
{
	if(exitCode != CT_CLIENTCLOSED && _bytesSent == 0 && _httpErrorCache == NULL)
	{
		/* �������ʱ��û�з����κ����ݵ� HTTP �ͻ���,����һ�� 500 error ���ͻ��� */
		int res = sendHttpError(SC_SERVERERROR, g_HTTP_Server_Error);
		if(CT_SUCESS != res) close(res);
	}
	else
	{
		_server->onResponder(this, exitCode);
	}
}

std::string FCGIResponder::getHeader()
{
	return _header.getHeader();
}

bool FCGIResponder::hasData()
{
	return !_header.eof() || !_cache->empty();
}