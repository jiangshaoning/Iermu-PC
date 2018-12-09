#include "StdAfx.h"
#include <assert.h>
#include "IOCPNetwork.h"
#include <mswsock.h>
#include <process.h>

IOCPNetwork::IOCPNetwork()
	: _iocpHandle(NULL),
	_threads(0),
	_tids(NULL),
	_lastErrorCode(0),
	_inited(false),
	_hrt(true)
{
}

IOCPNetwork::~IOCPNetwork()
{
	assert(_inited == false);
}

bool IOCPNetwork::initWinsockLib(WORD nMainVer, WORD nSubVer)
{
	WORD wVer;
	WSADATA ws;
	wVer = MAKEWORD(nMainVer, nSubVer);
	return WSAStartup(wVer, &ws) == 0;
}

bool IOCPNetwork::cleanWinsockLib()
{
	return WSACleanup() == 0;
}

int IOCPNetwork::init(int threads)
{
	/*
	* �������
	* ������ܻ�ô���������,Ĭ�ϴ���5���߳�
	*/

	if(threads <= 0)
	{
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		threads = sysInfo.dwNumberOfProcessors;
	}
	if(threads <= 0 || threads > 64) threads = 5; 
	
	/*
	* ������ɶ˿�
	*/
	assert(_iocpHandle == NULL);
	if( NULL == (_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, threads)) )
	{
		_lastErrorCode = GetLastError();
		destroy();
		return IOCP_UNDEFINED;
	}

	/*
	* ��ʼ����ʱ������
	*/
	_timerQueue.init();

	/*
	* ����ͬ�������
	*/
	_lockPool.init(0);

	/*
	* ���������߳�
	*/
	_threads = threads;
	assert(_tids == NULL);
	if(_tids) delete []_tids;
	_tids = new uintptr_t[threads];
	assert(_tids);
	memset(_tids, 0, sizeof(uintptr_t) * threads);

	for(int i = 0; i < _threads; ++i)
	{
		if(0 == (_tids[i] = _beginthreadex(NULL, 0, serviceProc, this, 0, NULL)))
		{
			_lastErrorCode = errno;
			destroy();
			return IOCP_UNDEFINED;
		}
	}

	_inited = true;
	return IOCP_SUCESS;
}

int IOCPNetwork::destroy()
{
	/*
	* destroy ��Ҫ�ȴ������߳̽���,��������߳��е���destroy,������.
	*/
	if(_tids)
	{
		uintptr_t curThread = reinterpret_cast<uintptr_t>(GetCurrentThread());
		for(int i = 0; i < _threads; ++i)
		{
			if(curThread == _tids[i]) return IOCP_DESTROYFAILED;
		}
	}

	/* 
	* ��������ֹͣ��־ 
	*/
	_inited = false;

	/*
	* �����˳�֪ͨ,ȷ�������̶߳���GetQueuedCompletionStatus�����з��ز���ֹ����.
	*/
	if(_tids)
	{
		assert(_threads > 0);
		if(_iocpHandle)
		{
			for(int i = 0; i < _threads; ++i)
			{
				PostQueuedCompletionStatus(_iocpHandle, 0, NULL, NULL);
			}
		}

		for(int i = 0; i < _threads; ++i)
		{
			if( _tids[i] ) 
			{
				WaitForSingleObject(reinterpret_cast<HANDLE>(_tids[i]), INFINITE);
				CloseHandle(reinterpret_cast<HANDLE>(_tids[i])); 
			}
		}
		delete []_tids;
	}
	_tids = NULL;
	_threads = 0;

	/*
	* �ر���ɶ˿ھ��
	*/
	if(_iocpHandle)
	{
		if(!CloseHandle(_iocpHandle))
		{
			assert(0);
		}
		_iocpHandle = NULL;
	}

	/*
	* �رն�ʱ��
	*/
	_timerQueue.destroy();

	/*
	* ������Դ
	*/
	if(_contextMap.size() > 0)
	{
		for(context_map_t::iterator iter = _contextMap.begin(); iter != _contextMap.end(); ++iter)
		{
			iocp_context_t *context = iter->second;

			/*
			* �ر��׽���
			*/
			if(!(context->status & IOCP_HANDLE_CLOSED))
			{
				closeHandle(context);
				context->status |= IOCP_HANDLE_CLOSED;
			}
			freeContext(context);
		}
		_contextMap.clear();
	}

	/*
	* ͬ������
	*/
	_lockPool.destroy();

	return IOCP_SUCESS;
}

IOCPNetwork::iocp_context_t* IOCPNetwork::allocContext()
{
	iocp_context_t* context = new iocp_context_t;
	memset(context, 0, sizeof(iocp_context_t));
	context->instPtr = this;
	return context;
}

void IOCPNetwork::freeContext(iocp_context_t* context)
{
	delete context;
}

