/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#include "stdafx.h"
#include <assert.h>
#include <io.h>
#include <sstream>
#include "HttpServer.h"
#include "HTTPRequest.h"
#include "HTTPResponder.h"
#include "FCGIResponder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
HTTPServer::HTTPServer()
	: _isRuning(false),
	_sListen(IOCP_NULLKEY),
	_sockNewClient(INVALID_SOCKET),
	_statusHandler(NULL),
	_hrt(true), 
	_fcgiFactory(NULL),
	_tmpFileNameNo(0)
{
	/*
	* �����16���ֽ��� AcceptEx()��Ҫ��,��MSDN.
	*/
	_acceptContext.len = (sizeof(sockaddr_in) + 16) * 2;  
	_acceptContext.buf = new byte[_acceptContext.len];
	IOCPNetwork::initWinsockLib(2, 2);
}

HTTPServer::~HTTPServer()
{
	assert(_sListen == IOCP_NULLKEY);
	assert(_sockNewClient == INVALID_SOCKET);

	delete []_acceptContext.buf;
	IOCPNetwork::cleanWinsockLib();
}

HTTPServer::connection_context_t* HTTPServer::allocConnectionContext(const std::string &ipAddr, unsigned int port)
{
	connection_context_t* conn = new connection_context_t;
	assert(conn);
	if( NULL == conn ) return NULL;
	memset(conn, 0, sizeof(connection_context_t));

	/*
	* ���� �������.
	*/
	conn->request = new HTTPRequest(this, &_network);

	/*
	* ��¼״̬.
	*/
	conn->startTime = _hrt.now();

	assert(ipAddr.size() <= MAX_IP_LENGTH);
	strcpy(conn->ip, ipAddr.c_str());
	conn->port = port;

	return conn;
}

void HTTPServer::freeConnectionContext(connection_context_t* conn)
{
	if(NULL == conn) 
	{
		assert(0);
		return;
	}

	/*
	* ��IOCPģ�����Ƴ����ҹر��׽���.
	*/
	if(conn->clientSock != IOCP_NULLKEY)
	{
		assert(0);
		_network.remove(conn->clientSock);
	}
	if(conn->request)
	{
		conn->request->reset();
		delete conn->request;
	}
	if(conn->responder)
	{
		conn->responder->reset();
		delete conn->responder;
	}

	delete conn;
}

bool HTTPServer::mapServerFilePath(const std::string& orgUrl, std::string& serverPath)
{
	// ��url���� utf-8 ����
	std::string deUrl = decode_url(orgUrl);

	// ��ø�Ŀ¼
	serverPath = _docRoot;
	if(serverPath.back() == '\\') serverPath.erase(--serverPath.end());

	// �� URL �е�·������(�������ֺ���)�ϲ��������·��
	std::string::size_type pos = orgUrl.find('?');
	if( pos != std::string::npos )
	{
		serverPath += orgUrl.substr(0, pos);
	}
	else
	{
		serverPath += deUrl;
	}
	
	// URL����б���滻Ϊ��б��.
	for(std::string::iterator iter = serverPath.begin(); iter != serverPath.end(); ++iter)
	{
		if( *iter == '/' ) *iter = '\\'; 
	}

	// �����Ŀ¼�����Ҳ��������Ŀ¼,�������Ĭ���ļ���
	if(serverPath.back() == '\\' && !isDirectoryVisible())
	{
		// ��ֹ���Ŀ¼,�ȳ��Դ�Ĭ���ļ�
		bool hasDftFile = false;
		str_vec_t dftFileNames;
		split_strings(defaultFileNames(), dftFileNames, ",");
		for(str_vec_t::iterator iter = dftFileNames.begin(); iter != dftFileNames.end(); ++iter)
		{
			std::string dftFilePath(serverPath);
			dftFilePath += *iter;
			if(WINFile::exist(AtoT(dftFilePath).c_str()))
			{
				serverPath += *iter;
				hasDftFile = true;
				break;
			}
		}

		return hasDftFile;
	}
	else
	{
		return true;
	}
}

