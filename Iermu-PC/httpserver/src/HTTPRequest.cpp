/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#include "StdAfx.h"
#include "HTTPRequest.h"

HTTPRequest::HTTPRequest(IHTTPServer *server, IOCPNetwork *network) :
	_hrt(true), _header(1024, MAX_REQUESTHEADERSIZE), _postFile(NULL), _isHeaderRecved(false), _bytesRecv(0),
	_server(server), _network(network), _sockBuf(NULL), _sockBufLen(0), _connId(NULL), _postData(NULL),
	_postFileName(""), _contentLength(0), _startTime(0), _headDeviation(0), _boundary(""), _ignoreLength(0)
{
}

HTTPRequest::~HTTPRequest()
{
}

size_t HTTPRequest::push(const byte* data, size_t len)
{
	//LOGGER_CINFO(theLogger, _T("[contentLength:%d   headsize:%d] - \r\n"), contentLength(), _header.fsize());
	if(_isHeaderRecved)
	{
		if( _postData )
		{
			size_t maxLen = contentLength() - _postData->tellp();
			if( len > maxLen ) len = maxLen;
			return _postData->write(data, len);
		}
		else if( _postFile )
		{
			//size_t maxLen = contentLength() - ftell(_postFile);
			size_t maxLen = contentLength() - static_cast<size_t>(_postFile->tell());
			if( len > maxLen ) len = maxLen;
			//return fwrite(data, 1, len, _postFile);
			return _postFile->write(data, len);
		}
		else
		{
			/*
			* ���� Content-Length ��ֵȷ�������ڴ��л��滹��ʹ���ļ�ϵͳ����.
			*/
			if( contentLength() > 0)
			{
				//_postFileName = _server->tmpFileName();
				_postFile = new WINFile;
				if(!_postFile->open(AtoT(_postFileName).c_str(), WINFile::rw, true))
				{
					assert(0);
					//LOGGER_CFATAL(theLogger, _T("�޷�����ʱ�ļ�[%s],������[%d].\r\n"), AtoT(_postFileName).c_str(), errno);
					return 0;
				}
			}
			else
			{
				_postData = new memfile(1024, POST_DATA_CACHE_SIZE);
				assert(_postData);
			}
			return push(data, len);
		}
	}
	else
	{
		/*
		* һ���ַ�һ���ַ�д��,ֱ��������������.
		*/

		size_t i = 0;
		for(; i < len; ++i)
		{
			if( 1 != _header.write(&data[i], 1))
			{
				/* ���� HTTP Request ͷ�ĳ��������� */
				/* ���Ǵӿͻ��˶�ȡ�����ݶ��ǲ���ȫ������,����������һ�����ֵ */
				assert(0);
				return 0;
			}

			if(is_end(reinterpret_cast<const byte*>(_header.buffer()), _header.fsize()))
			{
				/*
				* �Ѿ����յ�һ������������ͷ
				*/
				_contentLength = atoi(field("Content-Length").c_str());
				std::string contentType = get_field(reinterpret_cast<const char*>(_header.buffer())+_headDeviation, "Content-type");
				
				if (!contentType.empty())
				{
					int bdpos = contentType.find("boundary=");
					if (bdpos == std::string::npos && _headDeviation == 0)
					{
						_isHeaderRecved = true;
					}
					else
					{
						//��¼boundary������--
						if (!_boundary.length())
						{
							std::string bdTemp = contentType.substr(bdpos + 9);
							int bdend = bdTemp.find("\r\n");
							_boundary = bdTemp.substr(0, bdend);
							_boundary = _boundary + "--";
							_ignoreLength = _header.fsize();
						}

						std::string contentDisposition = get_field(reinterpret_cast<const char*>(_header.buffer()) + _headDeviation, "Content-Disposition");
						if (!contentDisposition.empty())
						{
							int pos = contentDisposition.find("filename=\"");
							std::string deviceID;
							std::string jpgname;
							if (pos != std::string::npos)
							{
								//��ȡ�豸��
								int i = 0;
								std::string url = uri(true);
								int idx = url.find("deviceID=");
								if (idx != std::string::npos)
								{
									std::string ptmp = url.substr(idx+9);
									int idx = ptmp.find("&");
									if (idx != std::string::npos)
									{
										deviceID = ptmp.substr(0, idx);
									}
								}
								
								//��ȡ�ļ���
								std::string strs = contentDisposition.substr(pos+10);
								int eof = strs.find("\"");
								if (eof != std::string::npos)
								{
									jpgname = strs.substr(0, eof);
								}
								if (jpgname.length() > 0)
									_postFileName = _server->jpgFileName(deviceID, jpgname);
								else
									_postFileName = _server->tmpFileName();

								_ignoreLength = _header.fsize() - _ignoreLength;
								_isHeaderRecved = true;
							}
						}
					}
				}
				else
				{
					_isHeaderRecved = true;
				}

				++i;
				_headDeviation += i;

				if (_isHeaderRecved)
				{ 
					_headDeviation = 0;
				}

				if(_contentLength >= MAX_POST_DATA)
				{
					/* ��� content-length �����Ƿ񳬳����� */
					assert(0);
					return 0;
				}

				if(i < len)
				{
					/* �������� */
					i += push(data + i, len - i);
				}

				break;
			}
		}

		return i;
	}
}


