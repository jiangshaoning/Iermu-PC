#include "StdAfx.h"
#include "FCGIFactory.h"
#include "HTTPDef.h"
#include <process.h>

/*
* FCGI ����������
*/
#define FCGI_SERVER_TYPE_LOCALE 0
#define FCGI_SERVER_TYPE_REMOTE 1

/*
*
*/
#define FCGI_CONNECTION_BUSY 0
#define FCGI_CONNECTION_CONNECTING 1
#define FCGI_CONNECTION_IDLE 2
#define FCGI_CONNECTION_DISCONNECT 3

FCGIFactory::FCGIFactory(IHTTPServer *httpServer, IOCPNetwork *network)
	: _fcgiRequestIdSeed(1), _network(network), _maxConn(200), _fcgiServer(NULL), _httpServer(httpServer),
	_maxWait(SIZE_T_MAX), _hrt(true), _inited(false), _cacheAll(false)
{
}


FCGIFactory::~FCGIFactory()
{
}

int FCGIFactory::init(const std::string &name, unsigned int port, const std::string &fileExts, size_t maxConn, size_t maxWait, bool cacheAll)
{
	_maxConn = maxConn;
	_maxWait = maxWait;
	_cacheAll = cacheAll;

	/* ת��Ϊ��д��ĸͳһ�Ƚ� */
	char exts[MAX_PATH + 1] = {0}; /* ��չ���ַ������ MAX_PATH ���ַ� */
	strcpy(exts, fileExts.c_str());
	strupr(exts);
	_fileExts = exts;

	/* ��¼FCGI Server ����Ϣ */
	_fcgiServer = new fcgi_server_context_t;
	memset(_fcgiServer, 0, sizeof(fcgi_server_context_t));
	strcpy(_fcgiServer->name, name.c_str());

	if(port == 0)
	{
		_fcgiServer->type = FCGI_SERVER_TYPE_LOCALE;
		_fcgiServer->processList = new fcgi_process_list_t;
	}
	else
	{
		_fcgiServer->type = FCGI_SERVER_TYPE_REMOTE;
		_fcgiServer->port = port;
	}

	_inited = true;
	return FCGIF_SUCESS;
}

int FCGIFactory::destroy()
{
	/* �����˳���־ */
	_lock.lock();
	_inited = false;
	_lock.unlock();

	/* �ͷ��������� */
	while(_workingFcgiConnList.size() > 0)
	{
		//_workingFcgiConnList.front()->comm = IOCP_NULLKEY;
		freeConnectionContext(_workingFcgiConnList.front());
		_workingFcgiConnList.pop_front();
	}
	while( _idleFcgiConnList.size() > 0)
	{
		//_idleFcgiConnList.front()->comm = IOCP_NULLKEY;
		freeConnectionContext(_idleFcgiConnList.front());
		_idleFcgiConnList.pop_front();
	}

	/* ��յȴ����� */
	_waitingList.clear();

	/* ɱ��FCGI���� */
	if(_fcgiServer)
	{
		if(_fcgiServer->processList)
		{
			while(_fcgiServer->processList->size() > 0)
			{
				freeProcessContext(_fcgiServer->processList->front());
				_fcgiServer->processList->pop_front();
			}
			delete _fcgiServer->processList;
		}
		delete _fcgiServer;
		_fcgiServer = NULL;
	}

	/* ����״̬ */
	_fcgiRequestIdSeed = 1;
	return 0;
}

bool FCGIFactory::catchRequest(const std::string &fileName)
{
	/*
	* Ŀ��ű��ļ��Ƿ����
	*/
	//if(!WINFile::exist(AtoT(fileName).c_str())) return false;

	std::string exts;
	_lock.lock();
	if(_inited) exts = _fileExts;
	else exts = "";
	_lock.unlock();

	/*
	* �ж�url�Ƿ����ָ������չ��
	*/
	std::string ext;
	get_file_ext(fileName, ext);
	char extStr[MAX_PATH] = {0};
	strcpy(extStr, ext.c_str());
	strupr(extStr);
	ext = extStr;

	return match_file_ext(ext, exts);
}