std::string HTTPServer::tmpFileName()
{
	char fileName[MAX_PATH + 1] = {0};
	if( 0 == GetTempFileNameA(_tmpRoot.c_str(), _tmpFileNamePre, 0, fileName))
	{
		//assert(0);

		/* �޷���ȡ��ʱ�ļ���,���������һ�� */
		int no = 0;
		_lock.lock();
		no = ++_tmpFileNameNo;
		_lock.unlock();

		std::stringstream fmt;

		if(_tmpRoot.back() == '\\')
		{
			fmt << _tmpRoot << no << ".tmp";
		}
		else
		{
			fmt << _tmpRoot << '\\' << no << ".tmp";
		}

		return fmt.str();
	}
	else
	{
		return fileName;
	}
}

std::string timeparse(std::string& piTime)
{
	std::string jpgdir;
	time_t currtime = { 0 };

	struct tm Tm = { 0 };
	struct tm *pTm = &Tm;

	currtime = atol(piTime.c_str());
	pTm = localtime(&currtime);


	if (pTm)
	{
		char name[12] = { 0 };
		sprintf(name, "%04d%02d%02d", pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday);
		jpgdir = name;

		//printf("-- %04d-%02d-%02d %02d:%02d:%02d \n",
		//	pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
		//	pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
	}
	return jpgdir;
}

std::string HTTPServer::jpgFileName(std::string& deviceIDdir, std::string& fileidir)
{
	std::stringstream fmt;
	std::string jpgdir = timeparse(fileidir.substr(0, 10));
	
	if (_tmpRoot.back() == '\\')
	{
		fmt << _tmpRoot << deviceIDdir << jpgdir;
	}
	else
	{
		fmt << _tmpRoot << '\\' << deviceIDdir << '\\' << jpgdir;
	}

	WINFile::createDirectory((fmt.str()).c_str());
	fmt << '\\' << fileidir;

	return fmt.str();
}

/*
* ��ʼ�������׽���
*/
int HTTPServer::initListenSocket(const std::string& strIP, int nPort, SOCKET& hListenSock)
{
	SOCKET hSock = INVALID_SOCKET;

	/*
	* �����׽���,����Ϊ������ģʽ,���Ұ������˿�.
	*/
	if( (hSock = socket(PF_INET, SOCK_STREAM, /*IPPROTO_TCP*/0 )) == INVALID_SOCKET )
	{
		//LOGGER_CERROR(theLogger, _T("�޷����������׽���.\r\n"));

		return SE_CREATESOCK_FAILED;
	}

	u_long nonblock = 1;
	ioctlsocket(hSock, FIONBIO, &nonblock);
	
	sockaddr_in addr;
	addr.sin_family	= AF_INET;
	addr.sin_port = htons(nPort);
	if( strIP == "" )
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		addr.sin_addr.s_addr = inet_addr(strIP.c_str());
	}
	if( 0 != bind(hSock, (sockaddr *)&addr, sizeof(sockaddr_in)) )
	{
		//LOGGER_CERROR(theLogger, _T("�����׽����޷��󶨶˿�:%d.\r\n"), nPort);
		closesocket(hSock);
		return SE_BIND_FAILED;
	}

	if( 0 != listen(hSock, 10))
	{
		//LOGGER_CERROR(theLogger, _T("�����׽����޷�����,������:%d.\r\n"), WSAGetLastError());
		closesocket(hSock);
		return SE_LISTEN_FAILED;
	}
	else
	{
		hListenSock = hSock;
		return SE_SUCCESS;
	}
}

