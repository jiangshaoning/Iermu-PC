#include "stdafx.h"
#include "RealWndDlg.h"
#include "json.h"
#include "CppSQLite3.h"
#include "SerializeObj.h"

CRealWndDlg::CRealWndDlg():SHostDialog(_T("LAYOUT:XML_REALWND"))								
{
	//m_menu_open.LoadMenu(_T("LAYOUT:menu_open"));
	m_main_dlg = NULL;
	m_ctrl_down = FALSE;
	m_bIsRecording = FALSE;
	m_bFullScreenMode = FALSE;
	m_LButtonDown = 0;
}

CRealWndDlg::~CRealWndDlg(void)
{
	RELEASEPLAYER(m_main_dlg->m_hplayer[0]);
}

LRESULT CRealWndDlg::OnInitRealWnd(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_main_dlg = (CMainDlg*)lParam;
	m_VolumeSlider = FindChildByName2<SSliderBar>(L"volumeSlider");
	if (m_VolumeSlider)
	{
		int volume = m_VolumeSlider->GetValue();
		player_setparam(m_main_dlg->m_hplayer[0], PARAM_AUDIO_VOLUME, (void*)&volume);
	}
	loadUserInfo();
	return 0;
}

BOOL CRealWndDlg::IsFileExist(const SStringT& csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

void CRealWndDlg::OnTimer(UINT_PTR nIDEvent)
{
	int64_t pos = 0;
	int64_t total = 1;
	int value = 0;
	SetMsgHandled(FALSE);
	switch (nIDEvent)
	{
	case TIMER_ID_QRCODE_STATUS:
		if (m_main_dlg->m_code.size())
		{
			string data = QRCODESTATUS_URL;
			data.append("code=").append(m_main_dlg->m_code);
			m_main_dlg->SendCMD(OPT_QRCODE_STATUS, m_hWnd, GET, data, "");
		}
		break;
	case TIMER_ID_HIDE_TEXT:
		KillTimer(TIMER_ID_HIDE_TEXT);
		player_textout(m_main_dlg->m_hplayer[0], 0, 0, 0, NULL);
		m_strTxt[0] = '\0';
		break;
	}
}

LRESULT CRealWndDlg::OnOpenLogin(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CameraOnSwitchLoginTip();
	return 0;
}

LRESULT CRealWndDlg::OnCameraTipPage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_main_dlg->m_loginInfo.user = "";
	m_main_dlg->m_loginInfo.passwd = "";
	saveUserInfo();
	CameraOnCLoseLoginTip();
	return 0;
}

//普通账号密码登录
void CRealWndDlg::CameraOnBtnLogin()
{
	SStringT  userName = FindChildByName2<SEdit>(L"edit_username")->GetWindowTextW();
	SStringT  userpasswd = FindChildByName2<SEdit>(L"edit_passwd")->GetWindowTextW();

	if (!userName.GetLength())
	{
		SMessageBox(NULL, _T("请输入邮箱或手机号！"), _T("提示"), MB_OK | MB_ICONERROR);
		return;
	}
	if (!userpasswd.GetLength())
	{
		SMessageBox(NULL, _T("请输入密码！"), _T("提示"), MB_OK | MB_ICONERROR);
		return;
	}

	m_main_dlg->m_loginInfo.user = S_CT2A(userName);
	m_main_dlg->m_loginInfo.passwd = S_CT2A(userpasswd);

	string url = AUTHORIZATION_URL;
	string data = "client_id=Ba3OkUvTJBzzDhc8yJlW&grant_type=password&lang=zh-Hans";
	data.append("&mobile=").append(m_main_dlg->m_loginInfo.user).append("&password=").append(m_main_dlg->m_loginInfo.passwd);

	m_main_dlg->SendCMD(OPT_PASSWD_LOGIN, m_main_dlg->m_playHwnd[0], POST, AUTHORIZATION_URL, data);
}

//点击关闭登录tip
void CRealWndDlg::CameraOnCLoseLoginTip()
{
	STabCtrl *pTab = FindChildByName2<STabCtrl>(L"camera_tab");
	if (pTab)
	{
		pTab->SetCurSel(_T("camera_tip_page"));
	}
}

