#pragma once
/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

/*
* ������
*/

class Lock
{
private:
	CRITICAL_SECTION _cs;

public:
	Lock(void);
	~Lock(void);
	void lock();
	void unlock();
};

/*
* ��д��
* ������ռ��,������������ȴ�����ʱ,����˼·
* 1. ʹ���Զ��¼�,һ��ֻ�ͷ�һ���������߳�. Ȼ��������ȷ���Ƿ��ٴδ����¼�. �൱�ڴ�������.
* 1.1 �ŵ�: Ч�ʸ���,û���������.
* 1.2 ȱ��: �ٶ�����,Ҫ��������¼�.

* 2. ʹ���ֶ��¼�,�ͷ����е��������߳�,������.
* 2.1 �ŵ�: �ṹ����,�ٶ��Կ�.
* 2.2 ȱ��: ���������,���б��ͷŵ��߳̿���ֻ��һ���õ�����Ȩ,�����߳�תһ��֮��Ҫ��������.
*
* ���Ӧ�ó�����,����������Զ����д��������,��ô�õڶ��ַ�ʽ�ܷ������Ч��.
*
* ����,��unlock()��Ҳ�����ִ���ʽ
* 1. ʹ���Զ��¼�,�����¼���,���ͷŻ�����,�������ͷŵ��Ǹ��߳̾ͽ�ռ�л�����.�����ʱ�ⲿ����һ����,��ô���ò�����������.
* 2. ʹ���Զ��¼�,�����¼���,�ͷŻ�����. ���ͷŵ��Ǹ��߳̽����¾���������������Ȩ.
*/

class RWLock
{
private:
	int _st; /* ��״ֵ̬ */
	int _rlockCount; /* �������� */
	int _rwaitingCount; /* ���ȴ����� */
	int _wwaitingCount; /* д�ȴ����� */
	HANDLE _ev; /* ֪ͨ�¼� Event */
	//HANDLE _stLock; /* ����״ֵ̬������ */ /* �����Ҫ�ȴ���ʱ,���� Mutex */
	CRITICAL_SECTION _stLock;

public:
	RWLock(void);
	~RWLock(void);
	void rlock();
	void wlock();
	void unlock();
};

/*
* �򵥵�ͬ������.
* ԭ�����ж��ٸ�CPU������ֻ��Ҫ���ٸ�ͬ����,�����Ƕ����.
*/

template <typename lock_t>
class LockPool
{
private:
	size_t _size;
	lock_t** _lockList;
	size_t _index;

	LockPool(LockPool&);
	const LockPool& operator = (const LockPool&);

public:
	LockPool();
	~LockPool();

	bool init(size_t sz);
	bool destroy();

	lock_t* allocate();
	void recycle(lock_t* lockPtr);
};

template <typename lock_t>
LockPool<lock_t>::LockPool()
	: _lockList(NULL),
	_size(0),
	_index(0)
{
}

template <typename lock_t>
LockPool<lock_t>::~LockPool()
{
}

template <typename lock_t>
bool LockPool<lock_t>::init(size_t sz)
{
	_size = sz;
	if(_size == 0)
	{
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		_size = sysInfo.dwNumberOfProcessors;
	}
	if(_size <= 0) sz = 2;

	_lockList = new lock_t*[_size];
	for(size_t i = 0; i < _size; ++i)
	{
		_lockList[i] = new lock_t;
	}

	_index = 0;
	return true;
}

template <typename lock_t>
bool LockPool<lock_t>::destroy()
{
	if(_lockList && _size > 0)
	{
		for(size_t i = 0; i < _size; ++i)
		{
			delete _lockList[i];
		}
		delete []_lockList;
	}
	_lockList = NULL;
	_size = 0;
	_index = 0;

	return true;
}

template <typename lock_t>
lock_t* LockPool<lock_t>::allocate()
{
	return _lockList[(_index++) % _size];
}

template <typename lock_t>
void LockPool<lock_t>::recycle(lock_t* lockPtr)
{

}