int HTTPServer::run(IHTTPConfig *conf, IHTTPServerStatusHandler *statusHandler)
{
	int ret = SE_SUCCESS;

	do
	{
		/*
		* �������Ѿ�������
		*/
		if(runing())
		{
			//assert(0);
			ret = SE_RUNING;
			//LOGGER_CWARNING(theLogger, _T("�Ѿ�������.\r\n"));
			return ret;
		}

		/*
		* ��ʼ��ServerEnv
		*/
		_docRoot = conf->docRoot(); /*��Ŀ¼*/
		_tmpRoot = conf->tmpRoot();
		_isDirectoryVisible = conf->dirVisible(); /*�Ƿ��������Ŀ¼*/
		_dftFileName = conf->defaultFileNames(); /*Ĭ���ļ���*/
		_ip = conf->ip(); /*������IP��ַ*/
		_port = conf->port(); /*�����������˿�*/
		_maxConnections = conf->maxConnections(); /*���������*/
		_maxConnectionsPerIp = conf->maxConnectionsPerIp(); /*ÿ��IP�����������*/
		_maxConnectionSpeed = conf->maxConnectionSpeed(); /*ÿ�����ӵ��ٶ�����,��λ b/s.*/

		_sessionTimeout = conf->sessionTimeout(); /*�Ự��ʱ*/
		_recvTimeout = conf->recvTimeout(); /*recv, connect, accept �����ĳ�ʱ*/
		_sendTimeout = conf->sendTimeout(); /*send �����ĳ�ʱ*/
		_keepAliveTimeout = conf->keepAliveTimeout();

		_statusHandler = statusHandler;

		/*
		* �����������ʹ�� fopen() �򿪵��ļ���
		*/
		//if(_maxConnections > MAX_STDIO) _maxConnections = MAX_STDIO;
		//_setmaxstdio(_maxConnections);
		assert(_maxConnections <= MAX_WINIO);
	
		/*
		* ��ʼ������ģ��
		*/
		if(IOCP_SUCESS != _network.init(0))
		{
			assert(0);
			ret = SE_NETWORKFAILD;
			//LOGGER_CERROR(theLogger, _T("�޷���ʼ������.\r\n"));
			break;
		}

		/*
		* ��ʼ�������׽���,��ע�ᵽIOCP����ģ����׼������������.
		*/
		SOCKET sockListen = INVALID_SOCKET;
		int lsRet = initListenSocket(_ip, _port, sockListen);
		if( SE_SUCCESS != lsRet )
		{
			ret = lsRet;
			break;
		}
		_sListen = _network.add(sockListen, 0, 0, 0);

		/*
		* ��ʼ�� Fast CGI ģ��
		* Ŀǰֻ֧��һ�� FCGI ������,�� _fcgiFactory ��Ϊ���к�������չ.
		*/
		assert(_fcgiFactory == NULL);
		fcgi_server_t fcgiServerInf;
		if(conf->getFirstFcgiServer(&fcgiServerInf) && fcgiServerInf.status)
		{
			_fcgiFactory = new FCGIFactory(this, &_network);
			_fcgiFactory->init(fcgiServerInf.path, fcgiServerInf.port, fcgiServerInf.exts, fcgiServerInf.maxConnections, 
				fcgiServerInf.maxWaitListSize == 0 ? SIZE_T_MAX : fcgiServerInf.maxWaitListSize, fcgiServerInf.cacheAll);
		}
	
		/*
		* ��¼״̬
		*/
		_isRuning = true;
		_connections.clear();
		_connectionIps.clear();

		/*
		* ��ʱ�ļ������ɲ���
		*/
		_tmpFileNameNo = 0;
		srand(static_cast<unsigned int>(time(NULL)));
		sprintf(_tmpFileNamePre, "%03d", rand() % 1000);

		/* 
		* ִ�е�һ��accept() 
		*/
		int acceptRet = doAccept();
		if(acceptRet != SE_SUCCESS)
		{
			assert(0);
			ret = acceptRet;
			break;
		}

		/*
		* �ɹ��ĳ���
		*/
		std::string ipAddress = ip();
		if(ipAddress == "")
		{
			get_ip_address(ipAddress);
		}
		//LOGGER_CINFO(theLogger, _T("Q++ HTTP Server ��������,��Ŀ¼[%s],��ַ[%s:%d],���������[%d].\r\n"), 
		//	AtoT(docRoot()).c_str(), AtoT(ipAddress).c_str(), port(), maxConnections());

		return SE_SUCCESS;
	}while(0);

	/*
	* ʧ�ܵĳ���
	*/
	doStop();
	//LOGGER_CWARNING(theLogger, _T("Q++ HTTP Server ����ʧ��.\r\n"));
	return ret;
}

