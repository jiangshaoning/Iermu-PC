/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#include "StdAfx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "HTTPResponder.h"
#include "HTTPContent.h"

HTTPResponder::HTTPResponder(IHTTPServer *server, IOCPNetwork *network)
	: _content(NULL), _sockBuf(NULL), _sockBufLen(0), _request(NULL), _bytesRecv(0), _bytesSent(0),
	_server(server), _network(network), _clientSock(IOCP_NULLKEY), _svrCode(SC_UNKNOWN), _connId(NULL)
{
	
}

HTTPResponder::~HTTPResponder()
{
}

bool HTTPResponder::makeResponseHeader(int svrCode)
{
	/* ����״ֵ̬ */
	_svrCode = svrCode;

	/* д��Ӧͷ */
	_header.setResponseCode(svrCode);
	_header.addDefaultFields();
	if(svrCode == SC_BADMETHOD)
	{
		_header.add("Allow", "GET, HEAD, POST, PUT");
	}
	if(_content && _content->isOpen())
	{
		// Last-Modified
		_header.add("Last-Modified", _content->lastModified());
		
		// ETag
		_header.add("ETag", _content->etag());
		
		// Content-Type
		_header.add("Content-Type", _content->contentType());
		
		// Content-Length
		char szLen[200] = {0};
		__int64 lLen = _content->contentLength();
		_header.add("Content-Length", _i64toa(lLen, szLen, 10));
		
		// Content-Range: bytes %d-%d/%d\r\n"
		if(SC_PARTIAL == svrCode)
		{
			_header.add("Content-Range", _content->contentRange());
		}

		// "Accept-Ranges: bytes" ֧�ֶϵ�����(ֻ�о�̬�ļ�֧�ֶϵ�����).
		if(_content->isFile())
		{
			_header.add("Accept-Ranges", "bytes");
		}
	}
	else
	{
		// Content-Length 
		_header.add("Content-Length", "0");
	}
	if(_request->keepAlive())
	{
		_header.add("Connection", "keep-alive");
	}
	else
	{
		_header.add("Connection", "close");
	}

	// ��ʽ����Ӧͷ׼�����
	_header.format();

	return true;
}

int HTTPResponder::run(conn_id_t connId, iocp_key_t clientSock, IRequest *request)
{
	if(_sockBuf)
	{
		return CT_INTERNAL_ERROR;
	}
	
	_connId = connId;
	_clientSock = clientSock;
	_request = request;

	_sockBuf = new byte[MAX_SOCKBUFF_SIZE];
	_sockBufLen = 0;
	_content = new HTTPContent;

	int svrCode = SC_OK;
	do
	{
		if (METHOD_POST == _request->method())
		{
			svrCode = SC_OK;
			break;
		}
		/* �Ƿ�����ͷ��������ͷ̫�� */
		if(!_request->isValid())
		{
			svrCode = SC_BADREQUEST;
			_content->open(g_HTTP_Bad_Request, strlen(g_HTTP_Bad_Request), OPEN_HTML);
			break;
		}

		/* ��̬�ļ�ֻ֧�� GET �� HEAD ���� */
		if( METHOD_GET != _request->method() && METHOD_HEAD != _request->method())
		{
			svrCode = SC_BADMETHOD;
			_content->open(g_HTTP_Bad_Method, strlen(g_HTTP_Bad_Method), OPEN_HTML);
			break;
		}

		/* ���ļ� */
		std::string uri = _request->uri(true);
		std::string serverFilePath("");

		// ӳ��Ϊ�������ļ���.
		if(!_server->mapServerFilePath(uri, serverFilePath))
		{
			// �޷�ӳ��������ļ���,��ʾ��ֹ
			svrCode = SC_FORBIDDEN;
			_content->open(g_HTTP_Forbidden, strlen(g_HTTP_Forbidden), OPEN_TEXT);
		}
		else if(serverFilePath.back() == '\\')
		{
			if(_content->open(uri, serverFilePath))
			{
				svrCode = SC_OK;
			}
			else
			{
				svrCode = SC_SERVERERROR;
				_content->open(g_HTTP_Server_Error, strlen(g_HTTP_Server_Error), OPEN_HTML);
			}
		}
		else
		{
			// �ͻ����Ƿ������˶ϵ�����������
			// �����ļ����ݶ��󲢹�����Response����
			__int64 lFrom = 0;
			__int64 lTo = -1;
			if(_request->range(lFrom, lTo))
			{
				svrCode = SC_PARTIAL;
			}

			if(!_content->open(serverFilePath, lFrom, lTo))
			{
				svrCode = SC_NOTFOUND;
				_content->open(g_HTTP_Content_NotFound, strlen(g_HTTP_Content_NotFound), OPEN_TEXT);
			}
		}
	}
	while(false);
	
	/*
	* ������Ӧͷ�����͵�һ�����ݰ�
	*/
	//assert(_content->isOpen());
	makeResponseHeader(svrCode);
	if(_request->method() == METHOD_HEAD)
	{
		_content->close();
		delete _content;
		_content = NULL;
	}

	/* 
	* ���͵�һ�����ݰ� 
	*/
	
	/* ��ȡ��Ӧͷ */
	if(_sockBufLen < MAX_SOCKBUFF_SIZE) _sockBufLen += _header.read(_sockBuf + _sockBufLen, MAX_SOCKBUFF_SIZE - _sockBufLen);

	if (METHOD_POST != _request->method())
	{
		/* ��ȡ���� */
		if (_content && _content->isOpen() && _sockBufLen < MAX_SOCKBUFF_SIZE)
		{
			_sockBufLen += _content->read(_sockBuf + _sockBufLen, MAX_SOCKBUFF_SIZE - _sockBufLen);
		}
	}

	int netIoRet = _network->send(_clientSock, _sockBuf, _sockBufLen, _server->sendTimeout(), IOCPCallback, this);
	if(IOCP_PENDING != netIoRet)
	{
		reset();
		return IOCP_SESSIONTIMEO == netIoRet ? CT_SESSION_TIMEO : CT_CLIENTCLOSED;
	}
	else
	{
		return CT_SUCESS;
	}
}