//点击登录切换到登录tip 并获取二维码
void CRealWndDlg::CameraOnSwitchLoginTip()
{
	STabCtrl *pTab = FindChildByName2<STabCtrl>(L"camera_tab");
	if (pTab)
	{
		pTab->SetCurSel(_T("login_page"));
		string data = QRCODEIMAGE_URL;
		data.append("response_type=qrcode&client_id=Ba3OkUvTJBzzDhc8yJlW&hardware=1&scope=basic");
		m_main_dlg->SendCMD(OPT_GET_QRCODE, m_main_dlg->m_playHwnd[0], GET, data, "");
	}
}

//点击切换到密码登录tip
void CRealWndDlg::CameraOnSwitchLoginPasswd()
{
	STabCtrl *pTab = FindChildByName2<STabCtrl>(L"camera_tab");
	if (pTab)
	{
		pTab->SetCurSel(_T("login_page"));
		KillTimer(TIMER_ID_QRCODE_STATUS);
	}
}

//点击切换到短信登录tip
void CRealWndDlg::CameraOnSwitchLoginMessage()
{
	STabCtrl *pTab = FindChildByName2<STabCtrl>(L"camera_tab");
	if (pTab)
	{
		pTab->SetCurSel(_T("message_page"));
	}
}

//点击切换到企业登录tip
void CRealWndDlg::CameraOnSwitchLoginEnterprise()
{
	STabCtrl *pTab = FindChildByName2<STabCtrl>(L"camera_tab");
	if (pTab)
	{
		pTab->SetCurSel(_T("enterprise_page"));
	}
}

//摄像机切换到普通播放主页面
void CRealWndDlg::CameraOnSwitchNorPlayer()
{
	STabCtrl *pTab = FindChildByName2<STabCtrl>(L"camera_tab");
	if (pTab)
	{
		pTab->SetCurSel(_T("nor_player_page"));
		m_main_dlg->m_isLogin = TRUE;
	}
}

//摄像机切换到AI播放主页面
void CRealWndDlg::CameraOnSwitchAIPlayer()
{
	STabCtrl *pTab = FindChildByName2<STabCtrl>(L"camera_tab");
	if (pTab)
	{
		pTab->SetCurSel(_T("ai_player_page"));
	}
}

//点击切换到扫二维码tip
void CRealWndDlg::CameraOnSwitchQrcodeTip()
{
	STabCtrl *pTab = FindChildByName2<STabCtrl>(L"camera_tab");
	if (pTab)
	{
		pTab->SetCurSel(_T("qrcode_page"));
		SetTimer(TIMER_ID_QRCODE_STATUS, 2000);
	}
}

//点击刷新二维码
void CRealWndDlg::CameraOnRefQrcode()
{
	string data = QRCODEIMAGE_URL;
	data.append("response_type=qrcode&client_id=Ba3OkUvTJBzzDhc8yJlW&hardware=1&scope=basic");
	m_main_dlg->SendCMD(OPT_GET_QRCODE, m_main_dlg->m_playHwnd[0], GET, data, "");
	m_main_dlg->m_code = "";
	SetTimer(TIMER_ID_QRCODE_STATUS, 2000);
	FindChildByName2<SImageWnd>(L"img_qrcode")->SetAttribute(L"alpha", L"255", FALSE);
	FindChildByName2<SWindow>(L"ref_qrcode")->SetVisible(FALSE);
}

//保存登录用户名密码token信息
void CRealWndDlg::saveUserInfo()
{
	string  user = "", passwd = "", token = "", refToken = "", uid = "";

	SCheckBox *check_passwd = FindChildByName2<SCheckBox>(L"check_passwd");
	SCheckBox *check_auto_login = FindChildByName2<SCheckBox>(L"check_auto_login");
	int type = ((check_auto_login->IsChecked() & 1) << 1) | (check_passwd->IsChecked() & 1);

	switch (type)
	{
	case 1:
		user = m_main_dlg->m_loginInfo.user;
		passwd = m_main_dlg->m_loginInfo.passwd;
		break;
	case 2:
		token = m_main_dlg->m_loginInfo.token;
		refToken = m_main_dlg->m_loginInfo.refToken;
		uid = m_main_dlg->m_loginInfo.uid;
		break;
	case 3:
		user = m_main_dlg->m_loginInfo.user;
		passwd = m_main_dlg->m_loginInfo.passwd;
		token = m_main_dlg->m_loginInfo.token;
		refToken = m_main_dlg->m_loginInfo.refToken;
		uid = m_main_dlg->m_loginInfo.uid;
		break;
	}

	CSerializeData lg(type, user, passwd, token, refToken, uid);
	ofstream fout(LOGIN_DAT, ios::binary);
	lg.save(fout);
	fout.close();
}

