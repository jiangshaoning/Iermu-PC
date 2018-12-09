#include "StdAfx.h"
#include <assert.h>
#include "TimerQueue.h"
#include <process.h>

#define TIMER_ST_NORMAL 0
#define TIMER_ST_PROCESSING 1
#define TIMER_ST_EXPIRED 2

HighResolutionTimer::HighResolutionTimer(bool high)
{
	if(high)
	{
		/* ʹ�ø߾��ȼ����� */
		LARGE_INTEGER freq;
		if(!QueryPerformanceFrequency(&freq))
		{
			/* Ӳ����֧��,ֻ��ʹ�ú�����Ϊ������ */
			_frequency = 0;
		}
		else
		{
			_frequency = freq.QuadPart;
		}
	}
	else
	{
		/* ʹ�ú�����Ϊ������ */
		_frequency = 0;
	}
}

HighResolutionTimer::~HighResolutionTimer()
{
}

__int64 HighResolutionTimer::now()
{
	/* ���ص�ǰʱ���Ӧ��counter*/
	LARGE_INTEGER counter;
	if(_frequency == 0)
	{
		return GetTickCount();
	}
	else
	{
		if(!QueryPerformanceCounter(&counter)) return GetTickCount();
		return counter.QuadPart;
	}
}

__int64 HighResolutionTimer::getCounters(__int64 ms)
{
	/* ��msʱ�任��Ϊ�������� */
	if( 0 == _frequency )
	{
		/* Ӳ����֧�ָ߾��ȼ���,����������counter */
		return ms;
	}
	else
	{
		return ms * _frequency / 1000;
	}
}

__int64 HighResolutionTimer::getMs(__int64 counter)
{
	if( 0 == _frequency )
	{
		/* Ӳ����֧�ָ߾��ȼ���,��ʱ counter ���� ����*/
		return counter;
	}
	else
	{
		return static_cast<__int64>((counter * 1.0 / _frequency) * 1000);
	}
}

/*
* TimerQueue ʵ�� **************************************************************
********************************************************************************
*/
#define OPP_ADD 1
#define OPP_CHANGE 2
#define OPP_REMOVE 3

#define WT_UNDIFINE 0
#define WT_TIMEOUT 1
#define WT_RESET 2


TimerQueue::TimerQueue(size_t poolSize /* = 256 */)
	: _waitableTimer(NULL),
	_timerThread(0),
	_hrt(true),
	_lock(NULL),
	_timerList(NULL),
	_poolSize(poolSize),
	_poolPos(0),
	_timerPool(NULL),
	_oppList(NULL),
	_wakeupType(WT_UNDIFINE),
	_curTimer(NULL),
	_stop(false)
{
	
}

TimerQueue::~TimerQueue()
{
	destroy();
}

int TimerQueue::init()
{
	assert(_waitableTimer == NULL);
	/*
	* �����ȴ�������¼�����
	*/
	if( NULL == (_waitableTimer = CreateWaitableTimer(NULL, FALSE, NULL)))
	{
		assert(0);
		return TQ_ERROR;
	}

	/*
	* ���䶨ʱ�����кͲ�������
	*/
	_lock = new Lock;
	_timerList = new timer_list_t;
	if(_poolSize > 0)
	{	
		_timerPool = new timer_desc_t*[_poolSize];
	}
	_oppList = new opp_list_t;

	/*
	* ���������߳�
	*/
	if( 0 == (_timerThread = _beginthreadex(NULL, 0, workerProc, this, 0, NULL)))
	{
		assert(0);
		return TQ_ERROR;
	}

	_stop = false;
	return TQ_SUCCESS;
}