bool HTTPResponder::stop(int ec)
{
	return false;
}

bool HTTPResponder::reset()
{
	if(_content)
	{
		if(_content->isOpen()) _content->close();
		delete _content;
		_content = NULL;
	}
	if(_sockBuf)
	{
		delete []_sockBuf;
		_sockBuf = NULL;
	}
	_connId = INVALID_CONNID;
	_request = NULL;
	_header.reset();
	_sockBufLen = 0;
	_clientSock = IOCP_NULLKEY;
	_bytesSent = 0;
	_bytesRecv = 0;
	_svrCode = SC_UNKNOWN;
	return true;
}

void HTTPResponder::onSend(size_t bytesTransfered, int flags)
{
	if(0 == bytesTransfered)
	{
		close((flags & IOCP_WRITETIMEO) ? CT_SEND_TIMEO : CT_CLIENTCLOSED);
	}
	else
	{
		_bytesSent += bytesTransfered;

		/* ״̬�ص� */
		_server->onResponderDataSent(this, bytesTransfered);

		/* 
		* �������� 
		*/
		_sockBufLen -= bytesTransfered;

		/* ���������Ƿ�������,�����Ƶ���������ͷ */
		if(_sockBufLen > 0)
		{
			memmove(_sockBuf, _sockBuf + bytesTransfered, _sockBufLen);
		}

		/* ������ȡ��Ӧͷ */
		if(_sockBufLen < MAX_SOCKBUFF_SIZE) _sockBufLen += _header.read(_sockBuf + _sockBufLen, MAX_SOCKBUFF_SIZE - _sockBufLen);

		/* ��ȡ���� */
		if(_content && _content->isOpen() && _sockBufLen < MAX_SOCKBUFF_SIZE)
		{
			_sockBufLen += _content->read(_sockBuf + _sockBufLen, MAX_SOCKBUFF_SIZE - _sockBufLen);
		}

		if( 0 == _sockBufLen )
		{
			/* ���ݷ������ */
			close(CT_SENDCOMPLETE);
		}
		else
		{
			/* �������� */
			int netIoRet = _network->send(_clientSock, _sockBuf, _sockBufLen, _server->sendTimeout(), IOCPCallback, this);
			if(IOCP_PENDING == netIoRet)
			{
				/* ���ͳɹ� */
			}
			else
			{
				close( IOCP_SESSIONTIMEO == netIoRet ? CT_SESSION_TIMEO : CT_CLIENTCLOSED);
			}
		}
	}
}

void HTTPResponder::close(int ct)
{
	_server->onResponder(this, ct);
}

void HTTPResponder::IOCPCallback(iocp_key_t s, int flags, bool result, int transfered, byte* buf, size_t len, void* param)
{
	HTTPResponder *instPtr = reinterpret_cast<HTTPResponder*>(param);
	if(flags & IOCP_SEND)
	{
		instPtr->onSend(transfered, flags);
	}
	else
	{
		assert(0);
	}
}

std::string HTTPResponder::getHeader()
{
	return _header.getHeader();
}