#pragma once
/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

/**************************
*
* Ŀ��: 
* ʵ�� Windowsƽ̨��֧�ֳ�ʱ���Ƶ�,�ɸ��õ�IOCP����ģ�Ϳ��(�������ͳ�ʱ,���ճ�ʱ,�Ự��ʱ).
* ʵ���ٶ�����.
* ����ͬʱ��һ��������(accept��recv)��һ��д����(connect��send).
* ��֧��ͬʱ�ж���첽�Ķ���������д����.
*
* ����ͬ��
* ����ͬһʱ��ֻ��һ�����͵Ĳ���,���Կ����ö���.�����漰��Ӱ����һ�����Ͳ����ĺ�����Ҫ��д��.
* �������Ҫͬʱ���ж���д����,���κ�ʱ����ֻ��һ�� IO �����ڽ���,��ô�Ϳ��Բ���ͬ��(add()���� sync = false ���Թر�ͬ��)
*
* �Ե����ߵ�Ҫ��
* 1. ����ȴ������첽������ɺ�(�ɹ�,ʧ�ܻ�ȡ��)���ܵ���removeɾ��.
* 2. ����һ���׽�ͬһʱ��ͬһ�����Ͳ���ֻ��һ��.
*/

#include <Winsock2.h>
#include <map>
#include "Lock.h"
#include "TimerQueue.h"

/* 
* Ԥ���峣��,��Ϊ��������ֵ���߲���,��Ҫʱ���Խ��� '&', '|' ����.
*/
const int IOCP_BUFFERROR = 0x0001;
const int IOCP_PENDING = 0x0002; /* �������ڽ����� */
const int IOCP_UNINITIALIZED = 0x0004;
const int IOCP_DESTROYFAILED = 0x0008;
const int IOCP_READTIMEO = 0x0010;
const int IOCP_SEND = 0x0020;
const int IOCP_RECV = 0x0040;
const int IOCP_CONNECT = 0x0080;
const int IOCP_ACCEPT = 0x0100;
const int IOCP_CANCELED = 0x0200;
const int IOCP_BUSY = 0x0400;
const int IOCP_REMOVE = 0x0800;
const int IOCP_WRITETIMEO = 0x1000;
const int IOCP_HANDLE_CLOSED = 0x2000;
const int IOCP_DELAY_READ = 0x4000;
const int IOCP_DELAY_WRITE = 0x8000;
const int IOCP_SESSIONTIMEO = 0x010000;
const int IOCP_ALL = 0x7FFFFFFF; /* ���г����ļ��� */
const int IOCP_UNDEFINED = 0xFFFFFFFF; /* δ������,ͨ��getLastError���Ի�ô����� */

/* ������Ԥ���峣�� */
void* const IOCP_NULLKEY = (void*)0;
const int IOCP_HANDLE_SOCKET = 1;
const int IOCP_HANDLE_FILE = 2;
const int IOCP_SUCESS = 0; /* �����ɹ� */
const int IOCP_NONE = 0; /* ����״̬ */
const int IOCP_NORMAL = 0; /* �׽���״̬����,���Ե�������sock���� */
const int IOCP_MAXWAITTIME_ONSPEEDLMT = 2000; /* �ٶ�����ʱ,�����ʱ2000����,��������ʱ���ܵ������ӱ��ͻ��˶Ͽ� */
const int IOCP_MINBUFLEN_ONSPEEDLMT = 512; /* �ٶ�����ʱ,��С�����ֽ��� */

/*
* ���Ͷ���
*/
typedef void* iocp_key_t;
typedef void (*iocp_proc_t)(iocp_key_t k, int flags, bool result, int transfered, byte* buf, size_t len, void* param); 

/*
* IOCPNetwork ����
*/
class IOCPNetwork : public INoCopy
{
protected:
	/*
	* �ص��ṹ����
	*/
	typedef struct iocp_overlapped_t
	{
		OVERLAPPED olp;
		int oppType;
		byte* buf;
		size_t len;
		size_t realLen; /* ʵ�ʴ��ݸ�recv �����ĳ���,�����ٶ����Ƶ�ԭ��,����С��len */
		iocp_proc_t iocpProc;
		void* param; /* �ص������Ķ������ */
		TimerQueue::timer_t timer; /* ��ʱ��ʱ��������ʱ������ʱ�� */
		size_t timeout; /* ��ʱʱ�� ms */
		size_t speedLmt; /* �ٶ����� B/s */
		__int64 lastCompleteCounter; /* ���һ�ε����ʱ�� */
		__int64 transfered; /* �ܼƴ��͵��ֽ��� */
	}IOCPOVERLAPPED;