int HTTPServer::doAccept()
{
	assert(_sListen != IOCP_NULLKEY);

	/*
	* ����һ���µ��׽���.
	*/
	_sockNewClient = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if( INVALID_SOCKET == _sockNewClient )
	{
		assert(0);
		//LOGGER_CERROR(theLogger, _T("�޷������׽���,������:%d.\r\n"), WSAGetLastError());
		return SE_CREATESOCK_FAILED;
	}

	/*
	* ����AcceptEx()����
	*/
	if(IOCP_PENDING != _network.accept(_sListen, _sockNewClient, _acceptContext.buf, _acceptContext.len, 0, IOCPCallback, this))
	{
		closesocket(_sockNewClient);
		_sockNewClient = INVALID_SOCKET;

		assert(0);
		//LOGGER_CERROR(theLogger, _T("ִ��acceptʧ��,�޷�����������,������:%d.\r\n"), _network.getLastError());
		return SE_NETWORKFAILD;
	}
	else
	{
		return SE_SUCCESS;
	}
}

void HTTPServer::doStop()
{
	/*
	* ���ճ�ʼ���ķ�˳������
	*/

	/*
	* �ͷ�����FCGI��Դ,����Fast CGI ģ��
	*/
	if(_fcgiFactory)
	{
		_fcgiFactory->destroy();
	}

	/*
	* ֹͣ����ģ��,ʹIOCP�ص��������ٱ�����.
	* �ر������׽��ֺ�Ϊ���������Ӷ�׼�����׽���.
	*/
	if(IOCP_SUCESS != _network.destroy())
	{
		assert(0);
		//LOGGER_CFATAL(theLogger, _T("�޷�ֹͣ����ģ��,������[%d].\r\n"), _network.getLastError());
	}
	if(IOCP_NULLKEY != _sListen)
	{
		_sListen = IOCP_NULLKEY;
	}
	if(INVALID_SOCKET != _sockNewClient)
	{
		closesocket(_sockNewClient);
		_sockNewClient = INVALID_SOCKET;
	}

	/*
	* �ͷ�ʣ��δ�Ͽ��Ŀͻ�������,��տͻ�IP��.
	* �������ͷ�ʣ�µ��׽��ֺ͹�������,
	* ����Ҫ����,���е�����,���ж�ʱ��,�̶߳��Ѿ�ֹͣ����,���ٻ�����Դ����.
	*/
	if(_connections.size() > 0)
	{
		//LOGGER_CWARNING(theLogger, _T("�˳�ʱ����:[%d]������,��ǿ�ƹر�.\r\n"), _connections.size());
	}
	for(connection_map_t::iterator iter = _connections.begin(); iter != _connections.end(); ++iter)
	{
		connection_context_t* conn = iter->second;
		conn->clientSock = IOCP_NULLKEY;
		freeConnectionContext(conn);
	}
	_connections.clear();
	
	delete _fcgiFactory;
	_fcgiFactory = NULL;
	_statusHandler = NULL;
	_connectionIps.clear();
	_isRuning = false;
}