void IOCPNetwork::cleanOlp(iocp_overlapped_t* olp)
{
	/*
	* ���������ص��ṹ��ĳЩֵ.
	*/
	olp->oppType = IOCP_NONE;
	if(olp->timer != NULL)
	{
		_timerQueue.deleteTimer(olp->timer, false);
		olp->timer = NULL;
	}
	memset(&olp->olp, 0, sizeof(OVERLAPPED));
	olp->buf = NULL;
	olp->len = 0;
	olp->realLen = 0;
	olp->param = NULL;
}

void IOCPNetwork::closeHandle(iocp_context_t *context)
{
	if(context->type == IOCP_HANDLE_SOCKET)
	{
		SOCKET s = reinterpret_cast<SOCKET>(context->h);
		shutdown(s, SD_BOTH);
		closesocket(s);
	}
	else
	{
		CloseHandle(context->h);
	}
}

bool IOCPNetwork::sessionTimeout(iocp_context_t* context)
{
	if(context->sessionTimeo == 0)
	{
		return false;
	}
	else
	{
		return _hrt.getMs(_hrt.now() - context->startCounter) >= context->sessionTimeo;
	}
}

iocp_key_t IOCPNetwork::add(HANDLE f, unsigned long sessionTimeo, size_t readSpeedLmt, size_t writeSpeedLmt, bool sync)
{
	return add(f, sessionTimeo, readSpeedLmt, writeSpeedLmt, true, sync);
}

iocp_key_t IOCPNetwork::add(SOCKET s, unsigned long sessionTimeo, size_t readSpeedLmt, size_t writeSpeedLmt, bool sync)
{
	return add(reinterpret_cast<HANDLE>(s), sessionTimeo, readSpeedLmt, writeSpeedLmt, false, sync);
}

iocp_key_t IOCPNetwork::add(HANDLE h, unsigned long sessionTimeo, size_t readSpeedLmt, size_t writeSpeedLmt, bool isFile, bool sync)
{
	iocp_context_t* context = allocContext();

	context->h = h;
	if(isFile) context->type = IOCP_HANDLE_FILE;
	else context->type = IOCP_HANDLE_SOCKET;
	context->lockPtr = NULL;
	context->readOlp.speedLmt = readSpeedLmt;
	context->writeOlp.speedLmt = writeSpeedLmt;
	context->startCounter = _hrt.now();
	context->sessionTimeo = sessionTimeo;

	bool ret = true;
	_lock.lock();
	do
	{
		if( _iocpHandle != CreateIoCompletionPort(h, _iocpHandle, reinterpret_cast<ULONG_PTR>(context), 0))
		{
			assert(0);
			ret = false;
			break;
		}
		else
		{
			if(sync)
			{
				context->lockPtr = _lockPool.allocate();
			}
			else
			{
				context->lockPtr = NULL;
			}
			_contextMap.insert(std::make_pair(h, context));
		}
	}while(0);
	_lock.unlock();

	if(ret)
	{
		//TRACE("iocp key 0x%x allocated.\r\n", context);
		return context;
	}
	else
	{
		freeContext(context);
		return IOCP_NULLKEY;
	}
}

SOCKET IOCPNetwork::getSocket(iocp_key_t key)
{
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(key);
	assert(context->type == IOCP_HANDLE_SOCKET);
	return reinterpret_cast<SOCKET>(context->h);
}

//iocp_key_t IOCPNetwork::getKey(SOCKET s)
//{
//	iocp_key_t ret = IOCP_NULLKEY;
//	_lock.lock();
//	context_map_t::iterator iter = _contextMap.find(s);
//	if(iter != _contextMap.end())
//	{
//		ret = iter->second;
//	}
//	_lock.unlock();
//	return ret;
//}

bool IOCPNetwork::refresh(iocp_key_t key)
{
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(key);
	bool ret = false;

	if(context->lockPtr) context->lockPtr->wlock();
	if(context->readOlp.oppType != IOCP_NONE || context->writeOlp.oppType != IOCP_NONE || context->status != IOCP_NORMAL)
	{
		/* ״̬����,��û�� IO �������ڽ���ʱ������ˢ�� */
	}
	else
	{
		context->startCounter = _hrt.now();
		ret = true;
	}
	if(context->lockPtr) context->lockPtr->unlock();

	return ret;
}

bool IOCPNetwork::busy(iocp_key_t key)
{
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(key);
	bool ret = false;

	if(context->lockPtr) context->lockPtr->wlock();
	ret = context->readOlp.oppType != IOCP_NONE || context->writeOlp.oppType != IOCP_NONE;
	if(context->lockPtr) context->lockPtr->unlock();

	return ret;
}