bool HTTPRequest::isValid()
{
	if (_isHeaderRecved)
	{
		if( _postData ) return _postData->tellp() == contentLength();
		else if (_postFile) return contentLength() == (static_cast<size_t>(_postFile->tell()) + _ignoreLength);
		else return contentLength() == 0;
	}
	return false;
}

size_t HTTPRequest::headerSize()
{
	return _header.fsize();
}

size_t HTTPRequest::size()
{
	if( _postFile )
	{
		return headerSize() + static_cast<size_t>(_postFile->tell());
	}
	else if( _postData )
	{
		return headerSize() + _postData->tellp();
	}
	else
	{
		return headerSize();
	}
}

bool HTTPRequest::keepAlive()
{
	return field("Connection") == std::string("keep-alive");
}

size_t HTTPRequest::contentLength()
{
	return _contentLength;
}

HTTP_METHOD HTTPRequest::method()
{
	// ȡ�� HTTP ����
	char szMethod[MAX_METHODSIZE] = {0};
	int nMethodIndex = 0;
	for(size_t i = 0; i < MAX_METHODSIZE && i < _header.fsize(); ++i)
	{
		if(reinterpret_cast<const char*>(_header.buffer())[i] != ' ')
		{
			szMethod[nMethodIndex++] = reinterpret_cast<const char*>(_header.buffer())[i];
		}
		else
		{
			break;
		}
	}

	// ����
	if( strcmp(szMethod, "GET") == 0 ) return METHOD_GET;
	if( strcmp(szMethod, "PUT") == 0 ) return METHOD_PUT;
	if( strcmp(szMethod, "POST") == 0 ) return METHOD_POST;
	if( strcmp(szMethod, "HEAD") == 0 ) return METHOD_HEAD;
	if( strcmp(szMethod, "DELETE") == 0 ) return METHOD_DELETE; // ɾ��
	if( strcmp(szMethod, "TRACE") == 0 ) return METHOD_TRACE;
	if( strcmp(szMethod, "CONNECT") == 0 ) return METHOD_CONNECT;

	return METHOD_UNKNOWN;
}