int HTTPServer::stop()
{
	if(!runing()) return SE_STOPPED;
	doStop();

	//LOGGER_CINFO(theLogger, _T("Q++ HTTP Server ֹͣ.\r\n"));
	return SE_SUCCESS;
}

void HTTPServer::onAccept(bool sucess)
{
	/*
	* ʧ�ܵ�accept����,����,������һ��.
	*/
	if(!sucess)
	{
		//LOGGER_CWARNING(theLogger, _T("��׽��һ��ʧ�ܵ�AcceptEx����,������[%d].\r\n"), WSAGetLastError());
		shutdown(_sockNewClient, SD_BOTH);
		closesocket(_sockNewClient);
		_sockNewClient = INVALID_SOCKET;
		doAccept();
		return;
	}

	/*
	* �¿ͻ�����, ��ȡ�ͻ�IP����Ϣ, �����׽�����Ϣ,ʹ getsockname() �� getpeername() ����.
	* ���MSDN���� AcceptEx ��˵��.
	*/
	SOCKET sockListen = _network.getSocket(_sListen);
	if( 0 != setsockopt( _sockNewClient, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&sockListen, sizeof(sockListen)) )
	{
		//LOGGER_CERROR(theLogger, _T("�޷������׽�����Ϣ,������[%d].\r\n"), WSAGetLastError());
	}

	/* 
	* ����¿ͻ�IP�Ͷ˿�
	*/
	sockaddr_in clientAddr;
	int nAddrLen = sizeof(sockaddr_in);
	if( 0 != getpeername(_sockNewClient, (sockaddr *)&clientAddr, &nAddrLen) )
	{
		//LOGGER_CERROR(theLogger, _T("�޷���ȡ�ͻ��˵�ַ�Ͷ˿�,������[%d].\r\n"), WSAGetLastError());
	}
	std::string clientIp = inet_ntoa(clientAddr.sin_addr);
	unsigned int clientPort = ntohs(clientAddr.sin_port);

	/*
	* �������Ӷ���.
	*/
	SOCKET sNewClient = _sockNewClient;
	_sockNewClient = INVALID_SOCKET;
	connection_context_t* conn = NULL;
	bool kicked = false;
	bool refused = false;

	_lock.lock();
	do 
	{
		/*
		* �鿴�Ƿ��Ѿ��ﵽ�������.
		*/
		if(_connections.size() >= this->maxConnections())
		{
			/*
			* �Ѿ��ﵽ���������,ֱ�ӹر��׽���.
			*/
			refused = true;
			break;
		}
		
		/*
		* ����IP������������Ƿ�ﵽ����,�����߳�.
		*/
		str_int_map_t::iterator iterClientIP = _connectionIps.end();
		if(this->maxConnectionsPerIp() > 0)
		{
			iterClientIP = _connectionIps.find(clientIp);
			if(iterClientIP != _connectionIps.end() && iterClientIP->second >= this->maxConnectionsPerIp())
			{
				kicked = true;
				break;
			}
		}

		/*
		* Ϊ�¿ͻ�������Դ.
		*/
		conn = allocConnectionContext(clientIp, clientPort);
		conn->clientSock = _network.add(sNewClient, this->sessionTimeout(), 0, this->maxConnectionSpeed());
		_connections.insert(std::make_pair(conn->clientSock, conn));
		if(this->maxConnectionsPerIp() > 0)
		{
			/*
			* ��¼��ǰ�ͻ�һ���Ѿ��ж��ٸ�������.
			*/
			if(iterClientIP != _connectionIps.end()) iterClientIP->second++;
			else _connectionIps.insert(std::make_pair(clientIp, 1));
		}
	} while(false);
	_lock.unlock();

	/*
	* �����Ӷ���ʼ����HTTP����
	*/
	if( conn != NULL )
	{
		/* ���������� */
		//LOGGER_CINFO(theLogger, _T("[%s:%d] - �����ӱ�����.\r\n"), AtoT(clientIp).c_str(), clientPort);
		
		/* ����״̬ */
		if(_statusHandler)
		{
			_statusHandler->onNewConnection(clientIp.c_str(), clientPort, false, false);
		}

		/* ��ʼ���� */
		assert(conn->request);
		int ret = conn->request->run(conn, conn->clientSock, recvTimeout());
		if(CT_SUCESS != ret)
		{
			/* ����ʧ�� */
			doConnectionClosed(conn, ret);
		}
		else
		{
			/* 
			* ���ճɹ�, ���治Ӧ���з��� conn �Ĵ���, �����������¼�������ģ��, 
			* conn->request->recv() �ɹ���,����֤ conn �������Ч��.
			*/
		}
	}
	else
	{
		/* �����ӱ��ܾ� */
		if(refused)
		{
			//LOGGER_CWARNING(theLogger, _T("[%s:%d] - �������Ѿ��ﵽ���������,���ӱ�����.\r\n"), AtoT(clientIp).c_str(), clientPort);
		}
		else if(kicked)
		{
			//LOGGER_CWARNING(theLogger, _T("[%s:%d] - �ͻ����ӱ��ܾ�,������IP������������������.\r\n"), AtoT(clientIp).c_str(), clientPort);
		}
		else
		{
			assert(0);
		}

		/* ����״̬ */
		if(_statusHandler)
		{
			_statusHandler->onNewConnection(clientIp.c_str(), clientPort, refused, kicked);
		}

		/* �ر��׽��� */
		shutdown(sNewClient, SD_BOTH);
		closesocket(sNewClient);
	}

	
	/*
	* ׼��������һ������
	*/
	doAccept();
}