int IOCPNetwork::cancel(iocp_key_t key)
{
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(key);
	int ret = IOCP_SUCESS;

	/*
	* ������ٶ����ƶ�ʱ��,����ɾ����ʱ��.
	*/
	if(context->lockPtr) context->lockPtr->wlock();

	if(context->readOlp.oppType == IOCP_DELAY_READ)
	{
		context->readOlp.oppType = IOCP_NONE;
		_timerQueue.deleteTimer(context->readOlp.timer, false);
		context->readOlp.timer = NULL;
	}
	if(context->writeOlp.oppType == IOCP_DELAY_WRITE)
	{
		context->writeOlp.oppType = IOCP_NONE;
		_timerQueue.deleteTimer(context->writeOlp.timer, false);
		context->writeOlp.timer = NULL;
	}

	/* 
	* �����ʱ���첽�������ڽ���,��ر��׽���ʹ�첽����ʧ�ܲ�����. 
	*/
	if(context->status == IOCP_NORMAL && (context->readOlp.oppType != IOCP_NONE || context->writeOlp.oppType != IOCP_NONE))
	{
		closeHandle(context);
		context->status |= (IOCP_CANCELED | IOCP_HANDLE_CLOSED);
		ret = IOCP_PENDING;
	}
	if(context->lockPtr) context->lockPtr->unlock();

	return ret;
}

/*
* ���׽��־����IOCPģ�����Ƴ�,�����׽��־�������ر�.
* ֻ�����첽��������ɺ�����Ƴ�.
*/
//
int IOCPNetwork::remove(iocp_key_t key)
{
	if( !_inited ) return IOCP_UNINITIALIZED;
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(key);
	bool isBusy = false;
	
	/*
	* ����׽��ִ���æµ״̬,������ɾ����־���ر��׽���.
	*/
	if(context->lockPtr) context->lockPtr->wlock();
	if( context->readOlp.oppType != IOCP_NONE || context->writeOlp.oppType != IOCP_NONE )
	{
		isBusy = true;
	}
	else
	{
		/* ����״̬,����һ��ɾ����־,��ֹ�������� */
		context->status |= IOCP_REMOVE;
	}
	if(context->lockPtr) context->lockPtr->unlock();

	/*
	* ����׽��ִ���æµ״̬,��ر�
	*/
	if(isBusy)
	{
		return IOCP_BUSY;
	}
	
	/*
	* ɾ��������ʱ��.��ʱ����.
	*/
	if(context->readOlp.timer != NULL)
	{
		assert(0);
		_timerQueue.deleteTimer(context->readOlp.timer, true);
		context->readOlp.timer = NULL;
	}
	if(context->writeOlp.timer != NULL)
	{
		assert(0);
		_timerQueue.deleteTimer(context->writeOlp.timer, true);
		context->writeOlp.timer = NULL;
	}

	/*
	* �ر��׽���
	*/
	if(!(context->status & IOCP_HANDLE_CLOSED))
	{
		closeHandle(context);
		context->status |= IOCP_HANDLE_CLOSED;
	}

	/*
	* �Ƴ���¼
	*/
	_lock.lock();
	_contextMap.erase(context->h);
	if(context->lockPtr)
	{
		_lockPool.recycle(context->lockPtr);
		context->lockPtr = NULL;
	}
	_lock.unlock();

	/*
	* ������Դ
	*/
	freeContext(context);
	return IOCP_SUCESS;
}

void IOCPNetwork::readTimeoutProc(void* param, unsigned char)
{
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(param);

	if(context->lockPtr) context->lockPtr->wlock();
	if(context->readOlp.oppType != IOCP_NONE && context->status == IOCP_NORMAL)
	{
		context->instPtr->closeHandle(context);
		context->status |= (IOCP_READTIMEO | IOCP_HANDLE_CLOSED);
	}
	if(context->lockPtr) context->lockPtr->unlock();
}

void IOCPNetwork::writeTimeoutProc(void* param, unsigned char)
{
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(param);

	if(context->lockPtr) context->lockPtr->wlock();
	if(context->writeOlp.oppType != IOCP_NONE && context->status == IOCP_NORMAL)
	{
		context->instPtr->closeHandle(context);
		context->status |= (IOCP_WRITETIMEO | IOCP_HANDLE_CLOSED);
	}
	if(context->lockPtr) context->lockPtr->unlock();
}

