#pragma once
#include "MainDlg.h"

class AddPopupWnd :public SHostWnd
{
public:
	AddPopupWnd(LPCTSTR pszResName = NULL, HWND hwnd = NULL) :SHostWnd(pszResName), m_dlgWnd(hwnd){}

	void OnClose()
	{
		SetVisible(FALSE, TRUE);
	}


	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"add_close", OnClose)
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