// ���ؿͻ����������, ������ؿ��ַ���,˵���ͻ��������ʽ����.
std::string HTTPRequest::uri(bool decode)
{
	std::string strObject("");
	const char* lpszRequest = reinterpret_cast<const char*>(_header.buffer());
	const char *pStart = NULL, *pEnd = NULL;

	// ��һ�еĵ�һ���ո����һ���ַ���ʼ��������ļ�����ʼ.
	for(size_t i = 0; i < _header.fsize(); ++i)
	{
		if(lpszRequest[i] == ' ')
		{
			pStart = lpszRequest + i + 1; 
			break;
		}
		if(lpszRequest[i] == '\n') break;
	}
	if(pStart == NULL)
	{
		// �Ҳ�����ʼλ��
		assert(0);
		return strObject;
	}

	// �ӵ�һ�е�ĩβ������ҵ�һ���ո�,ʵ��: GET / HTTP/1.1
	pEnd = strstr(lpszRequest, "\r\n"); 
	if(pEnd == NULL || pEnd < pStart) 
	{
		/* �Ҳ�����βλ�� */
		assert(0);
		return strObject;
	}

	// �ѽ�β�Ŀո��Ƴ�
	while(pEnd >= pStart)
	{
		if(pEnd[0] == ' ')
		{
			pEnd--;
			break;
		}
		pEnd--;
	}

	if(pEnd == NULL || pEnd < pStart)
	{
		assert(0);
	}
	else
	{
		strObject.assign(pStart, pEnd - pStart + 1);
	}

	if(decode) return decode_url(strObject);
	else return strObject;
}

std::string HTTPRequest::field(const char* pszKey)
{
	return get_field(reinterpret_cast<const char*>(_header.buffer()), pszKey);
}

bool HTTPRequest::range(__int64 &lFrom, __int64 &lTo)
{
	__int64 nFrom = 0;
	__int64 nTo = -1; // -1 ��ʾ�����һ���ֽ�.

	const char* lpszRequest = reinterpret_cast<const char*>(_header.buffer());
	const char* pRange = strstr(lpszRequest, "\r\nRange: bytes=");
	if(pRange)
	{
		/*
		The first 500 bytes (byte offsets 0-499, inclusive):
		bytes=0-499
		The second 500 bytes (byte offsets 500-999, inclusive):
		bytes=500-999
		The final 500 bytes (byte offsets 9500-9999, inclusive):
		bytes=-500
		bytes=9500-
		The first and last bytes only (bytes 0 and 9999):
		bytes=0-0,-1
		Several legal but not canonical specifications of the second 500 bytes (byte offsets 500-999, inclusive):
		bytes=500-600,601-999
		bytes=500-700,601-999
		*/

		pRange += strlen("\r\nRange: bytes=");
		const char *pMinus = strchr(pRange, '-');
		if(pMinus)
		{
			char szFrom[200], szTo[200];
			memset(szFrom, 0, 200);
			memset(szTo, 0, 200);
			memcpy(szFrom, pRange, pMinus - pRange);
			nFrom = _atoi64(szFrom);

			pMinus++;
			pRange = strstr(pMinus, "\r\n");
			if(pMinus + 1 == pRange)
			{
				nTo = -1;
			}
			else
			{
				memcpy(szTo, pMinus, pRange - pMinus);
				nTo = _atoi64(szTo);
				if(nTo <= 0) nTo = -1;
			}

			lFrom = nFrom;
			lTo = nTo;

			return true;
		}
		else
		{
		}
	}
	else
	{
	}
	return false;
}

size_t HTTPRequest::read(byte* buf, size_t len)
{
	if(_postData)
	{
		return _postData->read(buf, len);
	}
	else if(_postFile)
	{
		return _postFile->read(buf, len);
		//return fread(buf, 1, len, _postFile);
	}
	else
	{
		return 0;
	}
}

bool HTTPRequest::eof()
{
	if(_postData)
	{
		return _postData->eof();
	}
	else if(_postFile)
	{
		//return feof(_postFile) != 0;
		return _postFile->eof();
	}
	else
	{
		return true;
	}
}

bool HTTPRequest::reset()
{
	_connId = INVALID_CONNID;
	_clientSock = NULL;
	if(_sockBuf)
	{
		delete []_sockBuf;
		_sockBuf = NULL;
	}
	_sockBufLen = 0;
	if(_postFile)
	{
		_postFile->close();
		delete _postFile;
		_postFile = NULL;

		// ��ʱ�ļ�,close() ���Զ�ɾ��,����Ҫ���� remove()
		// WINFile::remove(AtoT(_postFileName).c_str());
	}
	if(_postData)
	{
		delete _postData;
		_postData = NULL;
	}
	
	_isHeaderRecved = false;
	_headDeviation = 0;
	_header.trunc();
	_bytesRecv = 0;
	_startTime = 0;
	return true;
}

