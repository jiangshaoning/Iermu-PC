#pragma once

#include "MainDlg.h"
#include "json.h"

class WinClientTask : public WorkItemBase
{

public:
	WinClientTask(CMainDlg* pdlg, LPVOID param) :m_dlg(pdlg), m_param(param){}
private:
	void			DoWork(void* pThreadContext);
	void			Abort();
	CMainDlg*		m_dlg;
	void*			m_param;
};

void WinClientTask::DoWork(void* pThreadContext)
{
	SocketData *param = (SocketData *)m_param;
	if (param->opt == OPT_GETCAMERA_LIST)
		m_dlg->TcpRequestTask(m_param);
	else
		m_dlg->HttpRequestTask(m_param);
	delete this;
}

void WinClientTask::Abort()
{
	delete this;
}