/*
* responder ������ϻ���ʧ�ܺ���� doRequestDone ��ʾһ�������Ѿ��������.
* ���� conn->responder һ������Ч��.
*/
void HTTPServer::doRequestDone(connection_context_t* conn, int status)
{
	assert(conn->request);
	assert(conn->responder);
	
	/*
	* ��¼��Ҫ�����ݷ��͵�״̬�ӿ�
	*/
	__int64 bytesSent = 0;
	__int64 bytesRecv = conn->request->size();
	if(conn->responder)
	{
		bytesRecv += conn->responder->getTotalRecvBytes();
		bytesSent += conn->responder->getTotalSentBytes();
	}
	size_t totalTime = conn->request->startTime() == 0 ? 0 : static_cast<unsigned int>(_hrt.getMs(_hrt.now() - conn->request->startTime()));
	std::string clientIp(conn->ip);
	unsigned int clinetPort = conn->port;
	std::string uri = conn->request->uri(true);
	int svrCode = conn->responder->getServerCode();

	/*
	* д��־��¼һ�������Ѿ��������.
	*/
	std::string strBytes = format_size(bytesSent);
	std::string strSpeed = format_speed(bytesSent, totalTime);
	//LOGGER_CTRACE(theLogger, _T("[%s:%d] - ��Ӧͷ:\r\n%s"), AtoT(clientIp).c_str(), clinetPort, AtoT(conn->responder->getHeader()).c_str());
	//LOGGER_CINFO(theLogger, _T("[%s:%d] - [%s]����%s[HTTP %d],��������[%s],��ʱ[%.3fs],ƽ���ٶ�[%s].\r\n"), 
	//	AtoT(clientIp).c_str(), clinetPort, AtoT(uri).c_str(), status == CT_SENDCOMPLETE ? _T("���") : _T("��ֹ"), svrCode,
	//	AtoT(strBytes).c_str(), totalTime * 1.0 / 1000, AtoT(strSpeed).c_str());
	std::string resp = conn->responder->getHeader();

	if(_statusHandler)
	{
		_statusHandler->onRequestEnd(clientIp.c_str(), clinetPort, uri.c_str(), svrCode,
			bytesSent, bytesRecv, totalTime, status == CT_SENDCOMPLETE);
	}

	/*
	* ��� �ͻ���Ҫ�� keep-alive ,��������
	*/
	if(status == CT_SENDCOMPLETE && conn->request->keepAlive())
	{
		/* ����connection��״̬ */
		assert(conn->responder);
		conn->request->reset();
		conn->responder->reset();
		delete conn->responder;
		conn->responder = NULL;

		_network.refresh(conn->clientSock);

		/* ��ʼ������һ������ */
		int ret = conn->request->run(conn, conn->clientSock, keepAliveTimeout());
		if(CT_SUCESS != ret)
		{
			/* ����ʧ�� */
			doConnectionClosed(conn, ret);
		}
		else
		{
			/* 
			* ���ճɹ�, ���治Ӧ���з��� conn �Ĵ���, �����������¼�������ģ��, 
			* conn->request->recv() �ɹ���,����֤ conn �������Ч��.
			*/
		}
	}
	else
	{
		doConnectionClosed(conn, status);
	}
}