//读取登录用户名密码token信息
void CRealWndDlg::loadUserInfo()
{
	CSerializeData  lg;
	ifstream fin(LOGIN_DAT, ios::binary);
	if (fin)
	{
		lg.load(fin);
		m_main_dlg->m_loginInfo.type = lg.getType();
		switch (m_main_dlg->m_loginInfo.type)
		{
		case 0:
			break;
		case 1:
			m_main_dlg->m_loginInfo.user = lg.getLoginUser();
			m_main_dlg->m_loginInfo.passwd = lg.getLoginPasswd();
			FindChildByName2<SEdit>(L"edit_username")->SetWindowTextW(S_CA2T(m_main_dlg->m_loginInfo.user.c_str()));
			FindChildByName2<SEdit>(L"edit_passwd")->SetWindowTextW(S_CA2T(m_main_dlg->m_loginInfo.passwd.c_str()));
			FindChildByName2<SCheckBox>(L"check_passwd")->SetCheck(TRUE);
			break;
		case 2:
			m_main_dlg->m_loginInfo.user = lg.getLoginUser();
			m_main_dlg->m_loginInfo.passwd = lg.getLoginPasswd();
			m_main_dlg->m_loginInfo.token = lg.getToken();
			m_main_dlg->m_loginInfo.refToken = lg.getRefreshToken();
			m_main_dlg->m_loginInfo.uid = lg.getUid();
			FindChildByName2<SCheckBox>(L"check_auto_login")->SetCheck(TRUE);
			if (m_main_dlg->m_loginInfo.user.size() > 0 && m_main_dlg->m_loginInfo.passwd.size() > 0)
			{
				m_main_dlg->LoadAvatar();
				m_main_dlg->GetUserInfoRequest();
				m_main_dlg->loadDeviceInfo();
				m_main_dlg->GetCameraList();
				CameraOnSwitchNorPlayer();
			}
			break;
		case 3:
			m_main_dlg->m_loginInfo.user = lg.getLoginUser();
			m_main_dlg->m_loginInfo.passwd = lg.getLoginPasswd();
			m_main_dlg->m_loginInfo.token = lg.getToken();
			m_main_dlg->m_loginInfo.refToken = lg.getRefreshToken();
			m_main_dlg->m_loginInfo.uid = lg.getUid();
			FindChildByName2<SEdit>(L"edit_username")->SetWindowTextW(S_CA2T(m_main_dlg->m_loginInfo.user.c_str()));
			FindChildByName2<SEdit>(L"edit_passwd")->SetWindowTextW(S_CA2T(m_main_dlg->m_loginInfo.passwd.c_str()));
			FindChildByName2<SCheckBox>(L"check_passwd")->SetCheck(TRUE);
			FindChildByName2<SCheckBox>(L"check_auto_login")->SetCheck(TRUE);
			if (m_main_dlg->m_loginInfo.user.size() > 0 && m_main_dlg->m_loginInfo.passwd.size() > 0)
			{
				m_main_dlg->LoadAvatar();
				m_main_dlg->GetUserInfoRequest();
				m_main_dlg->loadDeviceInfo();
				m_main_dlg->GetCameraList();
				CameraOnSwitchNorPlayer();
			}
			break;
		default:
			m_main_dlg->m_loginInfo.type = 0;
		}
	}
}

void CRealWndDlg::GetCurTimeName(char* Ctime, wchar_t* Wtime, char* name, char* postfix)
{
	time_t curtime = time(NULL);
	tm *ptm = localtime(&curtime);

	sprintf(Ctime, "%s_%d%02d%02d%02d%02d%02d.%s", name, ptm->tm_year + 1900, ptm->tm_mon + 1,
		ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, postfix);

	MultiByteToWideChar(CP_ACP, 0, Ctime, -1, Wtime, 40);

}