int TimerQueue::destroy()
{
	/*
	* ֹͣ�����߳�,Ȼ�������Դ.
	*/
	if(_timerThread)
	{
		assert(_oppList);

		/*
		* ���һ���յĶ�ʱ����Ϊֹͣ�ı�־.
		* Ȼ��ȴ������߳��˳�
		*/
		_lock->lock();
		_stop = true;
		wakeup();
		_lock->unlock();

		WaitForSingleObject(reinterpret_cast<HANDLE>(_timerThread), INFINITE);
		CloseHandle(reinterpret_cast<HANDLE>(_timerThread));
		_timerThread = 0;
	}

	if(_waitableTimer)
	{
		CloseHandle(_waitableTimer);
		_waitableTimer = NULL;
	}

	/*
	* ɾ�����ж�ʱ��
	*/
	if(_timerList)
	{
		for(timer_list_t::iterator iter = _timerList->begin(); iter != _timerList->end(); ++iter)
		{
			freeTimer(*iter);
		}
		if(_timerList->size() > 0)
		{
			//TRACE(_T("Undeleted timer:%d.\r\n"), _timerList->size());
			_timerList->clear();
		}
		delete _timerList;
		_timerList = NULL;
	}

	if(_timerPool)
	{
		for(size_t i = 0; i < _poolPos; ++i)
		{
			// ���ܵ��� freeTimer(),Ӧ��ֱ��ɾ��.
			delete _timerPool[i];
		}
		delete []_timerPool;
		_timerPool = NULL;
		_poolPos = 0;
	}

	if(_oppList)
	{
		delete _oppList;
		_oppList = NULL;
	}

	if(_lock)
	{
		delete _lock;
		_lock = NULL;
	}

	_stop = false;
	_curTimer = NULL;
	_wakeupType = WT_UNDIFINE;
	return TQ_SUCCESS;
}

TimerQueue::timer_desc_t* TimerQueue::allocTimer()
{
	timer_desc_t* timerPtr = NULL;
	if(_timerPool && _poolPos > 0)
	{
		timerPtr = _timerPool[--_poolPos];
	}
	else
	{
		timerPtr = new timer_desc_t;
	}
	memset(timerPtr, 0, sizeof(timer_desc_t));
	return timerPtr;
}

int TimerQueue::freeTimer(timer_desc_t* timerPtr)
{
	if(_timerPool && _poolPos < _poolSize)
	{
		_timerPool[_poolPos++] = timerPtr;
	}
	else
	{
		delete timerPtr;
	}
	return 0;
}

bool TimerQueue::isValidTimer(timer_desc_t* timerPtr)
{
	return timerPtr->st == TIMER_ST_NORMAL;
}

TimerQueue::timer_desc_t* TimerQueue::getFirstTimer()
{
	for(timer_list_t::iterator iter = _timerList->begin(); iter != _timerList->end(); ++iter)
	{
		if(isValidTimer(*iter))
		{
			return *iter;
		}
	}
	return NULL;
}

bool TimerQueue::isFirstTimer(timer_desc_t* timerPtr)
{
	return getFirstTimer() == timerPtr;
}

int TimerQueue::setNextTimer()
{
	/*
	* �������ö�ʱ����ʱ��
	*/
	_wakeupType = WT_TIMEOUT;
	_curTimer = getFirstTimer();

	if(NULL == _curTimer)
	{
		/* û�ж�ʱ����Ҫ���� */
		if(!CancelWaitableTimer(_waitableTimer))
		{
			assert(0);
		}
	}
	else
	{
		LARGE_INTEGER dueTime;
		__int64 curCounters = _hrt.now();
		if(curCounters >= _curTimer->expireCounters)
		{
			/*
			* ��ʱ���Ѿ���ʱ
			*/
			dueTime.QuadPart = 0;
		}
		else
		{
			/*
			* ��ʱ����û�г�ʱ
			*/
			__int64 ms;
			ms = _hrt.getMs(_curTimer->expireCounters - curCounters);
			
			dueTime.QuadPart = ms * 1000 * 10;
			dueTime.QuadPart = ~dueTime.QuadPart + 1; /*�����ڼ�����ڱ�ʾΪ����ֵȡ���ټ�һ(ȡ��ʱ����λ�Ѿ���Ϊ1��)*/

			//LOGGER_CINFO(theLogger, _T("Set timer expire after %dms.\r\n"), static_cast<int>(ms));
		}
		
		if(!SetWaitableTimer(_waitableTimer, &dueTime, 0, NULL, NULL, FALSE))
		{
			//LOGGER_CFATAL(theLogger, _T("SetWaitableTimer failed, error code:%d, handle:%x.\r\n"), GetLastError(), _waitableTimer );
			assert(0);
		}
		//TRACE(_T("SetTimer at:%d.\r\n"), curTime);
	}

	return 0;
}

void TimerQueue::oppExecute()
{
	if(!_oppList->empty())
	{
		for(opp_list_t::iterator iter = _oppList->begin(); iter != _oppList->end(); ++iter)
		{
			if(OPP_ADD == iter->second)
			{
				inqueue(iter->first);
			}
			else if(OPP_CHANGE == iter->second)
			{
				_timerList->remove(iter->first);
				inqueue(iter->first);
			}
			else if(OPP_REMOVE == iter->second)
			{
				_timerList->remove(iter->first);
				freeTimer(iter->first);
			}
			else
			{
				assert(0);
			}
		}
		_oppList->clear();
	}
}