/*
* �ر�����,�����ǳɹ����������ر�,Ҳ��������Ӧʧ�ܺ�رջ����������ʧ��.
* conn->request ����Ч��,��������ͷ��һ������.
*/
void HTTPServer::doConnectionClosed(connection_context_t* conn, int status)
{
	/*
	* �ȼ�¼����
	* ���ӵ�IP��ַ,�˿ں͸������ܹ�ռ�õ�ʱ��
	*/
	std::string clientIp(conn->ip);
	unsigned int clinetPort = conn->port;
	unsigned int connTime = static_cast<unsigned int>(_hrt.getMs(_hrt.now() - conn->startTime));

	/*
	* ��ֹͣ���ڸ����ӵ����е���,ȷ����ȫ����ܻ�����Դ.
	* 1. ��ʱ���������߳�
	* 2. ����ģ��
	*/
	_network.remove(conn->clientSock);
	//TRACE("HTTPServer::doRequestDone() remove iocp key 0x%x.\r\n", conn->clientSock);

	/*
	* �Ӷ��������,�������IP��.
	*/
	_lock.lock();
	_connections.erase(conn->clientSock);
	if(this->maxConnectionsPerIp() > 0)
	{
		str_int_map_t::iterator iter = _connectionIps.find(conn->ip);
		if(iter != _connectionIps.end())
		{
			if( --(iter->second) <= 0) _connectionIps.erase(iter);
		}
	}
	_lock.unlock();

	conn->clientSock = IOCP_NULLKEY;
	freeConnectionContext(conn);

	/*
	* ��־��״̬֪ͨ
	*/
	std::string txt("");
	switch(status)
	{
	case CT_CLIENTCLOSED: { txt = "�ͻ��˹ر�������"; break; }
	case CT_SENDCOMPLETE: { txt = "���ݷ������"; break; }
	case CT_SEND_TIMEO: { txt = "���ͳ�ʱ"; break; }
	case CT_RECV_TIMEO: { txt = "���ճ�ʱ"; break; }
	case CT_SESSION_TIMEO: { txt = "�Ự��ʱ"; break; }
	case CT_FCGI_CONNECT_FAILED: { txt = "�޷����ӵ�FCGI������"; break; }
	default: { txt = "δ֪"; break; }
	}
	//LOGGER_CINFO(theLogger, _T("[%s:%d] - ���ӱ��ر�[%s],�ܼ���ʱ[%.3fs].\r\n"), 
	//	AtoT(clientIp).c_str(), clinetPort, AtoT(txt).c_str(), connTime * 1.0 / 1000);
	if(_statusHandler)
	{
		_statusHandler->onConnectionClosed(clientIp.c_str(), clinetPort, static_cast<HTTP_CLOSE_TYPE>(status));
	}
}


