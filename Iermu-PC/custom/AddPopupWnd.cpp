
#include "stdafx.h"
#include "../MainDlg.h"
#include "json.h"

void AddPopupWnd::OnClose()
{
	SetVisible(FALSE, TRUE);
}

void AddPopupWnd::FirstStep()
{
	SStringT  device = FindChildByName2<SEdit>(L"edit_step1_device")->GetWindowTextW();
	if (!device.GetLength())
	{
		SMessageBox(NULL, _T("请输入设备号"), _T("提示"), MB_OK | MB_ICONERROR);
		return;
	}
	int sel = FindChildByName2<SComboBox>(L"cbx_step2_cloud")->GetCurSel();
	int connect_type = sel ? 1 : 2;
	m_deviceId = S_CT2A(device);
	string data = "deviceid=";
	data.append(m_deviceId).append("&device_type=1&desc=%e6%88%91%e7%9a%84%e6%91%84%e5%83%8f%e6%9c%ba&connect_type=").append(to_string(connect_type)).append("&method=register&access_token=").append(m_dlg->m_loginInfo.token);

	m_dlg->SendCMD(OPT_REGISTRE, m_hWnd, POST, GETDEVICEINFO_URL, data);

}

void AddPopupWnd::SecondStep()
{
	STabCtrl *pTab = FindChildByName2<STabCtrl>(L"add_tab");
	if (pTab)
	{
		pTab->SetCurSel(_T("add_third_tab"));
	}
}

void AddPopupWnd::ThirdStep()
{
	STabCtrl *pTab = FindChildByName2<STabCtrl>(L"add_tab");
	if (pTab)
	{
		pTab->SetCurSel(_T("add_first_page"));
	}
}

LRESULT AddPopupWnd::OnMsg_HTTP_TASK(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	SocketRetData *pEvt = (SocketRetData *)wp;
	Json::Reader reader;
	Json::Value jsonobj;
	if (!reader.parse(pEvt->hData, jsonobj))
	{
		//SMessageBox(NULL, _T("网络错误，请检查网络后重试！"), _T("提示"), MB_OK | MB_ICONERROR);
		return false;
	}
	switch (pEvt->opt)
	{
	case OPT_REGISTRE:
		STabCtrl *pTab = FindChildByName2<STabCtrl>(L"add_tab");
		if (pTab)
		{
			pTab->SetCurSel(_T("add_second_tab"));
		}
		break;
	}

	return true;
}
