#pragma once

class CThreadObject
{
public:
	CThreadObject();
	~CThreadObject();

	BOOL BeginThread(LPVOID data);

	BOOL EndThread(DWORD dwWaitEndMS = INFINITE);

	BOOL IsRunning() const;

	BOOL IsStoped() const;
protected:
	//Work Function.
	virtual UINT Run(LPVOID data) = 0;

	UINT ThreadProc(LPVOID data);
	static UINT __stdcall StaticTheadProc(LPVOID param);

	HANDLE m_evtStart;	//�����߳�֪ͨ�����̡߳��߳��������¼�
	HANDLE m_evtStop;   //�����߳�֪ͨ�����̡߳��߳̽������¼�
	HANDLE m_hThread;   //�߳̾��

	LPVOID data;
};