void CRealWndDlg::PlayerShowText(int time)
{
	player_textout(m_main_dlg->m_hplayer[0], 20, 20, RGB(0, 255, 0), m_strTxt);
	SetTimer(TIMER_ID_HIDE_TEXT, time);
}

void CRealWndDlg::OnSnapshot()
{
	if (!m_main_dlg->m_hplayer[0]) return;

	char ctime[40] = { 0 };
	wchar_t wtime[48] = { 0 };
	GetCurTimeName(ctime, wtime, "snapshot", "jpg");
	player_snapshot(m_main_dlg->m_hplayer[0], ctime, 0, 0, 1000);
	_stprintf(m_strTxt, _T("抓拍到当前路径：%s"), wtime);
	PlayerShowText(3000);
}

void CRealWndDlg::OnRecord()
{
	if (!m_main_dlg->m_hplayer[0]) return;

	char ctime[40] = { 0 };
	wchar_t wtime[48] = { 0 };
	GetCurTimeName(ctime, wtime, "record", "mp4");
	player_record(m_main_dlg->m_hplayer[0], m_bIsRecording ? NULL : ctime);
	m_bIsRecording = !m_bIsRecording;
	if (m_bIsRecording)
	{
		FindChildByName2<SWindow>(L"btn_record")->SetVisible(FALSE, TRUE);
		FindChildByName2<SWindow>(L"btn_record_stop")->SetVisible(TRUE, TRUE);
	}
	else
	{
		FindChildByName2<SWindow>(L"btn_record")->SetVisible(TRUE, TRUE);
		FindChildByName2<SWindow>(L"btn_record_stop")->SetVisible(FALSE, TRUE);
	}

	_stprintf(m_strTxt, _T("%s: %s"), m_bIsRecording ? _T("开始录屏") : _T("结束录屏 保存到当前路径"), wtime);
	PlayerShowText(3000);
}

void CRealWndDlg::OnVolume()
{
	int volume = -182;
	FindChildByName2<SWindow>(L"btn_volume")->SetVisible(FALSE, TRUE);
	FindChildByName2<SWindow>(L"btn_volume_zero")->SetVisible(TRUE, TRUE);
	player_setparam(m_main_dlg->m_hplayer[0], PARAM_AUDIO_VOLUME, (void*)&volume);
}

void CRealWndDlg::OnVolumeZero()
{
	int volume = m_VolumeSlider->GetValue();
	FindChildByName2<SWindow>(L"btn_volume")->SetVisible(TRUE, TRUE);
	FindChildByName2<SWindow>(L"btn_volume_zero")->SetVisible(FALSE, TRUE);
	player_setparam(m_main_dlg->m_hplayer[0], PARAM_AUDIO_VOLUME, (void*)&volume);
}

void CRealWndDlg::OnScreenFull()
{
	if (!m_bFullScreenMode)
		FullScreen(true);
	else
		FullScreen(false);
}


