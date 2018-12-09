#include "StdAfx.h"
#include <assert.h>
#include "Lock.h"

/*
* ��д��״̬
*/
#define RWLOCK_IDLE 0 /* ���� */
#define RWLOCK_R 0x01 /* ���� */
#define RWLOCK_W 0x02 /* д�� */
//#define RWLOCK_IS_WAITING 0x04 /* �Ƿ������������ڵȴ� */
//#define USE_SYNCLOCK

Lock::Lock(void)
{
	InitializeCriticalSection(&_cs);
}


Lock::~Lock(void)
{
	DeleteCriticalSection(&_cs);
}

void Lock::lock()
{
	EnterCriticalSection(&_cs);
}

void Lock::unlock()
{
	LeaveCriticalSection(&_cs);
}

RWLock::RWLock(void)
	: _rlockCount(0),
	_st(RWLOCK_IDLE),
	_rwaitingCount(0),
	_wwaitingCount(0)
{
	//_stLock = CreateMutex(NULL, FALSE, NULL);
	//assert(_stLock != INVALID_HANDLE_VALUE);
	InitializeCriticalSection(&_stLock);

	/*
	* ���赱ǰ�ж�������������ڵȴ�д���ͷ�,��ô��д�����ͷ�ʱ,������Щ������Ӧ���л�����ִ��.
	*/
	_ev = CreateEvent(NULL, TRUE, FALSE, NULL);
	assert(_ev != INVALID_HANDLE_VALUE);
}


RWLock::~RWLock(void)
{
	//CloseHandle(_stLock);
	DeleteCriticalSection(&_stLock);
	CloseHandle(_ev);
}

#ifdef USE_SYNCLOCK
void RWLock::rlock()
{
	EnterCriticalSection(&_stLock);
}

void RWLock::wlock()
{
	EnterCriticalSection(&_stLock);
}

void RWLock::unlock()
{
	LeaveCriticalSection(&_stLock);
}
#else
void RWLock::rlock()
{
	bool isWaitReturn = false;
	while(1)
	{
		//WaitForSingleObject(_stLock, INFINITE);
		EnterCriticalSection(&_stLock);
		if(isWaitReturn)
		{
			/*
			* �ȴ��¼�����,���¾�����.
			*/
			--_rwaitingCount;
		}

		if(_st == RWLOCK_IDLE)
		{
			/*
			* ����״̬,ֱ�ӵõ�����Ȩ
			*/
			_st = RWLOCK_R;
			_rlockCount++;
			//ReleaseMutex(_stLock);
			LeaveCriticalSection(&_stLock);
			break;
		}
		else if( _st == RWLOCK_R)
		{
			if(_wwaitingCount > 0)
			{
				/*
				* ��д�����ڵȴ�,��һ��ȴ�,��ʹд���ܻ�þ�������.
				*/
				++_rwaitingCount;
				ResetEvent(_ev);
				//SignalObjectAndWait(_stLock, _ev, INFINITE, FALSE);
				LeaveCriticalSection(&_stLock);

				/*
				* ��Ȼ LeaveCriticalSection() �� WaitForSingleObject() ֮����һ��ʱ�䴰��,
				* ��������windowsƽ̨���¼��ź��ǲ��ᶪʧ��,����û������.
				*/
				WaitForSingleObject(_ev, INFINITE);

				/*
				* �ȴ�����,�������Լ���.
				*/
				isWaitReturn = true;
			}
			else
			{	
				/*
				* �õ�����,����+1
				*/
				++_rlockCount;
				//ReleaseMutex(_stLock);
				LeaveCriticalSection(&_stLock);
				break;
			}
		}
		else if(_st == RWLOCK_W)
		{
			/*
			* �ȴ�д���ͷ�
			*/
			++_rwaitingCount;
			ResetEvent(_ev);
			//SignalObjectAndWait(_stLock, _ev, INFINITE, FALSE);
			LeaveCriticalSection(&_stLock);
			WaitForSingleObject(_ev, INFINITE);

			/*
			* �ȴ�����,�������Լ���.
			*/
			isWaitReturn = true;
		}
		else
		{
			assert(0);
			break;
		}
	}
}

void RWLock::wlock()
{
	bool isWaitReturn = false;

	while(1)
	{
		//WaitForSingleObject(_stLock, INFINITE);
		EnterCriticalSection(&_stLock);

		if(isWaitReturn) --_wwaitingCount;

		if(_st == RWLOCK_IDLE)
		{
			_st = RWLOCK_W;
			//ReleaseMutex(_stLock);
			LeaveCriticalSection(&_stLock);
			break;
		}
		else
		{
			++_wwaitingCount;
			ResetEvent(_ev);
			//SignalObjectAndWait(_stLock, _ev, INFINITE, FALSE);
			LeaveCriticalSection(&_stLock);
			WaitForSingleObject(_ev, INFINITE);

			isWaitReturn = true;
		}
	}
}