void IOCPNetwork::delaySendProc(void* param, unsigned char)
{
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(param);
	iocp_proc_t func = NULL;
	void* callbackParam = NULL;
	int flags = IOCP_NONE;
	byte* buf = NULL;
	size_t len = 0;
	int ret = IOCP_PENDING;

	/*
	* ɾ����ʱ�����Ķ�ʱ��.
	*/
	assert(context->writeOlp.timer);
	if(context->writeOlp.timer)
	{
		/* �ص������ڲ�Ӧ�õȴ���ʱ��,��������� */
		context->instPtr->_timerQueue.deleteTimer(context->writeOlp.timer, false);
		context->writeOlp.timer = NULL;
	}

	if(context->lockPtr) context->lockPtr->rlock();
	if(context->status != IOCP_NORMAL)
	{
		ret = IOCP_BUSY;
	}
	else
	{
		if(context->writeOlp.oppType != IOCP_DELAY_WRITE)
		{
			/* ��ʱ���Ͷ�ʱ����ȡ��ʱ,��ʱ���������ڵȴ������ٽ�� */
		}
		else
		{
			context->writeOlp.oppType = IOCP_SEND;
			if(context->writeOlp.timeout > 0)
			{
				context->writeOlp.timer = context->instPtr->_timerQueue.createTimer(context->writeOlp.timeout, writeTimeoutProc, context);
			}

			ret =  context->instPtr->realSend(context);

			/*
			* ������ղ���ʧ��,Ӧ�ûָ��ֳ�,�Ա���һ�ε��ÿ��Գɹ�.
			*/
			if(IOCP_PENDING != ret)
			{
				func = context->writeOlp.iocpProc;
				callbackParam = context->writeOlp.param;
				flags = context->writeOlp.oppType;
				buf = context->writeOlp.buf;
				len = context->writeOlp.len;

				context->instPtr->cleanOlp(&context->writeOlp);
			}
		}
	}
	if(context->lockPtr) context->lockPtr->unlock();


	if(func)
	{
		/* ģ��һ��ʧ�ܵĽ�� */
		/* ����ص���������,���Ӱ�춨ʱ����׼ȷ��*/
		func(context, flags, false, 0, buf, len, callbackParam);
	}
}

void IOCPNetwork::delayRecvProc(void* param, unsigned char)
{
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(param);
	
	iocp_proc_t func = NULL;
	void* callbackParam = NULL;
	int flags = IOCP_NONE;
	byte* buf = NULL;
	size_t len = 0;
	int ret = IOCP_PENDING;

	/*
	* ɾ����ʱ�����Ķ�ʱ��.
	* ���ʹ�õ� Windows TimerQueue,�����ڴ˴�ɾ����ʱ��,��Ϊ�ڶ�ʱ���ص�������ɾ����ʱ���ǲ��ܳɹ���.
	*/
	assert(context->readOlp.timer);
	if(context->readOlp.timer)
	{
		/* �ص������ڲ�Ӧ�õȴ���ʱ��,��������� */
		context->instPtr->_timerQueue.deleteTimer(context->readOlp.timer, false);
		context->readOlp.timer = NULL;
	}

	if(context->lockPtr) context->lockPtr->rlock();
	if(context->status != IOCP_NORMAL)
	{
		ret = IOCP_BUSY;
	}
	else
	{
		if(context->readOlp.oppType != IOCP_DELAY_READ)
		{
		}
		else
		{
			context->readOlp.oppType = IOCP_RECV;
			
			/*
			* ������ʱ��ʱ��
			*/
			if(context->readOlp.timeout > 0)
			{
				context->readOlp.timer = context->instPtr->_timerQueue.createTimer(context->readOlp.timeout, readTimeoutProc, context);
			}

			int ret =  context->instPtr->realRecv(context);

			/*
			* ������ղ���ʧ��,Ӧ�ûָ��ֳ�,�Ա���һ�ε��ÿ��Գɹ�.
			*/
			if(IOCP_PENDING != ret)
			{
				func = context->readOlp.iocpProc;
				callbackParam = context->readOlp.param;
				flags = context->readOlp.oppType;
				buf = context->readOlp.buf;
				len = context->readOlp.len;

				context->instPtr->cleanOlp(&context->readOlp);
			}
		}
	}
	if(context->lockPtr) context->lockPtr->unlock();


	if(func)
	{
		/* ģ��һ��ʧ�ܵĽ�� */
		/* ����ص���������,���Ӱ�춨ʱ����׼ȷ��*/
		func(context, flags, false, 0, buf, len, callbackParam);
	}
}