FCGIFactory::fcgi_process_context_t* FCGIFactory::allocProcessContext()
{
	fcgi_process_context_t *context = new fcgi_process_context_t;
	memset(context, 0, sizeof(fcgi_process_context_t));
	context->instPtr = this;
	context->processInfo = new PROCESS_INFORMATION;
	memset(context->processInfo, 0, sizeof(PROCESS_INFORMATION));
	return context;
}

bool FCGIFactory::freeProcessContext(fcgi_process_context_t *context)
{
	/* ����һ���Ѿ��ȱ��ر��� */
	assert(context->conn == NULL);

	/* ��������߳�����������,�ȴ����� */
	if(context->thread)
	{
		if( WAIT_OBJECT_0 != WaitForSingleObject(reinterpret_cast<HANDLE>(context->thread), 0) )
		{
			TerminateThread(reinterpret_cast<HANDLE>(context->thread), 1);
		}
		CloseHandle(reinterpret_cast<HANDLE>(context->thread));
	}

	/* ���FCGI������������,��ֹ֮ */
	if(context->processInfo->hProcess)
	{
		if(WAIT_OBJECT_0 != WaitForSingleObject(context->processInfo->hProcess, 0))
		{
			TerminateProcess(context->processInfo->hProcess, 1);
		}
		CloseHandle(context->processInfo->hThread);
		CloseHandle(context->processInfo->hProcess);
	}
	delete context->processInfo;
	delete context;
	return true;
}

fcgi_conn_t* FCGIFactory::allocConnectionContext()
{
	fcgi_conn_t *conn = new fcgi_conn_t;
	memset(conn, 0, sizeof(fcgi_conn_t));

	conn->requestId = _fcgiRequestIdSeed++;
	if(conn->requestId == 0) conn->requestId = 1;
	conn->cacheAll = _cacheAll;
	conn->instPtr = this;

	return conn;
}

FCGIFactory::fcgi_process_context_t* FCGIFactory::getProcessContext(fcgi_conn_t *conn)
{
	fcgi_process_context_t *procContext = NULL;
	if(_fcgiServer->processList)
	{
		for(fcgi_process_list_t::iterator iter = _fcgiServer->processList->begin(); iter != _fcgiServer->processList->end(); ++iter)
		{
			if( conn == (*iter)->conn )
			{
				procContext = *iter;
				break;
			}
		}
	}
	return procContext;
}

bool FCGIFactory::freeConnectionContext(fcgi_conn_t *conn)
{
	if(conn->comm != IOCP_NULLKEY)
	{
		_network->remove(conn->comm);
	}
	delete conn;

	/*
	* ���ڱ���ģʽ,ÿ�����̶��ɶ�Ӧ��һ������.
	*/
	fcgi_process_context_t *procContext = getProcessContext(conn);
	if(procContext) procContext->conn = NULL;

	return true;
}

void FCGIFactory::onConnect(fcgi_conn_t *conn, bool sucess)
{
	//TRACE("fcgi connection: 0x%x %s.\r\n", conn, sucess ? "connected" : "unconnect"); 
	fcgi_conn_ready_func_t waitingFunc = NULL;
	void *waitingParam = NULL;

	_lock.lock();
	
	/* ȡ���ȴ����еĶ�ͷ���� */
	if(_waitingList.size() > 0)
	{
		waitingFunc = _waitingList.front().first;
		waitingParam = _waitingList.front().second;
		_waitingList.pop_front();
	}

	if(!sucess)
	{
		/* ����ʧ��,��æµ�������Ƴ�(����ǰconn�Ѿ�������æµ����) */
		_workingFcgiConnList.remove(conn);

		/* �ͷ����� */
		freeConnectionContext(conn);
		conn = NULL;
	}
	else
	{
		/* ���ӳɹ�,����conn��æµ������(����ǰconn�Ѿ�������æµ����) */
		/* ���û�еȴ��е�����,�������ж��� */
		if(waitingFunc)
		{
			/* ��Ȼ������æµ������ */
		}
		else
		{
			_workingFcgiConnList.remove(conn);
			_idleFcgiConnList.push_back(conn);
		}
	}
	_lock.unlock();

	if(waitingFunc)
	{
		waitingFunc(conn, waitingParam);
	}
}