void RWLock::unlock()
{
	//WaitForSingleObject(_stLock, INFINITE);
	EnterCriticalSection(&_stLock);
	if(_rlockCount > 0)
	{
		/* �������� */
		--_rlockCount;

		if( 0 == _rlockCount)
		{
			_st = RWLOCK_IDLE;

			/* �ͷ� */
			if( _wwaitingCount > 0 || _rwaitingCount > 0 )
			{
				/* 
				* ��ʱ�����������ڵȴ�,�������еȴ����߳�.(�ֶ��¼�).
				* ʹ��Щ�������¾�����.
				*/
				SetEvent(_ev);
			}
			else
			{
				/* ���� */
			}
		}
		else
		{
			/* ���ж��� */
		}
	}
	else
	{
		_st = RWLOCK_IDLE;

		/* д������ */
		if( _wwaitingCount > 0 || _rwaitingCount > 0 )
		{
			/* 
			* �����ռ�л�����_stLock�������,�����¼�,��ô���ܻ�ʹһЩ�������ܵõ���������.
			* �������unlockʱ,��һ���߳����õ���rlock����wlock.������ͷŻ�����,ֻ��֮ǰ�Ѿ��ȴ����������л�����������Ȩ.
			*/
			SetEvent(_ev);
		}
		else
		{
			/* ���� */
		}
	}
	//ReleaseMutex(_stLock);
	LeaveCriticalSection(&_stLock);
}
#endif
/*
* ��һ��ʵ��,�����ҵ�.
* ��ȫ�����Ķ�д��, class RWLock ��д���ȵĶ�д��.
*/
/****************************************************
 *  @file    $URL$
 *
 *  read-write lock c file
 *
 *  $Id$
 *
 *  @author gang chen <eyesfour@gmail.com>
 ***************************************************/

//
//#include <stdio.h>
//#include <stdlib.h>
//#define   WIN32_LEAN_AND_MEAN   
//#define _WIN32_WINNT 0x0400
//#include <Windows.h>
//#include "../../include/cg_rwlock.h"
//
//
//TRWLock  *RWLockCreate(void)
//{
//	TRWLock *hRWLock = (TRWLock*)malloc(sizeof(TRWLock));
//	if (hRWLock == NULL) return NULL;
//	// �������ڱ����ڲ����ݵĻ�����
//	hRWLock->hMutex = CreateMutex(NULL, FALSE, NULL);
//    // ��������ͬ����������̵߳��¼����ֶ��¼���
//	hRWLock->hReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//    // ��������ͬ����ռ�����̵߳��¼����Զ��¼���
//	hRWLock->hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//	hRWLock->nSharedNum = 0;
//	hRWLock->nExclusiveNum = 0;
//	hRWLock->nLockType = LOCK_NONE;
//	return hRWLock;
//}
//
//void RWLockDestroy(TRWLock *hRWLock)
//{
//	CloseHandle(hRWLock->hMutex);
//	CloseHandle(hRWLock->hReadEvent);
//	CloseHandle(hRWLock->hWriteEvent);
//	free(hRWLock);
//}
//
//bool EnterReadLock(TRWLock *hRWLock, DWORD waitTime)
//{
//	WaitForSingleObject(hRWLock->hMutex, INFINITE);
//	++hRWLock->nSharedNum;
//	if (hRWLock->nLockType == LOCK_EXCLUSIVE) 
//	{
//		DWORD retCode = SignalObjectAndWait(hRWLock->hMutex, hRWLock->hReadEvent, /*waitTime*/INFINITE, FALSE);
//        if (retCode == WAIT_OBJECT_0) {
//            return true;
//        } else {
//            if (retCode == WAIT_TIMEOUT)
//                SetLastError(WAIT_TIMEOUT);
//            return false;
//        }
//    }
//	hRWLock->nLockType = LOCK_SHARED;
//	ReleaseMutex(hRWLock->hMutex);
//	return true;
//}
//
//
//void LeaveReadLock(TRWLock *hRWLock)
//{
//	//assert(hRWLock->nLockType == LOCK_SHARED);
//    WaitForSingleObject(hRWLock->hMutex, INFINITE);
//    --hRWLock->nSharedNum;
//    if (hRWLock->nSharedNum == 0) 
//	{
//		if (hRWLock->nExclusiveNum > 0) 
//		{
//            // ����һ����ռ�����߳�
//            hRWLock->nLockType = LOCK_EXCLUSIVE;
//			SetEvent(hRWLock->hWriteEvent);
//        } 
//		else 
//		{
//            // û�еȴ��߳�
//            hRWLock->nLockType = LOCK_NONE;
//        }
//    }
//    ReleaseMutex(hRWLock->hMutex);
//}
//
//
//bool EnterWriteLock(TRWLock *hRWLock)
//{
//	WaitForSingleObject(hRWLock->hMutex, INFINITE);
//    ++hRWLock->nExclusiveNum;
//    if (hRWLock->nLockType != LOCK_NONE) 
//	{
//		DWORD retCode = SignalObjectAndWait(hRWLock->hMutex, hRWLock->hWriteEvent, /*waitTime*/INFINITE, FALSE);
//        if (retCode == WAIT_OBJECT_0) 
//		{
//            return true;
//        } 
//		else 
//		{
//            if (retCode == WAIT_TIMEOUT)
//                SetLastError(WAIT_TIMEOUT);
//            return false;
//        }
//    }
//    hRWLock->nLockType = LOCK_EXCLUSIVE;
//    ReleaseMutex(hRWLock->hMutex);
//    return true;
//}
//
//void LeaveWriteLock(TRWLock *hRWLock)
//{
//    //assert(hRWLock->nLockType == LOCK_EXCLUSIVE);
//    WaitForSingleObject(hRWLock->hMutex, INFINITE);
//    --hRWLock->nExclusiveNum;
//    // ��ռ�����߳�����
//    if (hRWLock->nExclusiveNum > 0) 
//	{
//        // ����һ����ռ�����߳�
//        SetEvent(hRWLock->hWriteEvent);
//    } 
//	else if (hRWLock->nSharedNum > 0) 
//	{
//        // ���ѵ�ǰ���й�������߳�
//        hRWLock->nLockType = LOCK_SHARED;
//        PulseEvent(hRWLock->hReadEvent);
//    } 
//	else 
//	{
//        // û�еȴ��߳�
//        hRWLock->nLockType = LOCK_NONE;
//    }
//    ReleaseMutex(hRWLock->hMutex);
//} ...