#pragma once
#include "../MainDlg.h"
#include "ffplayer.h"
class CRealWndDlg : public SHostDialog
{
public:
    CRealWndDlg();
    ~CRealWndDlg(void);
private:
	void OnTimer(UINT_PTR nIDEvent);
	void CameraOnBtnLogin();							//��ͨ�˺������¼
	void CameraOnSwitchLoginTip();						//�����¼�л�����¼tip ����ȡ��ά��
	void CameraOnSwitchLoginPasswd();					//����л��������¼tip
	void CameraOnSwitchLoginMessage();					//����л������ŵ�¼tip
	void CameraOnSwitchLoginEnterprise();				//����л�����ҵ��¼tip
	void CameraOnCLoseLoginTip();						//����رյ�¼tip
	void CameraOnSwitchQrcodeTip();						//����л���ɨ��ά��tip
	void CameraOnRefQrcode();							//���ˢ�¶�ά��
	void CameraOnSwitchNorPlayer();						//������л�����ͨ������ҳ��
	void CameraOnSwitchAIPlayer();						//������л���AI������ҳ��
	void saveUserInfo();								//�����¼�û�������token��Ϣ
	void loadUserInfo();								//��ȡ��¼�û�������token��Ϣ
	BOOL IsFileExist(const SStringT& csFile);
	void GetCurTimeName(char* Ctime, wchar_t* Wtime, char* name, char* postfix);
	void PlayerShowText(int time);
	void OnSnapshot();
	void OnRecord();
	void OnVolume();
	void OnVolumeZero();
	void OnScreenFull();
	void FullScreen(BOOL bFull);

	//���ܼ�������
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLbuttonDBLCLK(UINT nFlags, CPoint pt);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	LRESULT OnInitRealWnd(UINT uMsg, WPARAM wParam, LPARAM lParam);				//��ʼ������Ͳ�����
	LRESULT OnCameraTipPage(UINT uMsg, WPARAM wParam, LPARAM lParam);			//�����һ������
	LRESULT OnOpenLogin(UINT uMsg, WPARAM wParam, LPARAM lParam);				//�򿪵�¼����
	LRESULT openVideo(UINT uMsg, WPARAM wParam, LPARAM lParam);		
	LRESULT closeVideo(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT playVideo(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_HTTP_TASK(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled);
protected:
	//soui��Ϣ
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"login_link", CameraOnSwitchLoginTip)
		EVENT_NAME_COMMAND(L"login_passwd", CameraOnSwitchLoginPasswd)
		EVENT_NAME_COMMAND(L"login_message", CameraOnSwitchLoginMessage)
		EVENT_NAME_COMMAND(L"login_enterprise", CameraOnSwitchLoginEnterprise)
		EVENT_NAME_COMMAND(L"login_close", CameraOnCLoseLoginTip)
		EVENT_NAME_COMMAND(L"sweep_qrcode", CameraOnSwitchQrcodeTip)
		EVENT_NAME_COMMAND(L"btn_qrcode", CameraOnRefQrcode)
		EVENT_NAME_COMMAND(L"btn_login", CameraOnBtnLogin)
		EVENT_NAME_COMMAND(L"btn_snapshot", OnSnapshot)
		EVENT_NAME_COMMAND(L"btn_record", OnRecord)
		EVENT_NAME_COMMAND(L"btn_record_stop", OnRecord)
		EVENT_NAME_COMMAND(L"btn_volume", OnVolume)
		EVENT_NAME_COMMAND(L"btn_volume_zero", OnVolumeZero)
		EVENT_NAME_COMMAND(L"btn_screen_full", OnScreenFull)
	EVENT_MAP_END()
    //��Ϣӳ���
    BEGIN_MSG_MAP_EX(CRealWndDlg)
		MSG_WM_TIMER(OnTimer)
		//������̰�����Ϣ
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_LBUTTONDBLCLK(OnLbuttonDBLCLK)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MESSAGE_HANDLER_EX(MS_INIT_REALWND, OnInitRealWnd)
		MESSAGE_HANDLER_EX(MS_OPEN_TIPPAGE, OnCameraTipPage)
		MESSAGE_HANDLER_EX(MS_OPEN_LOGIN, OnOpenLogin)
		MESSAGE_HANDLER_EX(MS_OPENVIDEO_REALWND, openVideo)
		MESSAGE_HANDLER_EX(MS_CLOSEVIDEO_REALWND, closeVideo)
		MESSAGE_HANDLER_EX(MSG_FANPLAYER, playVideo)				//��������������Ⱦ��Ϣ
		MESSAGE_HANDLER(WM_WINREQUEST_TASK, OnMsg_HTTP_TASK)
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
private:
	//LoginInfo		m_loginInfo;		// ��¼�������Ϣ
	//string		m_code;				// �����ά���codeֵ
	SMenuEx			m_menu_open;
	CMainDlg*		m_main_dlg;
	int				m_LButtonDown;
	BOOL			m_ctrl_down;
	BOOL			m_bIsRecording;
	BOOL            m_bFullScreenMode;  // �Ƿ���ȫ��ģʽ
	SSliderBar*		m_VolumeSlider;
	WINDOWPLACEMENT		OldWndPlacement;  // ���洰��ԭ����λ��
	TCHAR			m_strTxt[MAX_PATH];
};