void FCGIFactory::IOCPCallback(iocp_key_t s, int flags, bool result, int transfered, byte* buf, size_t len, void* param)
{
	assert(flags & IOCP_CONNECT);
	fcgi_conn_t *conn = reinterpret_cast<fcgi_conn_t*>(param);
	FCGIFactory *instPtr = reinterpret_cast<FCGIFactory*>(conn->instPtr);
	instPtr->onConnect(conn, result);
}

bool FCGIFactory::getConnection(fcgi_conn_t *&connOut, fcgi_conn_ready_func_t callbackFunc, void *param)
{
	assert(_fcgiServer);
	fcgi_conn_t *conn = NULL;
	bool sucess = false;
	connOut = NULL;
	fcgi_conn_ready_func_t expiredFunc = NULL;
	void *expiredParam = NULL;

	_lock.lock();
	do
	{
		if(!_inited) break;

		/* ���Դӿ������Ӷ����з��� */
		if(_idleFcgiConnList.size() > 0)
		{
			conn = _idleFcgiConnList.front();
			_idleFcgiConnList.pop_front();
			_workingFcgiConnList.push_back(conn);
			sucess = true;
			connOut = conn;
			break;
		}

		/* ���û�дﵽ���������,���´���һ������ */
		if(_workingFcgiConnList.size() < _maxConn)
		{
			/* ����һ��������,���ҽ���æµ���� */
			/* �ѻ�ȡ�����������ȴ�����,һ�������ӳ�ʼ�������ص� */
			conn = allocConnectionContext(); 
			_workingFcgiConnList.push_back(conn);
			_waitingList.push_back(std::make_pair(callbackFunc, param));
			
			//TRACE("fcgi connections: 0x%x created.\r\n", conn);

			/* ��ʼ�������� */
			if(initConnection(conn))
			{
				/* �½������Ѿ��ɹ�,��������,�ȴ��ص�����֪ͨ */
				sucess = true;
			}
			else
			{
				/* ��������ʧ��,��������,�ָ��ֳ� */
				assert(0);
				_waitingList.pop_back();
				_workingFcgiConnList.pop_back();
				freeConnectionContext(conn);
			}

			break;
		}

		/* ����Ѿ��ﵽ���������,����ȴ����� */
		if(_waitingList.size() >= _maxWait)
		{
			/* ����ȴ����дﵽ�������,�򵯳���ͷ��¼,�ص�֪ͨ��ʱ,Ȼ��ѵ�ǰ������뵽��β */
			expiredFunc = _waitingList.front().first;
			expiredParam = _waitingList.front().second;

			_waitingList.pop_front();
		}
		_waitingList.push_back(std::make_pair(callbackFunc, param));
		sucess = true;
		
	}while(0);
	_lock.unlock();

	/* ������������ȴ�-ʧ��֪ͨ */
	if(expiredFunc)
	{
		expiredFunc(NULL, expiredParam);
	}

	return sucess;
}