unsigned int __stdcall IOCPNetwork::serviceProc(void* lpParam)
{
	IOCPNetwork* instPtr = reinterpret_cast<IOCPNetwork*>(lpParam);

	while(1)
	{
		DWORD transfered = 0;
		iocp_context_t* context = NULL;
		iocp_overlapped_t* iocpOlpPtr = NULL;
		if(!GetQueuedCompletionStatus(instPtr->_iocpHandle, &transfered, reinterpret_cast<PULONG_PTR>(&context), reinterpret_cast<LPOVERLAPPED*>(&iocpOlpPtr), INFINITE))
		{
			if(iocpOlpPtr)
			{
				/*
				* �ɹ���IOCP������ȡ��һ����,����I/O���������Ϊʧ��.
				* ԭ��1: Զ�������ر�������.
				* ԭ��2: ���ڳ�ʱ,�����׽��ֱ� shutdown.
				* ԭ��3: �����򲻳���. (���ڵ����� CancelIoEx �����첽������ȡ��. GetLastError() == ERROR_OPERATION_ABORTED)
				* 
				*/
				//assert(0);
				instPtr->onIoFinished(context, false, iocpOlpPtr, transfered);
			}
			else
			{
				/*
				* IOCP������ر�.
				* �ڱ������߼������ڷ������˳�.
				*/
				assert(0);
				instPtr->_lastErrorCode = GetLastError();
				return IOCP_UNDEFINED;
			}
		}
		else
		{
			/*
			* Լ���������˳���־
			*/
			if(transfered == 0 && iocpOlpPtr == NULL && context == NULL)
			{
				//TRACE("IOCPNetwork::serviceProc()IOCP�����߳��˳�.\r\n");
				break;
			}
			
			/*
			* ����MSDN��˵��GetQueuedCompletionStatus()����TRUE[ֻ]��ʾ��IOCP�Ķ�����ȡ��һ���ɹ����IO�����İ�.
			* ����"�ɹ�"������ֻ��ָ���������������ɹ����,������ɵĽ���ǲ��ǳ�����Ϊ��"�ɹ�",��һ��.
			* 
			* 1. AcceptEx �� ConnectEx �ɹ��Ļ�,�����Ҫ��һ����/��������(��ַ�����ݳ���),��ô transfered == 0����.
			* 2. Send, Recv �������������Ļ��������ȴ���0,��transfered == 0Ӧ���ж�Ϊʧ��.
			*
			* ʵ�ʲ��Է��ֽ��ܿͻ�������,ִ��һ��Recv����,Ȼ��ͻ������϶Ͽ�,�������е�����,���� Recv transfered == 0����.
			* �ܶ���֮,�ϲ�Ӧ���ж�������������(������AcceptEx��ConnectEx���յ�Զ�̵�ַ,��ר��ָ���ݲ���)���������ȴ���0,
			* �����صĽ����ʾ transfered = 0 ˵������ʧ��.
			*
			* ����ģ�鱾���޷����� transfered�Ƿ����0���жϲ����Ƿ�ɹ�,��Ϊ�ϲ���ȫ����Ͷ��һ������������Ϊ0��Recv����.
			* ���ڷ������������ǳ��õļ���,������Լ�ڴ�.
			*
			*/
			instPtr->onIoFinished(context, true, iocpOlpPtr, transfered);
		}
	}
	return 0;
}

void IOCPNetwork::onIoFinished(iocp_context_t* context, bool result, iocp_overlapped_t* iocpOlpPtr, int transfered)
{
	//TRACE("IOCPNetwork::onIoFinished() iocp key 0x%x, socket %d called.\r\n", context, context->s);
	/*
	* ����״̬,�رն�ʱ��,����ͳ����Ϣ
	*/
	iocp_proc_t func = NULL;
	void* callbackParam = NULL;
	int flags = IOCP_NONE;
	byte* buf = NULL;
	size_t len = 0;

	/*
	* ɾ����ʱ��
	*/
	if(iocpOlpPtr->timer != NULL)
	{
		_timerQueue.deleteTimer(iocpOlpPtr->timer, true);
		iocpOlpPtr->timer = NULL;
		/* ��ʱ��ɾ����,�ڻص�����ִ��ǰ,�������в���ʹ�� iocpOlpPtr */
	}

	/*
	* һ���첽�����Ѿ�����,����״̬�����֮.
	* ��ȡ�׽���״ֵ̬,�Ի�ò���ʧ�ܵ�ԭ��.
	*/
	if(context->lockPtr) context->lockPtr->rlock();

	if(result) iocpOlpPtr->transfered += transfered;
	iocpOlpPtr->lastCompleteCounter = _hrt.now();

	func = iocpOlpPtr->iocpProc;
	callbackParam = iocpOlpPtr->param;
	flags = iocpOlpPtr->oppType;
	buf = iocpOlpPtr->buf;
	len = iocpOlpPtr->len;

	cleanOlp(iocpOlpPtr);

	if(context->status != IOCP_NORMAL)
	{
		flags |= context->status;
	}
	if(context->lockPtr) context->lockPtr->unlock();
	
	/*
	* �ص����
	*/
	if(func)
	{
		func(context, flags, result, transfered, buf, len, callbackParam);
	}
}

