
#include "stdafx.h"
#include "RealWndDlg_Dev.h"

CRealWndDlg_Dev::CRealWndDlg_Dev():SHostDialog(_T("LAYOUT:XML_REALWND_DEV"))								
{
	//m_menu_open.LoadMenu(_T("LAYOUT:menu_open"));
	m_main_dlg = NULL;
	m_ctrl_down = FALSE;
	m_bIsRecording = FALSE;
	m_bFullScreenMode = FALSE;
	m_LButtonDown = 0;
}

CRealWndDlg_Dev::~CRealWndDlg_Dev(void)
{
	RELEASEPLAYER(m_main_dlg->m_hplayer[1]);
}

LRESULT CRealWndDlg_Dev::OnInitRealWnd(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_main_dlg = (CMainDlg*)lParam;
	m_VolumeSlider = FindChildByName2<SSliderBar>(L"volumeSlider");
	if (m_VolumeSlider)
	{
		int volume = m_VolumeSlider->GetValue();
		player_setparam(m_main_dlg->m_hplayer[1], PARAM_AUDIO_VOLUME, (void*)&volume);
	}

	return 0;
}

BOOL CRealWndDlg_Dev::IsFileExist(const SStringT& csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

void CRealWndDlg_Dev::OnTimer(UINT_PTR nIDEvent)
{
	int64_t pos = 0;
	int64_t total = 1;
	int value = 0;
	SetMsgHandled(FALSE);
	switch (nIDEvent)
	{
	case TIMER_ID_HIDE_TEXT:
		KillTimer(TIMER_ID_HIDE_TEXT);
		player_textout(m_main_dlg->m_hplayer[1], 0, 0, 0, NULL);
		m_strTxt[0] = '\0';
		break;
	}
}

void CRealWndDlg_Dev::deviceOnSwitchToPlayer()
{
	m_main_dlg->GetDeviceList();
}

void CRealWndDlg_Dev::GetCurTimeName(char* Ctime, wchar_t* Wtime, char* name, char* postfix)
{
	time_t curtime = time(NULL);
	tm *ptm = localtime(&curtime);

	sprintf(Ctime, "%s_%d%02d%02d%02d%02d%02d.%s", name, ptm->tm_year + 1900, ptm->tm_mon + 1,
		ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, postfix);

	MultiByteToWideChar(CP_ACP, 0, Ctime, -1, Wtime, 40);

}

void CRealWndDlg_Dev::PlayerShowText(int time)
{
	player_textout(m_main_dlg->m_hplayer[1], 20, 20, RGB(0, 255, 0), m_strTxt);
	SetTimer(TIMER_ID_HIDE_TEXT, time);
}

void CRealWndDlg_Dev::OnSnapshot()
{
	if (!m_main_dlg->m_hplayer[1]) return;

	char ctime[40] = { 0 };
	wchar_t wtime[48] = { 0 };
	GetCurTimeName(ctime, wtime, "snapshot", "jpg");
	player_snapshot(m_main_dlg->m_hplayer[1], ctime, 0, 0, 1000);
	_stprintf(m_strTxt, _T("抓拍到当前路径：%s"), wtime);
	PlayerShowText(3000);
}

void CRealWndDlg_Dev::OnRecord()
{
	if (!m_main_dlg->m_hplayer[1]) return;

	char ctime[40] = { 0 };
	wchar_t wtime[48] = { 0 };
	GetCurTimeName(ctime, wtime, "record", "mp4");
	player_record(m_main_dlg->m_hplayer, m_bIsRecording ? NULL : ctime);
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

void CRealWndDlg_Dev::OnVolume()
{
	int volume = -182;
	FindChildByName2<SWindow>(L"btn_volume")->SetVisible(FALSE, TRUE);
	FindChildByName2<SWindow>(L"btn_volume_zero")->SetVisible(TRUE, TRUE);
	player_setparam(m_main_dlg->m_hplayer[1], PARAM_AUDIO_VOLUME, (void*)&volume);
}

void CRealWndDlg_Dev::OnVolumeZero()
{
	int volume = m_VolumeSlider->GetValue();
	FindChildByName2<SWindow>(L"btn_volume")->SetVisible(TRUE, TRUE);
	FindChildByName2<SWindow>(L"btn_volume_zero")->SetVisible(FALSE, TRUE);
	player_setparam(m_main_dlg->m_hplayer[1], PARAM_AUDIO_VOLUME, (void*)&volume);
}

void CRealWndDlg_Dev::OnScreenFull()
{
	if (!m_bFullScreenMode)
		FullScreen(true);
	else
		FullScreen(false);
}


void CRealWndDlg_Dev::FullScreen(BOOL bFull)
{
	if (!m_main_dlg->m_hplayer[1]) return;

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
void CRealWndDlg_Dev::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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

void CRealWndDlg_Dev::OnLbuttonDBLCLK(UINT nFlags, CPoint point)
{	
	//双击全屏/退出
	if (m_main_dlg)
		m_bFullScreenMode ? FullScreen(FALSE) : FullScreen(TRUE);
	
	SetMsgHandled(false);
}

void CRealWndDlg_Dev::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_main_dlg)
	{
		player_leftbtndown(m_main_dlg->m_hplayer[1], point.x, point.y);
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
	//		if (!m_bPlayPause) player_pause(m_main_dlg->m_hplayer[1]);
	//		else player_play(m_ffPlayer);
	//		m_bPlayPause = !m_bPlayPause;
	//	}
	//}
	SetMsgHandled(false);
}

void CRealWndDlg_Dev::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_main_dlg)
	{
		player_leftbtnup(m_main_dlg->m_hplayer[1]);
	}
	SetMsgHandled(false);
}