void FCGIFactory::releaseConnection(fcgi_conn_t* conn, bool good)
{
	fcgi_conn_ready_func_t waitingFunc = NULL;
	void *waitingParam = NULL;

	_lock.lock();
	do
	{
		if(!_inited) break;

		/* �����������,��ر�֮ */
		if(!good)
		{
			_workingFcgiConnList.remove(conn);
			freeConnectionContext(conn);
			conn = NULL;
		}
	
		/* ���ȴ����� */
		if( _waitingList.size() > 0)
		{
			if(!conn)
			{
				/* �Ѿ��пռ�,�㹻����һ�������� */
				conn = allocConnectionContext(); 
				_workingFcgiConnList.push_back(conn);

				/* ��ʼ�������� */
				if(initConnection(conn))
				{
					/* �½������Ѿ��ɹ�,��������,�ȴ��ص�����֪ͨ */
				}
				else
				{
					assert(0);
					_workingFcgiConnList.pop_back();
					freeConnectionContext(conn);
					conn = NULL;
				}
			}
			else
			{
				/* �ѵ�ǰ����(��õ�)������ȴ����ж�ͷ������,conn��Ȼ������æµ������ */
				waitingFunc = _waitingList.front().first;
				waitingParam = _waitingList.front().second;
				_waitingList.pop_front();
			}
		}
		else
		{
			/* ����Ȼ���õ����ӷ�����ж����� */
			if(conn)
			{
				_workingFcgiConnList.remove(conn);

				conn->idleTime = _hrt.now();
				_idleFcgiConnList.push_back(conn);

				//TRACE("fcgi connection: 0x%x becomes idle.\r\n", conn);
			}
		}
		
		/* ά�����ж���һ�� */
		maintain();

	}while(0);
	_lock.unlock();

	/*
	* ֪ͨ�ȴ������еĶ�ͷ����
	*/
	if(waitingFunc)
	{
		waitingFunc(conn, waitingParam);
	}
}

/*
* ����û��ʹ�ö�ʱ��,��ʵ�ϲ��ܱ�֤�ܾ����ͷŶ���Ŀ�������,��Ҫ���µ������ͷ�ʱ�Ŵ��� maintain()
* ���Ǵ�һ���ϳ�������������,���ԴﵽĿ��.
*/
void FCGIFactory::maintain()
{
	/*
	* �ر����п��г��� FCGI_MAX_IDLE_SECONDS ������,���ж��������ٻ���һ������(���Ǹոձ��ͷŵ��Ǹ�)
	*/
	if(_idleFcgiConnList.size() > 1)
	{
		for(fcgi_conn_list_t::iterator iter = _idleFcgiConnList.begin(); iter != _idleFcgiConnList.end(); )
		{
			fcgi_conn_t *conn = *iter;
			if(_hrt.getMs(_hrt.now() - conn->idleTime) >= FCGI_MAX_IDLE_SECONDS * 1000)
			{
				/* ���Ӷ�Ӧ�ı��ؽ��� */
				fcgi_process_context_t *procContext = getProcessContext(conn);

				/* �ر����� */
				_idleFcgiConnList.erase(iter++);
				freeConnectionContext(conn);

				/* �����Ӷ�Ӧ�ı��� FCGI ����ɱ�� */
				if(procContext)
				{
					_fcgiServer->processList->remove(procContext);
					freeProcessContext(procContext);
				}
			}
			else
			{
				++iter;
			}
		}
	}
}