int IOCPNetwork::accept(iocp_key_t key, SOCKET sockNew, byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param)
{
	if( !_inited ) return IOCP_UNINITIALIZED;
	if(len < (sizeof(sockaddr_in) + 16) * 2) return IOCP_BUFFERROR; /* ���������Ȳ��� */
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(key);
	assert(IOCP_HANDLE_SOCKET == context->type);

	/*
	* ���AcceptEx()����ָ�벢����֮,�ο�MSDN����AcceptEx��ʾ������.
	*/
	SOCKET sockListen = reinterpret_cast<SOCKET>(context->h);
	DWORD dwBytesReceived = 0;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	DWORD dwBytes = 0;
	if( 0 != WSAIoctl(sockListen, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL) )
	{
		assert(0);
		_lastErrorCode = WSAGetLastError();
		return IOCP_UNDEFINED;
	}

	/*
	* ���ñ��,Ȼ�����AcceptEx,������ȷ��ͬ����ȷ
	* ���ڷ��ͺͽ��շֱ��Ӧ��ͬ���ص��ṹ,���Կ����ö�д��ͬʱ����,ֻҪ�׽���״̬����.
	*/
	int ret = IOCP_PENDING;
	if(context->lockPtr) context->lockPtr->rlock();
	if(context->status != IOCP_NORMAL)
	{
		ret = context->status; 
	}
	else if(sessionTimeout(context))
	{
		ret = IOCP_SESSIONTIMEO;
	}
	else
	{
		/* assert���:��ҪͬʱͶ�ݶ��ͬ���͵Ĳ���,�ɳ����߼�ȷ����һ��,������ͬ��������߲�����,����ֻ���û�������. */
		assert(context->readOlp.oppType == IOCP_NONE);
		context->readOlp.oppType = IOCP_ACCEPT;
		context->readOlp.buf = buf;
		context->readOlp.len = len;
		context->readOlp.realLen = len;
		context->readOlp.iocpProc = func;
		context->readOlp.param = param;
		context->readOlp.timeout = timeout;
		if(timeout > 0)
		{
			/* �ȴ�����ʱ��,acceptExʧ����ɾ�����б�Ҫ��,��������û��ͬ��������� */
			context->readOlp.timer = _timerQueue.createTimer(timeout, readTimeoutProc, context);
		}

		if( !lpfnAcceptEx(sockListen, sockNew, buf, 
			len - (sizeof(sockaddr_in) + 16) * 2 , sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 
			&dwBytesReceived, reinterpret_cast<LPOVERLAPPED>(&context->readOlp)) )
		{
			if( WSA_IO_PENDING != WSAGetLastError())
			{
				assert(0);
				_lastErrorCode = WSAGetLastError();
				ret = IOCP_UNDEFINED;
			}
		}
		else
		{
			assert(0);
			ret = IOCP_UNDEFINED;
		}

		/*
		* ����ʧ��,�����־������Դ
		*/
		if(ret != IOCP_PENDING)
		{
			cleanOlp(&context->readOlp);
		}
	}

	if(context->lockPtr) context->lockPtr->unlock();

	return ret;
}

int IOCPNetwork::realRecv(iocp_context_t* context)
{
	DWORD dwTransfered = 0;
	DWORD dwLastError = 0;
	if(context->type == IOCP_HANDLE_SOCKET)
	{
		DWORD dwFlags = 0;
		WSABUF wsaBuf = { context->readOlp.len, reinterpret_cast<char*>(context->readOlp.buf) };

		SOCKET s = reinterpret_cast<SOCKET>(context->h);
		if(SOCKET_ERROR == WSARecv(s, &wsaBuf, 1, &dwTransfered, &dwFlags, reinterpret_cast<LPWSAOVERLAPPED>(&context->readOlp), NULL))
		{
			dwLastError = WSAGetLastError();
			if(dwLastError != WSA_IO_PENDING)
			{
				_lastErrorCode = dwLastError;
				return IOCP_UNDEFINED;
			}
		}
		return IOCP_PENDING;
	}
	else
	{
		if(!ReadFile(context->h, context->readOlp.buf, context->readOlp.len, &dwTransfered, reinterpret_cast<LPOVERLAPPED>(&context->readOlp)))
		{
			dwLastError = GetLastError();
			if(dwLastError != ERROR_IO_PENDING)
			{
				_lastErrorCode = dwLastError;
				return IOCP_UNDEFINED;
			}
		}
		return IOCP_PENDING;
	}
}

int IOCPNetwork::read(iocp_key_t key, byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param)
{
	return recv(key, buf, len, timeout, func, param);
}

