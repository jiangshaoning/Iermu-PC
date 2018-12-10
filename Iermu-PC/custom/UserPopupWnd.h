#pragma once
#include "MainDlg.h"

class UserPopupWnd :public SHostWnd
{
public:
	UserPopupWnd(LPCTSTR pszResName = NULL, HWND hwnd = NULL) :SHostWnd(pszResName), m_dlgWnd(hwnd){}

	void OnClose()
	{
		SetVisible(FALSE, TRUE);
	}

	void OnQuitLogin()
	{
		::SendMessage(m_dlgWnd, WM_QUIT_LOGIN, 0, 0);
		OnClose();
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
		//��ʾOnFinalMessage�÷�,����new�����Ĳ���Ҫ��ʾ����delete
		__super::OnFinalMessage(hWnd);
		delete this;
	}
private:
	HWND m_dlgWnd;
};