bool FCGIFactory::initConnection(fcgi_conn_t *conn)
{
	assert(_fcgiServer);
	if(_fcgiServer->type == FCGI_SERVER_TYPE_REMOTE)
	{
		/* �����׽��� */
		SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if( INVALID_SOCKET == s )
		{
			assert(0);
			//LOGGER_CERROR(theLogger, _T("�޷�ΪFCGI���Ӵ����׽���,������[%d].\r\n"), WSAGetLastError());
			return false;
		}
		else
		{
			/* ���׽��ֵ����ص�����һ���˿� */
			sockaddr_in localAddr;
			memset(&localAddr, 0, sizeof(sockaddr_in));
			localAddr.sin_port = 0;
			localAddr.sin_family = AF_INET;
			localAddr.sin_addr.s_addr = INADDR_ANY;
			if(SOCKET_ERROR == bind(s, reinterpret_cast<sockaddr*>(&localAddr), sizeof(sockaddr_in)))
			{
				closesocket(s);
				assert(0);
				//LOGGER_CERROR(theLogger, _T("�޷�ΪFCGI�����׽��ְ󶨱��ض˿�,������[%d].\r\n"), WSAGetLastError());
				return false;
			}

			/* ��ӵ�����ģ��,û�лỰ��ʱҲ�������ٶ� */
			conn->comm = _network->add(s, 0, 0, 0);

			/* ���ӵ�FCGI������ */
			sockaddr_in fcgiServerAddr;
			memset(&fcgiServerAddr, 0, sizeof(sockaddr_in));
			fcgiServerAddr.sin_port = htons(_fcgiServer->port);
			fcgiServerAddr.sin_family = AF_INET;
			fcgiServerAddr.sin_addr.s_addr = inet_addr(_fcgiServer->name);

			if( IOCP_PENDING == _network->connect(conn->comm, reinterpret_cast<sockaddr*>(&fcgiServerAddr), NULL, 0, _httpServer->sendTimeout(), IOCPCallback, conn))
			{
				return true;
			}
			else
			{
				//LOGGER_CERROR(theLogger, _T("�޷����ӵ�FCGI������[%s:%d],������[%d].\r\n"), AtoT(_fcgiServer->name).c_str(), _fcgiServer->port, _network->getLastError());
				return false;
			}
		}
	}
	else
	{
		/*
		* Ϊ�����ӷ���һ�����е� FCGI ���ؽ���,���û�п��н���,�򴴽�һ��
		*/
		fcgi_process_context_t *context = NULL;
		for(fcgi_process_list_t::iterator iter = _fcgiServer->processList->begin(); iter != _fcgiServer->processList->end(); ++iter)
		{
			if( (*iter)->conn == NULL )
			{
				context = (*iter);
				break;
			}
		}
		if(!context)
		{
			/* ��Ҫ�´���һ������ */
			assert(_fcgiServer->processList->size() < _maxConn);
			context = allocProcessContext();
			_fcgiServer->processList->push_back(context);
		}
		else
		{
			/* ����Ҫ�����½���,Ӧ�ð���һ������ʱ���߳̾���ر�,�Ա��´���һ���߳� */
			assert(context->thread);
			CloseHandle(reinterpret_cast<HANDLE>(context->thread));
			context->thread = NULL;
		}
		context->conn = conn;

		/*
		* ����һ���߳�,���̺߳�����ִ�����²���:
		* 1. ����Ψһ�������ܵ��ļ���(���Ƕ�� HTTP server ʵ��,Ӧȷ����������ϵͳ��(�����ǽ���������)��Ψһ��.
		* 2. ����һ��FCGI���ؽ���.
		* 3. �������ܵ����ӵ��´�����FCGI���ؽ���.
		* 4. �ص� FCGIFactory ֪ͨ���.
		*/
		context->thread = _beginthreadex(NULL, 0, spawnChild, context, 0, NULL);
		if(context->thread == -1)
		{
			context->thread = 0;
			//LOGGER_CERROR(theLogger, _T("�޷�Ϊ����FCGI���ؽ�������һ�������߳�,_beginthreadex����ʧ��,������:%d.\r\n"), errno);
			
			_fcgiServer->processList->remove(context);
			freeProcessContext(context);
			return false;
		}
		else
		{
			return true;
		}
	}
}

