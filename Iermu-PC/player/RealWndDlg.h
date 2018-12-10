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
	void CameraOnBtnLogin();							//普通账号密码登录
	void CameraOnSwitchLoginTip();						//点击登录切换到登录tip 并获取二维码
	void CameraOnSwitchLoginPasswd();					//点击切换到密码登录tip
	void CameraOnSwitchLoginMessage();					//点击切换到短信登录tip
	void CameraOnSwitchLoginEnterprise();				//点击切换到企业登录tip
	void CameraOnCLoseLoginTip();						//点击关闭登录tip
	void CameraOnSwitchQrcodeTip();						//点击切换到扫二维码tip
	void CameraOnRefQrcode();							//点击刷新二维码
	void CameraOnSwitchNorPlayer();						//摄像机切换到普通播放主页面
	void CameraOnSwitchAIPlayer();						//摄像机切换到AI播放主页面
	void saveUserInfo();								//保存登录用户名密码token信息
	void loadUserInfo();								//读取登录用户名密码token信息
	BOOL IsFileExist(const SStringT& csFile);
	void GetCurTimeName(char* Ctime, wchar_t* Wtime, char* name, char* postfix);
	void PlayerShowText(int time);
	void OnSnapshot();
	void OnRecord();
	void OnVolume();
	void OnVolumeZero();
	void OnScreenFull();
	void FullScreen(BOOL bFull);

	//接受键盘输入
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLbuttonDBLCLK(UINT nFlags, CPoint pt);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	LRESULT OnInitRealWnd(UINT uMsg, WPARAM wParam, LPARAM lParam);				//初始化界面和播放器
	LRESULT OnCameraTipPage(UINT uMsg, WPARAM wParam, LPARAM lParam);			//进入第一个界面
	LRESULT OnOpenLogin(UINT uMsg, WPARAM wParam, LPARAM lParam);				//打开登录界面
	LRESULT openVideo(UINT uMsg, WPARAM wParam, LPARAM lParam);		
	LRESULT closeVideo(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT playVideo(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_HTTP_TASK(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled);
protected:
	//soui消息
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
    //消息映射表
    BEGIN_MSG_MAP_EX(CRealWndDlg)
		MSG_WM_TIMER(OnTimer)
		//处理键盘按键消息
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
		MESSAGE_HANDLER_EX(MSG_FANPLAYER, playVideo)				//播放器过来的渲染消息
		MESSAGE_HANDLER(WM_WINREQUEST_TASK, OnMsg_HTTP_TASK)
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
private:
	//LoginInfo		m_loginInfo;		// 登录的相关信息
	//string		m_code;				// 请求二维码的code值
	SMenuEx			m_menu_open;
	CMainDlg*		m_main_dlg;
	int				m_LButtonDown;
	BOOL			m_ctrl_down;
	BOOL			m_bIsRecording;
	BOOL            m_bFullScreenMode;  // 是否在全屏模式
	SSliderBar*		m_VolumeSlider;
	WINDOWPLACEMENT		OldWndPlacement;  // 保存窗口原来的位置
	TCHAR			m_strTxt[MAX_PATH];
};