bool TimerQueue::proc(HANDLE ce)
{
	_lock->lock();

	/* �Ƿ��˳� */
	if(_stop)
	{
		_lock->unlock();
		return false;
	}

	/*
	* ������ֶ����õĻ���,���ȴ����������,��Ϊ��ʱ��ͷ��ʱ��Ҫ���޸�
	*/
	if(WT_RESET == _wakeupType)
	{
		oppExecute();
	}
	else
	{
		/*
		* ����ǰ��ʱ��.
		*/
		assert(_curTimer);
		timer_desc_t* timerDescPtr = _curTimer;
		timerDescPtr->st = TIMER_ST_PROCESSING;
		timerDescPtr->ev = ce;
		_lock->unlock();

		/*
		* ���Բ鿴��ʱ������.
		*/
		//int ms = static_cast<int>(_hrt.getMs(_hrt.now() - timerDescPtr->expireCounters));
		//LOGGER_CINFO(theLogger, _T("TimerQueue - Timer deviation: %dms\r\n"), ms);
		//TRACE(_T("TimerQueue - Timer deviation: %dms\r\n"), ms);

		/* ִ�лص����� */
		if(timerDescPtr->funcPtr) timerDescPtr->funcPtr(timerDescPtr->param, TRUE);
	
		/*
		* ����ǰ�������� TIMER_ST_PROCESSING ��־,���Կ���ȷ�� timerDescPtr ָ���ʱ����Ч��.
		* �趨������־,�������֪ͨ
		*/
		_lock->lock();
		timerDescPtr->st = TIMER_ST_EXPIRED;
		if(timerDescPtr->waitting)
		{
			SetEvent(timerDescPtr->ev);
		}
		if(timerDescPtr->autoDelete)
		{
			_timerList->remove(timerDescPtr);
			freeTimer(timerDescPtr);
		}

		/* ��ǰ��ʱ���Ѿ��������,ִ�в������� */
		oppExecute();
	}

	/* ������һ����ʱ�� */
	setNextTimer();

	_lock->unlock();
	return  true;
}

unsigned int __stdcall TimerQueue::workerProc(void* param)
{
	TimerQueue* instPtr = reinterpret_cast<TimerQueue*>(param);
	HANDLE completeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(!completeEvent)
	{
		//LOGGER_CFATAL(theLogger, _T("�޷������¼�����,������:%d\r\n"), GetLastError());
		return 1;
	}

	int ret = 0;
	while(true)
	{
		if( WAIT_OBJECT_0 == WaitForSingleObject(instPtr->_waitableTimer, INFINITE) )
		{
			if(!instPtr->proc(completeEvent))
			{
				/* �����˳���־ */
				break;
			}
		}
		else
		{
			/*
			* �������� WAIT_FAILED
			*/
			ret = 2;
			break;
		}
	}

	/*
	* �˳�
	*/
	CloseHandle(completeEvent);
	return ret;
}

bool TimerQueue::inqueue(timer_desc_t* newTimerPtr)
{
	/*
	* _timerList ��һ���������(����������Ч�Ķ�ʱ������)
	* ���ڴ������ʱ������,����ӵĶ�ʱ��һ��Ҳ����ʱ,���Դ�ĩβ��ǰ������Ч�ʸ���һЩ.
	* �������,��Ҳûʲô����.
	*/
	bool inserted = false;
	if(!_timerList->empty())
	{
		timer_list_t::iterator iter = _timerList->end();
		do
		{
			--iter;
			timer_desc_t* timerPtr = *iter;
			if(isValidTimer(timerPtr) && newTimerPtr->expireCounters >= timerPtr->expireCounters)
			{
				// std::list insert ��ָ����������**ǰ��**������Ԫ��,������Ҫ�����¶�ʱ��Ӧ����
				// ����,�����Ȱѵ����������ƶ�
				++iter;
				_timerList->insert(iter, newTimerPtr);
				inserted = true;
				break;
			}
		}while(iter != _timerList->begin());
	}

	if(!inserted)
	{
		/* û���ҵ����ʵ�λ������ӵ���ͷ */
		_timerList->push_front(newTimerPtr);
		return true;
	}
	else
	{
		/* insert ������ iter ֮�����,�����¶�ʱ���϶����ڶ�ͷ */
		return false;
	}
}