unsigned __stdcall FCGIFactory::spawnChild(void *param)
{
	fcgi_process_context_t *context = reinterpret_cast<fcgi_process_context_t*>(param);
	bool sucess = false;

	do
	{
		/* �ж��Ƿ���Ҫ�´���һ������: 1.���̾��ΪNULL,��ʾ�·���; 2.���̾����Ϊ��,�����ź�,��ʾ�����Ѿ��˳���,Ҳ��Ҫ���´��� */
		if( context->processInfo->hProcess != NULL )
		{
			if(WAIT_OBJECT_0 == WaitForSingleObject(context->processInfo->hProcess, 0))
			{
				CloseHandle(context->processInfo->hThread);
				CloseHandle(context->processInfo->hProcess);
				memset(context->processInfo, 0, sizeof(PROCESS_INFORMATION));
			}
		}
		if(  context->processInfo->hProcess == NULL )
		{
			/* ����һ��Ψһ�Ĺܵ��� */
			unsigned int seed = static_cast<unsigned int>(time( NULL )); /* ȷ����ͬexeʵ���䲻ͬ */
			seed += reinterpret_cast<unsigned int>(context); /* ȷ��ͬһ��exeʵ����,��ͬ���̲߳�ͬ */
			srand(seed);
			sprintf(context->pipeName, "%s\\%04d_%04d", FCGI_PIPE_BASENAME, rand() % 10000, rand() % 10000);
			//TRACE("pipename:%s.\r\n", context->pipeName);

			/* ����һ�������ܵ� */
			HANDLE hPipe = CreateNamedPipe(AtoT(context->pipeName).c_str(),  PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_READMODE_BYTE,
				PIPE_UNLIMITED_INSTANCES,
				4096, 4096, 0, NULL);
			if( INVALID_HANDLE_VALUE == hPipe )
			{
				//LOGGER_CERROR(theLogger, _T("�޷����������ܵ�,����FCGI��������ʧ��,������:%d\r\n"), GetLastError());
				break;
			}
			if(!SetHandleInformation(hPipe, HANDLE_FLAG_INHERIT, TRUE))
			{
				//LOGGER_CERROR(theLogger, _T("SetHandleInformation()����ʧ��,����FCGI��������ʧ��,������:%d\r\n"), GetLastError());
				break;
			}

			/* �������ܵ�ΪSTDIN����һ������FCGI���� */
			STARTUPINFO startupInfo;
			memset(&startupInfo, 0, sizeof(STARTUPINFO));
			startupInfo.cb = sizeof(STARTUPINFO);
			startupInfo.dwFlags = STARTF_USESTDHANDLES;
			startupInfo.hStdInput  = hPipe;
			startupInfo.hStdOutput = INVALID_HANDLE_VALUE;
			startupInfo.hStdError  = INVALID_HANDLE_VALUE;

			if( !CreateProcess(AtoT(context->instPtr->_fcgiServer->name).c_str(), NULL, NULL, NULL, TRUE,

#ifdef _DEBUG /* ����״̬��,������PHP-CGI���̴�����̨����, releaseʱ��������̨���� */	
				0, 
#else
				CREATE_NO_WINDOW,
#endif

				NULL, NULL, &startupInfo, context->processInfo))
			{
				//LOGGER_CERROR(theLogger, _T("CreateProcess()����ʧ��,�޷����ɱ���FCGI����,����:%s.\r\n"), AtoT(get_last_error()).c_str());
				break;
			}
		}

		/* �ȴ������ܵ� */
		if(!WaitNamedPipe(AtoT(context->pipeName).c_str(), FCGI_CONNECT_TIMEO))
		{
			//LOGGER_CERROR(theLogger, _T("���������ܵ�ʧ��[%s]\r\n"), AtoT(get_last_error()).c_str());
			break;
		}

		/* ����һ���ļ����,���������ܵ� */
		HANDLE hConn = CreateFile(AtoT(context->pipeName).c_str(),
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);
		if( INVALID_HANDLE_VALUE == hConn )
		{
			//LOGGER_CERROR(theLogger, _T("CreateFile()����ʧ��,�޷����������ܵ�,������:%d\r\n"), GetLastError());
			break;
		}

		/* �����ܵ����ӳɹ� */
		context->conn->comm = context->instPtr->_network->add(hConn, 0, 0, 0);
		sucess = true;

	}while(0);

	/* �ص���� */
	context->instPtr->onConnect(context->conn, sucess);

	return 0;
}