int IOCPNetwork::recv(iocp_key_t key, byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param)
{
	if( !_inited ) return IOCP_UNINITIALIZED;
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(key);

	/*
	* ִ��WSARecv
	*/
	int ret = IOCP_PENDING;

	if(context->lockPtr) context->lockPtr->rlock();
	if(context->status != IOCP_NORMAL)
	{
		/* ����һ�����ղ�����һ�����Ͳ���ͬʱ���� */
		ret = context->status; 
	}
	else if(sessionTimeout(context))
	{
		ret = IOCP_SESSIONTIMEO;
	}
	else
	{
		assert(context->readOlp.oppType == IOCP_NONE);
		context->readOlp.oppType = IOCP_RECV;
		context->readOlp.buf = buf;
		context->readOlp.len = len;
		context->readOlp.timeout = timeout;
		assert(context->readOlp.timer == NULL);
		context->readOlp.iocpProc = func;
		context->readOlp.param = param;
		context->readOlp.realLen = len;

		/*
		* ����Ƿ񳬳��ٶ�����,�����,����ʱ����/����
		*/
		size_t delay = 0;
		if(context->readOlp.speedLmt > 0 && context->readOlp.lastCompleteCounter != 0)
		{
			/* 
			* ��������ٶ�����,���� nSended�������Ӧ���� nExpectTime ���ʱ������.
			*/
			__int64 expectTime = _hrt.getCounters(static_cast<__int64>(context->readOlp.transfered * 1.0 / context->readOlp.speedLmt * 1000));
			expectTime += context->startCounter;

			if(expectTime > context->readOlp.lastCompleteCounter) /* ��ɵ�ʱ�����ǰ,˵���ٶȳ��� */
			{
				/* �������ʱ��������ʱ��. */
				delay = static_cast<size_t>(_hrt.getMs(expectTime - context->readOlp.lastCompleteCounter));
				if(delay > IOCP_MAXWAITTIME_ONSPEEDLMT)
				{
					/* ������ȴ�ʱ��,��һ�η���һ����С��. */
					delay = IOCP_MAXWAITTIME_ONSPEEDLMT;
					if(context->readOlp.len > IOCP_MINBUFLEN_ONSPEEDLMT)
					{
						context->readOlp.realLen = IOCP_MINBUFLEN_ONSPEEDLMT;
					}
				}
				else
				{
					/* ��һ�ο��Խ���һ������. */
					context->readOlp.realLen = context->readOlp.len;
				}
			}
		}


		/*
		* ֱ�ӷ��ͻ�������һ����ʱ����ʱ����
		*/
		if(delay > 0)
		{
			/*
			*  ����һ����ʱ�����Ķ�ʱ��,�����سɹ�. ���������Ľ���ȶ�ʱ�����ں���ͨ���ص�����֪ͨ.
			*/
			context->readOlp.oppType = IOCP_DELAY_READ;
			context->readOlp.timer = _timerQueue.createTimer(delay, delayRecvProc, context);
		}
		else
		{
			/*
			* ���ó�ʱ��ʱ��,Ȼ�����
			*/
			if(context->readOlp.timeout > 0)
			{
				context->readOlp.timer = _timerQueue.createTimer(context->readOlp.timeout, readTimeoutProc, context);
			}

			ret = realRecv(context);

			/*
			* ������ղ���ʧ��,Ӧ�ûָ��ֳ�,�Ա���һ�ε��ÿ��Գɹ�.
			*/
			if(IOCP_PENDING != ret)
			{
				cleanOlp(&context->readOlp);
			}
		}
	}
	if(context->lockPtr) context->lockPtr->unlock();
	
	return ret;
}

int IOCPNetwork::realSend(iocp_context_t* context)
{
	DWORD dwTransfered = 0;
	DWORD dwLastError = 0;

	if(context->type == IOCP_HANDLE_SOCKET)
	{
		WSABUF wsaBuf = { context->writeOlp.realLen, reinterpret_cast<char*>(context->writeOlp.buf) };
		SOCKET s = reinterpret_cast<SOCKET>(context->h);
		if(SOCKET_ERROR == WSASend(s, &wsaBuf, 1, &dwTransfered, 0, reinterpret_cast<LPWSAOVERLAPPED>(&context->writeOlp), NULL))
		{
			dwLastError = WSAGetLastError();
			if(dwLastError != WSA_IO_PENDING)
			{
				_lastErrorCode = dwLastError;
				return IOCP_UNDEFINED;
			}
		}
		return IOCP_PENDING;
	}
	else
	{
		if(!WriteFile(context->h, context->writeOlp.buf, context->writeOlp.len, &dwTransfered, reinterpret_cast<LPOVERLAPPED>(&context->writeOlp)))
		{
			dwLastError = GetLastError();
			if(dwLastError != ERROR_IO_PENDING)
			{
				_lastErrorCode = dwLastError;
				return IOCP_UNDEFINED;
			}
		}
		return IOCP_PENDING;
	}
}

int IOCPNetwork::write(iocp_key_t key, const byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param)
{
	return send(key, buf, len, timeout, func, param);
}