TimerQueue::timer_t TimerQueue::createTimer(DWORD timeout, timer_func_t callbackFunc, void* param)
{
	
	/*
	* ���´����Ķ�ʱ�����ճ�ʱ��ʱ����뵽���ʵ�λ��,Ҫȷ����һ����ʱ���϶������ȳ�ʱ�Ķ�ʱ��.
	*/
	_lock->lock();
	timer_desc_t* newTimerPtr = allocTimer();
	newTimerPtr->expireCounters = _hrt.now() + _hrt.getCounters(timeout);
	newTimerPtr->st = TIMER_ST_NORMAL;
	newTimerPtr->funcPtr = callbackFunc;
	newTimerPtr->param = param;
	newTimerPtr->waitting = false;

	oppAcquire(newTimerPtr, OPP_ADD);
	_lock->unlock();

	return newTimerPtr;
}

/*
* �޸Ķ�ʱ��,��ʹ��ʱ���Ѿ�����,��Ȼ��Ч.
*/
int TimerQueue::changeTimer(timer_t timerPtr, DWORD timeout)
{
	timer_desc_t* timerDescPtr = reinterpret_cast<timer_desc_t*>(timerPtr);
	_lock->lock();
	timerDescPtr->expireCounters = _hrt.now() + _hrt.getCounters(timeout);
	timerDescPtr->st = TIMER_ST_NORMAL;
	timerDescPtr->waitting = false;

	oppAcquire(timerDescPtr, OPP_CHANGE);
	_lock->unlock();
	return TQ_SUCCESS;
}

int TimerQueue::deleteTimer(timer_t timerPtr, bool wait)
{
	int ret = TQ_SUCCESS;
	timer_desc_t* timerDescPtr = reinterpret_cast<timer_desc_t*>(timerPtr);
	HANDLE ev = NULL;

	_lock->lock();
	if(timerDescPtr->st != TIMER_ST_PROCESSING)
	{
		/* ��ʱ������ */
		oppAcquire(timerDescPtr, OPP_REMOVE);
	}
	else
	{
		/* �����߳�����ִ�лص����� */
		timerDescPtr->waitting = wait;
		timerDescPtr->autoDelete = true;

		/* ����ִ�ж�ʱ���ص����� */
		if(wait)
		{
			/* �ȴ��ص��������� */
			ev = timerDescPtr->ev;
		}
	}
	_lock->unlock();

	/* �ȴ��ص�����ִ�����(��Ϊǰ���������Զ�ɾ����־,�����߳��ڻص�����ִ�����֮���Ѷ�ʱ��ɾ��) */
	if(ev != NULL)
	{
		WaitForSingleObject(ev, INFINITE);
	}
	return ret;
}

void TimerQueue::oppAcquire(timer_desc_t* timerPtr, int oppType)
{
	_oppList->push_back(std::make_pair(timerPtr, oppType));
	
	/*
	* ֻ�ж�ͷ���ƶ������Ҫ���ѹ����߳�,�������ö�ʱ��
	*/
	bool wake = false;
	if(oppType == OPP_ADD)
	{
		// �¶�ʱ���ȵ�ǰ��ʱ�����쳬ʱ,��Ҫ�������ö�ʱ��.
		wake = (_curTimer == NULL || timerPtr->expireCounters < _curTimer->expireCounters);
	}
	else if(oppType == OPP_CHANGE)
	{
		// ��ǰ��ʱ�������Ļ����޸ĺ�Ķ�ʱ���ȵ�ǰ��ʱ�����쳬ʱ.
		wake = (timerPtr == _curTimer || timerPtr->expireCounters < _curTimer->expireCounters);
	}
	else
	{
		// ��ǰ��ʱ����ɾ��.
		wake = (_curTimer == NULL || timerPtr == _curTimer);
	}

	if(wake)
	{
		wakeup();
	}
}

void TimerQueue::wakeup()
{
	if(_wakeupType != WT_RESET)
	{
		_wakeupType = WT_RESET;

		LARGE_INTEGER dueTime;
		dueTime.QuadPart = 0;

		if(!SetWaitableTimer(_waitableTimer, &dueTime, 0, NULL, NULL, FALSE))
		{
			//LOGGER_CFATAL(theLogger, _T("wakeup failed, error code:%d, handle:%x.\r\n"), GetLastError(), _waitableTimer );
			assert(0);
		}
	}
	else
	{
		/*
		* �Ѿ��ֶ����� _waitableTimer, ����Ҫ�ٴδ���
		*/
	}
}