void CRealWndDlg::FullScreen(BOOL bFull)
{
	if (!m_main_dlg->m_hplayer[0]) return;

	int iBorderX = GetSystemMetrics(SM_CXFIXEDFRAME) + GetSystemMetrics(SM_CXBORDER);
	int iBorderY = GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYBORDER);
	m_bFullScreenMode = bFull;

	if (bFull)
	{

		::GetWindowPlacement(m_main_dlg->m_hWnd, &OldWndPlacement);

		if (::IsZoomed(m_main_dlg->m_hWnd))
		{
			::ShowWindow(m_main_dlg->m_hWnd, SW_SHOWDEFAULT);
		}

		::SetWindowPos(m_main_dlg->m_hWnd, HWND_TOP, -iBorderX - m_main_dlg->m_otherWidth, -iBorderY - APPWND_TOP_HEIGHT,
			GetSystemMetrics(SM_CXSCREEN) + 2 * iBorderX + m_main_dlg->m_otherWidth, GetSystemMetrics(SM_CYSCREEN) + 2 * iBorderY + m_main_dlg->m_otherHeight, 0);
		//m_otherWidth = 0;
		//m_otherHeight = 0;
		//SCaption  *cap = FindChildByID2<SCaption>(6000);
		//cap->SetVisible(FALSE, TRUE);
		//cap = FindChildByID2<SCaption>(7000);
		//cap->SetVisible(FALSE, TRUE);
		//FindChildByID2<SWindow>(8000)->SetVisible(FALSE, TRUE);
	}
	else
	{
		//m_otherWidth = APPWND_LEFT_WIDTH;
		//m_otherHeight = APPWND_TOP_HEIGHT;
		::SetWindowPlacement(m_main_dlg->m_hWnd, &OldWndPlacement);
		::SetWindowPos(m_main_dlg->m_hWnd, HWND_NOTOPMOST, 0, 0, APPWND_LEFT_WIDTH, APPWND_TOP_HEIGHT, SWP_NOSIZE | SWP_NOMOVE);

		//SCaption  *cap = FindChildByID2<SCaption>(6000);
		//cap->SetVisible(TRUE, TRUE);
		//cap = FindChildByID2<SCaption>(7000);
		//cap->SetVisible(TRUE, TRUE);
		//FindChildByID2<SWindow>(8000)->SetVisible(m_bOpenPlayList, TRUE);
	}
}
// 键盘消息
void CRealWndDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_ESCAPE:
		FullScreen(FALSE);
		break;
	case VK_CONTROL:
		m_ctrl_down = TRUE;
		break;
	case 'R'://Ctrl + R
		if (m_ctrl_down)
		{
			OnRecord();
		}
		break;
	case 'S'://Ctrl + S
		if (m_ctrl_down)
		{
			OnSnapshot();
		}
		break;
	}
	SetMsgHandled(false);
}

void CRealWndDlg::OnLbuttonDBLCLK(UINT nFlags, CPoint point)
{	
	//双击全屏/退出
	if (m_main_dlg)
		m_bFullScreenMode ? FullScreen(FALSE) : FullScreen(TRUE);
	
	SetMsgHandled(false);
}

void CRealWndDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_main_dlg)
	{
		player_leftbtndown(m_main_dlg->m_hplayer[0], point.x, point.y);
	}

	if (m_VolumeSlider->GetWindowRect().PtInRect(point))//处理音量
		m_LButtonDown = 1;

	//if (!m_bLiveStream) {
	//	if (point.y > m_rtClient.bottom - 8) {
	//		LONGLONG total = 1;
	//		player_getparam(m_ffPlayer, PARAM_MEDIA_DURATION, &total);
	//		KillTimer(TIMER_ID_PROGRESS);
	//		player_seek(m_ffPlayer, total * point.x / m_rtClient.right, SEEK_PRECISELY);
	//		SetTimer(TIMER_ID_PROGRESS, 100, NULL);
	//	}
	//	else {
	//		if (!m_bPlayPause) player_pause(m_main_dlg->m_hplayer);
	//		else player_play(m_ffPlayer);
	//		m_bPlayPause = !m_bPlayPause;
	//	}
	//}
	SetMsgHandled(false);
}

void CRealWndDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_main_dlg)
	{
		player_leftbtnup(m_main_dlg->m_hplayer[0]);
	}
	SetMsgHandled(false);
}

void CRealWndDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_main_dlg)
	{
		player_mousemove(m_main_dlg->m_hplayer[0], point.x, point.y);
	}
	SetMsgHandled(false);
}

BOOL CRealWndDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	if (m_main_dlg)
	{
		player_mousewheel(m_main_dlg->m_hplayer[0], zDelta);
	}
	SetMsgHandled(false);
	return TRUE;
}

LRESULT CRealWndDlg::openVideo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_main_dlg = (CMainDlg*)lParam;
	return 0;
}

LRESULT CRealWndDlg::closeVideo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RELEASEPLAYER(m_main_dlg->m_hplayer[0]);
	
	return 0;
}