int HTTPRequest::run(conn_id_t connId, iocp_key_t clientSock, size_t timeout)
{
	/*
	* timeout: ��һ�� recv �����ĳ�ʱʱ��,����� keep-alive ���ֵ�����,���ֵ���ܻ���½����ӵ�ֵ��ͬ
	*/
	assert(_sockBuf == NULL);
	if(_sockBuf)
	{
		return CT_INTERNAL_ERROR;
	}

	_clientSock = clientSock;
	_sockBuf = new byte[MAX_SOCKBUFF_SIZE];
	_sockBufLen = MAX_SOCKBUFF_SIZE;
	_connId = connId;

	/*
	* ��ʼ��������ͷ
	*/
	if(IOCP_PENDING == _network->recv(_clientSock, _sockBuf, _sockBufLen, timeout, IOCPCallback, this))
	{
		return CT_SUCESS;
	}
	else
	{
		reset();
		return CT_CLIENTCLOSED;
	}
}

bool HTTPRequest::stop(int ec)
{
	return true;
}

void HTTPRequest::deleteSocketBuf()
{
	assert(_sockBuf);
	delete []_sockBuf;
	_sockBuf = NULL;
	_sockBufLen = 0;
}

void HTTPRequest::close(int exitCode)
{
	/* ɾ��������,������Ҫ�� */
	deleteSocketBuf();
	_server->onRequest(this, exitCode);
}

bool HTTPRequest::isBoundary()
{
	return _boundary.length() > 0;
}

void HTTPRequest::onRecv(int flags, size_t bytesTransfered)
{
	/* ����ʧ�� */
	if(bytesTransfered == 0)
	{
		if(flags & IOCP_READTIMEO)
		{
			close(CT_RECV_TIMEO);
		}
		else
		{
			close(CT_CLIENTCLOSED);
		}
		return;
	}

	/* ���ճɹ� */

	/* �յ���һ������ͷ������ʱ,��Ϊ����ʼ��ʱ���¼����*/
	if(0 == _startTime) _startTime = _hrt.now();

	_bytesRecv += bytesTransfered;
	_server->onRequestDataReceived(this, bytesTransfered);

	size_t bytesPushed = push(_sockBuf, bytesTransfered);
	if(isValid())
	{
		assert(bytesTransfered == bytesPushed);

		/* ���ļ�ָ���Ƶ���ʼλ��,׼���� */
		if( _postFile )
		{
			//fseek(_postFile, 0, SEEK_SET);
			_postFile->seek(0, WINFile::s_set);
		}

		/* HTTP Request ������� */
		_ignoreLength = 0;
		_boundary = "";
		close(CT_SUCESS);
	}
	else
	{
		if( bytesPushed != bytesTransfered )
		{
			/* ������������,��Ҫ���ͻ��˷��� 400 ����,������ CT_SUCESS �˳��� */
			close(CT_SUCESS);
		}
		else
		{
			int netIoRet = _network->recv(_clientSock, _sockBuf, MAX_SOCKBUFF_SIZE, _server->recvTimeout(), IOCPCallback, this);
			if(IOCP_PENDING != netIoRet)
			{
				close(netIoRet == IOCP_SESSIONTIMEO ? CT_SESSION_TIMEO : CT_CLIENTCLOSED);
			}
			else
			{
				/* �������� */
			}
		}
	}
}

void HTTPRequest::IOCPCallback(iocp_key_t s, int flags, bool result, int transfered, byte* buf, size_t len, void* param)
{
	HTTPRequest* instPtr = reinterpret_cast<HTTPRequest*>(param);
	assert(flags & IOCP_RECV);
	instPtr->onRecv(flags, transfered);
}

std::string HTTPRequest::getHeader()
{
	if(!_isHeaderRecved) return std::string("");

	return std::string(reinterpret_cast<char*>(_header.buffer()), _header.fsize());
}