#pragma once
#include "../MainDlg.h"
#include "ffplayer.h"
class CRealWndDlg_File : public SHostDialog
{
public:
    CRealWndDlg_File();
    ~CRealWndDlg_File(void);
private:
	void OnTimer(UINT_PTR nIDEvent);
	BOOL IsFileExist(const SStringT& csFile);
	void GetCurTimeName(char* Ctime, wchar_t* Wtime, char* name, char* postfix);
	void PlayerShowText(int time);
	void OnSnapshot();
	void OnRecord();
	void OnVolume();
	void OnVolumeZero();
	void OnScreenFull();
	void FullScreen(BOOL bFull);
	void fileOnSwitchToPlayer();

	//���ܼ�������
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLbuttonDBLCLK(UINT nFlags, CPoint pt);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	LRESULT OnInitRealWnd(UINT uMsg, WPARAM wParam, LPARAM lParam);				//��ʼ������Ͳ�����
	LRESULT playVideo(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnMsg_TCP_TASK(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled);
protected:
	//soui��Ϣ
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_snapshot", OnSnapshot)
		EVENT_NAME_COMMAND(L"btn_record", OnRecord)
		EVENT_NAME_COMMAND(L"btn_record_stop", OnRecord)
		EVENT_NAME_COMMAND(L"btn_volume", OnVolume)
		EVENT_NAME_COMMAND(L"btn_volume_zero", OnVolumeZero)
		EVENT_NAME_COMMAND(L"btn_screen_full", OnScreenFull)
		EVENT_NAME_COMMAND(L"add_link", fileOnSwitchToPlayer)
	EVENT_MAP_END()
    //��Ϣӳ���
    BEGIN_MSG_MAP_EX(CRealWndDlg_File)
		MSG_WM_TIMER(OnTimer)
		//������̰�����Ϣ
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_LBUTTONDBLCLK(OnLbuttonDBLCLK)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		//MESSAGE_HANDLER(WM_TCPREQUEST_TASK, OnMsg_TCP_TASK)
		MESSAGE_HANDLER_EX(MS_INIT_REALWND, OnInitRealWnd)
		MESSAGE_HANDLER_EX(MSG_FANPLAYER, playVideo)		//��������������Ⱦ��Ϣ
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
private:

	SMenuEx			m_menu_open;
	CMainDlg*		m_main_dlg;
	int				m_LButtonDown;
	BOOL			m_ctrl_down;
	BOOL			m_bIsRecording;
	BOOL            m_bFullScreenMode;		// �Ƿ���ȫ��ģʽ
	SSliderBar*		m_VolumeSlider;
	TCHAR			m_strTxt[MAX_PATH];
	WINDOWPLACEMENT		OldWndPlacement;	// ���洰��ԭ����λ��
};
