#pragma once
#include "MainDlg.h"

class SSetPopupWnd :public SHostWnd
{
public:
	SSetPopupWnd(LPCTSTR pszResName = NULL, HWND hwnd = NULL) :SHostWnd(pszResName), m_dlgWnd(hwnd){}

	void OnClose()
	{
		SetVisible(FALSE, TRUE);
	}

	void OnQuitLogin()
	{
		::PostMessage(m_dlgWnd, WM_QUIT_LOGIN, 0, 0);
	}

	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"user_close", OnClose)
		EVENT_NAME_COMMAND(L"quit_login", OnQuitLogin)
		EVENT_MAP_END()
	BEGIN_MSG_MAP_EX(SSkiaTestWnd)
		CHAIN_MSG_MAP(__super)
		END_MSG_MAP()

protected:
	void OnFinalMessage(HWND hWnd){
		//演示OnFinalMessage用法,下面new出来的不需要显示调用delete
		__super::OnFinalMessage(hWnd);
		delete this;
	}
private:
	HWND m_dlgWnd;
};