void CRealWndDlg_Dev::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_main_dlg)
	{
		player_mousemove(m_main_dlg->m_hplayer[1], point.x, point.y);
	}
	SetMsgHandled(false);
}

BOOL CRealWndDlg_Dev::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	if (m_main_dlg)
	{
		player_mousewheel(m_main_dlg->m_hplayer[1], zDelta);
	}
	SetMsgHandled(false);
	return TRUE;
}


LRESULT CRealWndDlg_Dev::OnMsg_TCP_TASK(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	SocketRetData *pEvt = (SocketRetData *)wp;
	switch (pEvt->opt)
	{
	case OPT_GETCAMERA_LIST:
		if (!pEvt->retOK)
		{
			//SetDisplayProgress(L"cameralist_win", L"refresh_progress");
			//SetDisplayProgress(L"local_ip_win", L"local_ip_progress");
			SMessageBox(NULL, _T("没有获取到局域网的设备，\n请确保局域网内有AI摄像机"), _T("提示"), MB_OK | MB_ICONERROR);
			return false;
		}
		m_main_dlg->RefDeviceAdapterView();
		STabCtrl *pTab = FindChildByName2<STabCtrl>(L"device_tab");
		if (pTab)
		{
			pTab->SetCurSel(_T("dev_player_page"));
		}
		break;
	}

	delete pEvt;
	return true;
}

LRESULT CRealWndDlg_Dev::playVideo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case MSG_OPEN_FAILED:
		RELEASEPLAYER(m_main_dlg->m_hplayer[1]);
		SMessageBox(NULL, _T("播放失败，请重新再试！"), _T("提示"), MB_OK | MB_ICONERROR);
		break;
	case MSG_OPEN_DONE:
		player_play(m_main_dlg->m_hplayer[1]);
		player_setrect(m_main_dlg->m_hplayer[1], 0, 0, 0, m_main_dlg->m_rtClient.cx - m_main_dlg->m_otherWidth, m_main_dlg->m_rtClient.cy - m_main_dlg->m_otherHeight);
		player_setrect(m_main_dlg->m_hplayer[1], 1, 0, 0, m_main_dlg->m_rtClient.cx - m_main_dlg->m_otherWidth, m_main_dlg->m_rtClient.cy - m_main_dlg->m_otherHeight);
		m_main_dlg->m_isplaying = TRUE;
		//m_main_dlg->OnPlaySwitchPause();
		//m_main_dlg->OnPlayProgress();
		break;
	case MSG_PLAY_COMPLETED:
		RELEASEPLAYER(m_main_dlg->m_hplayer[1]);
		m_main_dlg->m_isplaying = FALSE;
		//m_main_dlg->OnPlaySwitchPause();
		break;
	}
	return 0;
}