LRESULT CRealWndDlg::playVideo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case MSG_OPEN_FAILED:
		RELEASEPLAYER(m_main_dlg->m_hplayer[0]);
		SMessageBox(NULL, _T("播放失败，请重新再试！"), _T("提示"), MB_OK | MB_ICONERROR);
		break;
	case MSG_OPEN_DONE:
		player_play(m_main_dlg->m_hplayer[0]);
		player_setrect(m_main_dlg->m_hplayer[0], 0, 0, 0, m_main_dlg->m_rtClient.cx - m_main_dlg->m_otherWidth, m_main_dlg->m_rtClient.cy - m_main_dlg->m_otherHeight);
		player_setrect(m_main_dlg->m_hplayer[0], 1, 0, 0, m_main_dlg->m_rtClient.cx - m_main_dlg->m_otherWidth, m_main_dlg->m_rtClient.cy - m_main_dlg->m_otherHeight);
		m_main_dlg->m_isplaying = TRUE;
		//m_main_dlg->OnPlaySwitchPause();
		//m_main_dlg->OnPlayProgress();
		break;
	case MSG_PLAY_COMPLETED:
		RELEASEPLAYER(m_main_dlg->m_hplayer[0]);
		m_main_dlg->m_isplaying = FALSE;
		//m_main_dlg->OnPlaySwitchPause();
		break;
	}
	return 0;
}

LRESULT CRealWndDlg::OnMsg_HTTP_TASK(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	SocketRetData *pEvt = (SocketRetData *)wp;
	Json::Reader reader;
	Json::Value jsonobj;
	if (!reader.parse(pEvt->hData, jsonobj))
	{
		//SMessageBox(NULL, _T("网络错误，请检查网络后重试！"), _T("提示"),  MB_ICONERROR);
		return false;
	}

	switch (pEvt->opt)
	{
	case OPT_GET_QRCODE:
		if (pEvt->retOK)
		{
			IBitmap *pBitmap = NULL;
			if (IsFileExist(LQRCODE_PATH))
			{
				pBitmap = SResLoadFromFile::LoadImage(LQRCODE_PATH);
				FindChildByName2<SImageWnd>(L"img_qrcode")->SetImage(pBitmap, kHigh_FilterLevel);
			}
		}
		break;
	case OPT_QRCODE_STATUS:
		if (pEvt->retOK)
		{
			switch (pEvt->retValue)
			{
			case 1:
				KillTimer(TIMER_ID_QRCODE_STATUS);
				m_main_dlg->m_loginInfo.uid = jsonobj["uid"].asString();
				m_main_dlg->m_loginInfo.token = jsonobj["access_token"].asString();
				m_main_dlg->m_loginInfo.refToken = jsonobj["refresh_token"].asString();
				if (!m_main_dlg->m_loginInfo.token.empty())
				{
					saveUserInfo();
					m_main_dlg->loadDeviceInfo();
					m_main_dlg->GetCameraList();
					m_main_dlg->LoadAvatar();
					m_main_dlg->GetUserInfoRequest();
					CameraOnSwitchNorPlayer();
				}
				break;
			case -1:
				FindChildByName2<SImageWnd>(L"img_qrcode")->SetAttribute(L"alpha", L"30", FALSE);
				FindChildByName2<SWindow>(L"ref_qrcode")->SetVisible(TRUE);
				KillTimer(TIMER_ID_QRCODE_STATUS);
				break;
			}
		}
		break;
	case OPT_PASSWD_LOGIN:
		if (pEvt->retOK)
		{
			m_main_dlg->m_loginInfo.uid = jsonobj["uid"].asString();
			m_main_dlg->m_loginInfo.token = jsonobj["access_token"].asString();
			m_main_dlg->m_loginInfo.refToken = jsonobj["refresh_token"].asString();
			if (!m_main_dlg->m_loginInfo.token.empty())
			{
				saveUserInfo();
				m_main_dlg->loadDeviceInfo();
				m_main_dlg->GetCameraList();
				m_main_dlg->LoadAvatar();
				m_main_dlg->GetUserInfoRequest();
				CameraOnSwitchNorPlayer();
			}
		}
		else
			SMessageBox(NULL, _T("账号或密码错误，请输入正确的爱耳目账号密码！"), _T("提示"), MB_OK | MB_ICONERROR);
		break;
	case OPT_REGISTRE:

		break;
	case OPT_GETDEVICEINFO:

		break;
	default:
		break;

	}

	delete pEvt;
	return true;
}