int IOCPNetwork::send(iocp_key_t key, const byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param)
{
	if( !_inited ) return IOCP_UNINITIALIZED;
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(key);

	/*
	* ִ��WSARecv
	*/
	int ret = IOCP_PENDING;

	if(context->lockPtr) context->lockPtr->rlock();
	if(context->status != IOCP_NORMAL)
	{
		/* ����һ�����ղ�����һ�����Ͳ���ͬʱ���� */
		ret = context->status; 
	}
	else if(sessionTimeout(context))
	{
		ret = IOCP_SESSIONTIMEO;
	}
	else
	{
		assert(context->writeOlp.oppType == IOCP_NONE);
		context->writeOlp.oppType = IOCP_SEND;
		context->writeOlp.buf = const_cast<byte*>(buf);
		context->writeOlp.len = len;
		context->writeOlp.timeout = timeout;
		assert(context->writeOlp.timer == NULL);
		context->writeOlp.iocpProc = func;
		context->writeOlp.param = param;
		context->writeOlp.realLen = len;

		/*
		* ����Ƿ񳬳��ٶ�����,�����,����ʱ����/����
		* ��һ�β��������
		*/
		size_t delay = 0;
		if(context->writeOlp.speedLmt > 0 && context->writeOlp.lastCompleteCounter != 0)
		{
			/* 
			* ��������ٶ�����,���� nSended�������Ӧ���� nExpectTime ���ʱ������.
			*/
			__int64 expectTime = _hrt.getCounters(static_cast<__int64>(context->writeOlp.transfered * 1.0 / context->writeOlp.speedLmt * 1000));
			expectTime += context->startCounter;

			if(expectTime > context->writeOlp.lastCompleteCounter) /* ��ɵ�ʱ�����ǰ,˵���ٶȳ��� */
			{
				/* �������ʱ��������ʱ��. */
				delay = static_cast<size_t>(_hrt.getMs(expectTime - context->writeOlp.lastCompleteCounter));
				if(delay > IOCP_MAXWAITTIME_ONSPEEDLMT)
				{
					/* ������ȴ�ʱ��,��һ�η���һ����С��. */
					delay = IOCP_MAXWAITTIME_ONSPEEDLMT;
					if(context->writeOlp.len > IOCP_MINBUFLEN_ONSPEEDLMT)
					{
						context->writeOlp.realLen = IOCP_MINBUFLEN_ONSPEEDLMT;
					}
				}
				else
				{
					/* ��һ�ο��Է���һ������. */
					context->writeOlp.realLen = context->writeOlp.len;
				}

				//TRACE(_T("delay send:%dms.\r\n"), delay);
			}
		}

		/*
		* ֱ�ӷ��ͻ�������һ����ʱ����ʱ����
		*/
		if(delay > 0)
		{
			context->writeOlp.oppType = IOCP_DELAY_WRITE;
			context->writeOlp.timer = _timerQueue.createTimer(delay, delaySendProc, context);
		}
		else
		{
			/*
			* ���ó�ʱ��ʱ��,Ȼ����
			*/
			if(context->writeOlp.timeout > 0)
			{
				context->writeOlp.timer = _timerQueue.createTimer(context->writeOlp.timeout, readTimeoutProc, context);
			}

			ret = realSend(context);

			if( ret != IOCP_PENDING)
			{
				cleanOlp(&context->writeOlp);
			}
		}
	}
	if(context->lockPtr) context->lockPtr->unlock();
	
	return ret;
}

int IOCPNetwork::connect(iocp_key_t key, sockaddr* addr, byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param)
{
	if( !_inited ) return IOCP_UNINITIALIZED;
	iocp_context_t* context = reinterpret_cast<iocp_context_t*>(key);
	assert(IOCP_HANDLE_SOCKET == context->type);

	/*
	* ȡ��ConnectEx����ָ��
	*/
	SOCKET s = reinterpret_cast<SOCKET>(context->h);
	DWORD dwBytesReceived = 0;
	GUID GuidConnectEx = WSAID_CONNECTEX;
	LPFN_CONNECTEX lpfnConnectEx = NULL;
	DWORD dwBytes = 0;
	if( 0 != WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx), &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, NULL, NULL) )
	{
		assert(0);
		_lastErrorCode = WSAGetLastError();
		return IOCP_UNDEFINED;
	}

	/*
	* ִ��ConnectEx
	*/

	int ret = IOCP_PENDING;
	DWORD bytesSent = 0;
	if(context->lockPtr) context->lockPtr->rlock();
	if(context->status != IOCP_NORMAL)
	{
		ret = context->status; 
	}
	else if(sessionTimeout(context))
	{
		ret = IOCP_SESSIONTIMEO;
	}
	else
	{
		assert(context->writeOlp.oppType == IOCP_NONE);
		context->writeOlp.oppType = IOCP_CONNECT;
		context->writeOlp.buf = buf;
		context->writeOlp.len = len;
		context->writeOlp.iocpProc = func;
		context->writeOlp.param = param;
		context->writeOlp.realLen = len;
		context->writeOlp.timeout = timeout;

		if(timeout > 0)
		{
			context->writeOlp.timer = _timerQueue.createTimer(timeout, writeTimeoutProc, context);
		}

		if( !lpfnConnectEx(s, addr, sizeof(sockaddr), buf, len, &bytesSent, reinterpret_cast<LPOVERLAPPED>(&context->writeOlp)) )
		{
			int errorCode = WSAGetLastError();
			if( WSA_IO_PENDING != errorCode )
			{
				assert(0);
				_lastErrorCode = errorCode;
				ret = IOCP_UNDEFINED;
			}
		}
		else
		{
			assert(0);
			ret = IOCP_UNDEFINED;
		}

		/*
		* ����ʧ��,�����־������Դ
		*/
		if(ret != IOCP_PENDING)
		{
			cleanOlp(&context->writeOlp);
		}
	}
	if(context->lockPtr) context->lockPtr->unlock();

	return ret;
}
