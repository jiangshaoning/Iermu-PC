#pragma once

class CMainDlg;

class AddPopupWnd :public SHostWnd
{
public:
	AddPopupWnd(LPCTSTR pszResName = NULL, CMainDlg* dlg = NULL) :SHostWnd(pszResName), m_dlg(dlg){}

	void OnClose();
	void FirstStep();
	void SecondStep();
	void ThirdStep();
	LRESULT OnMsg_HTTP_TASK(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled);

	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"add_close", OnClose)
		EVENT_NAME_COMMAND(L"first_step", FirstStep)
		EVENT_NAME_COMMAND(L"second_step", SecondStep)
		EVENT_NAME_COMMAND(L"third_step", ThirdStep)
		EVENT_MAP_END()
	BEGIN_MSG_MAP_EX(SSkiaTestWnd)
	MESSAGE_HANDLER(WM_ADD_FILESED, OnMsg_HTTP_TASK)
		CHAIN_MSG_MAP(__super)
		END_MSG_MAP()

protected:
	void OnFinalMessage(HWND hWnd){
		//演示OnFinalMessage用法,下面new出来的不需要显示调用delete
		__super::OnFinalMessage(hWnd);
		delete this;
	}
private:
	CMainDlg*	m_dlg;
	string		m_deviceId;
};