	/*
	* IOCP�׽���context
	*/
	typedef struct iocp_context_t
	{
		HANDLE h; /* �ļ����,������ CreateFile ������ HANDLE Ҳ������ SOCKET �׽���*/
		iocp_overlapped_t readOlp; /* ������ accept, recv */
		iocp_overlapped_t writeOlp; /* д���� connect, send */
		RWLock* lockPtr; /* ͬ����д�� */
		int status; /* IOCP�׽���״̬,��д�������Ķ��� */
		__int64 startCounter; /* ��ʼʱ�� */
		unsigned long sessionTimeo; /* �Ự��ʱ,���� */
		IOCPNetwork* instPtr; /* ��ʵ��ָ��,�ڲ�ʹ�� */
		int type; /* �������:�ļ������׽��� */
	};
	typedef std::map<HANDLE, iocp_context_t*> context_map_t;

	/*
	* �ڲ����ݳ�Ա
	* _tids: �����߳�ID����
	* _inited: �Ƿ��Ѿ���ʼ����,�����,����key�Ĳ�����Ӧ��ʧ��.
	*
	* ͬ�������,����׽��ֹ���һ���򼸸�ͬ������,ֻ���CPU������ȼ���.
	* ���ʹ�ö�д����������޶���߲���ִ�е�Ч��.
	* ��д����ԭ����: ֻ�漰��һ������(�� accept, recv ����д connect, send)�Ĳ���ֻҪ�Ӷ���. �漰����������Ĳ���Ҫ��д��
	*/
	HANDLE _iocpHandle;
	int _threads;
	uintptr_t* _tids;
	bool _inited; 
	DWORD _lastErrorCode;

	context_map_t _contextMap;
	Lock _lock;
	TimerQueue _timerQueue;
	HighResolutionTimer _hrt;
	LockPool<RWLock> _lockPool;
	
	/*
	* �����̺߳������߶�ʱ���ص�����.
	* �������� iocp_context_t* ���͵�ֵ.
	*/
	static unsigned int __stdcall serviceProc(void* lpParam);
	static void __stdcall readTimeoutProc(void* param, unsigned char);
	static void __stdcall writeTimeoutProc(void* param, unsigned char);
	static void __stdcall delaySendProc(void* param, unsigned char);
	static void __stdcall delayRecvProc(void* param, unsigned char);

	/*
	* �첽�������ʱ,�����̵߳���onIOFinishʹ��IOCPNetwork�л����ڻص����ϲ�ʱ����һЩ����.
	* ����ɾ����ʱ��ʱ����.
	*/
	void onIoFinished(iocp_context_t* context, bool result, iocp_overlapped_t* olp, int transfered);

	/*
	* �ڲ���������
	*/
	int realSend(iocp_context_t* context);
	int realRecv(iocp_context_t* context);
	iocp_context_t* allocContext();
	void freeContext(iocp_context_t* context);
	void cleanOlp(iocp_overlapped_t* olp);
	bool sessionTimeout(iocp_context_t* context);
	iocp_key_t add(HANDLE h, unsigned long sessionTimeo, size_t readSpeedLmt, size_t writeSpeedLmt, bool isFile, bool sync);
	void closeHandle(iocp_context_t *context);

public:
	IOCPNetwork();
	virtual ~IOCPNetwork();

	/*
	* ����������� WS2 ��.
	*/
	static bool initWinsockLib(WORD nMainVer, WORD nSubVer);
	static bool cleanWinsockLib();

	/*
	* ��ʼ��������
	* threads: �����̵߳ĸ���,��� threads = 0 �򴴽���ǰCPU����ͬ�������߳���.
	* ����ֵ: Ԥ���峣���е�һ��.
	*/
	int init(int threads);
	int destroy();
	inline DWORD getLastError() { return _lastErrorCode; }

	/* 
	* �����׽��־������ɶ˿ھ��.
	* ����ֵ: Ԥ���峣���е�һ��.
	*/
	iocp_key_t add(SOCKET s, unsigned long sessionTimeo, size_t readSpeedLmt, size_t writeSpeedLmt, bool sync = false);
	iocp_key_t add(HANDLE f, unsigned long sessionTimeo, size_t readSpeedLmt, size_t writeSpeedLmt, bool sync = false);
	int remove(iocp_key_t key);
	int cancel(iocp_key_t key);
	bool busy(iocp_key_t key);
	SOCKET getSocket(iocp_key_t key);
	bool refresh(iocp_key_t key); /* ���ûỰ��ʱ,ֻ���ڿ���״̬�²���ˢ�³ɹ� */

	/*
	* �첽��������
	* ����s: �׽��־��.
	* ����key: ���ûص�����ʱ���ݲ���.
	* accept: ִ���첽accept����.����MSDN��˵��,�������� len >= (sizeof(sockaddr_in) + 16) * 2
	* recv: ��������.
	*
	* ����ֵ: Ԥ���峣���е�һ��.
	*/
	int accept(iocp_key_t key, SOCKET sockNew, byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param);
	int connect(iocp_key_t key, sockaddr* addr, byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param);
	int recv(iocp_key_t key, byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param);
	int send(iocp_key_t key, const byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param);
	int read(iocp_key_t key, byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param);
	int write(iocp_key_t key, const byte* buf, size_t len, size_t timeout, iocp_proc_t func, void* param);
};