int HTTPServer::onRequestDataReceived(IRequest* request, size_t bytesTransfered)
{
	connection_context_t *conn = reinterpret_cast<connection_context_t *>(request->getConnectionId());

	/* ֪ͨ״̬����ӿ�,�Ա�ͳ�ƴ��� */
	if(_statusHandler)
	{
		_statusHandler->onDataReceived(conn->ip, conn->port, bytesTransfered);
	}
	return 0;
}

int HTTPServer::onResponderDataSent(IResponder *responder, size_t bytesTransfered)
{
	connection_context_t *conn = reinterpret_cast<connection_context_t *>(responder->getConnectionId());

	/* ֪ͨ״̬����ӿ�,�Ա�ͳ�ƴ��� */
	if(_statusHandler)
	{
		_statusHandler->onDataSent(conn->ip, conn->port, bytesTransfered);
	}
	return 0;
}

void HTTPServer::onRequest(IRequest* request, int status)
{
	connection_context_t* conn = reinterpret_cast<connection_context_t*>(request->getConnectionId());
	if(status != CT_SUCESS)
	{
		/* ���ճ�ʱ����ʧ��,�޷���������ͷ,ֱ�ӹر����� */
		doConnectionClosed(conn, status);
		return;
	}

	/*
	* �Ѿ����յ�һ������,����һ����Ӧ.
	*/
	std::string serverFileName;
	std::string uri = request->uri(true);

	/*
	* ���յ�һ������ͷ,trace
	*/
	char method[100];
	map_method(request->method(), method);
	//LOGGER_CTRACE(theLogger, _T("[%s:%d] - ����ͷ:\r\n%s"), AtoT(conn->ip).c_str(), conn->port, AtoT(request->getHeader()).c_str());
	//LOGGER_CINFO(theLogger, _T("[%s:%d] - [%s][%s]...\r\n"), AtoT(conn->ip).c_str(), conn->port, AtoT(method).c_str(), AtoT(uri).c_str());  
	if(_statusHandler)
	{
		_statusHandler->onRequestBegin(conn->ip, conn->port, uri.c_str(),request->method());
	}
	
	/* ��ʼ�������� */
	do
	{
		/* ӳ��Ϊ�������ļ��� */
		if(mapServerFilePath(uri, serverFileName))
		{
			/* ����һ��Responder���� */
			if(conn->request->isValid())
			{
				if(_fcgiFactory && _fcgiFactory->catchRequest(serverFileName))
				{
					/* ����һ��FCGI��Ӧ */
					conn->responder = new FCGIResponder(this, &_network, _fcgiFactory);
					break;
				}
			}
		}

		//byte data[1024] = { 0 };
		//int len = 0;
		//request->read(data, request->contentLength());
	
		/* ����һ����̬��Ӧ���ڴ���Ĭ������ */
		assert(conn->responder == NULL);
		conn->responder = new HTTPResponder(this, &_network);
	}while(0);

	/*
	* ������Ӧ
	*/
	int ret = conn->responder->run(conn, conn->clientSock, conn->request);
	if(CT_SUCESS != ret)
	{
		doRequestDone(conn, ret);
	}
	else
	{
		/* 
		* ���治Ӧ�������κδ������ conn, �¼�����ģʽ��,�޷���֤ conn ����Ч�� 
		*/
	}
}

void HTTPServer::onResponder(IResponder *responder, int status)
{
	/*
	* ������ϻ��߳���
	*/
	connection_context_t* conn = reinterpret_cast<connection_context_t*>(responder->getConnectionId());
	doRequestDone(conn, status);
}

/*
* IOCP����ģ��Ļص�����.
*/
void HTTPServer::IOCPCallback(iocp_key_t s, int flags, bool result, int transfered, byte* buf, size_t len, void* param)
{

	HTTPServer* instPtr = reinterpret_cast<HTTPServer*>(param);

	assert(flags & IOCP_ACCEPT);

	/*
	* accpet �ص�.
	*/
	instPtr->onAccept(result);
}
