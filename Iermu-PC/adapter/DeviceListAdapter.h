#pragma once
#include "stdafx.h"
#include <helper/SAdapterBase.h>
#include "MainDlg.h"

class CDeviceListWnd :public SMcAdapterBase
{
public:
	CMainDlg*				m_dlg;

	CDeviceListWnd(HWND _hwnd) :_m_hwnd(_hwnd), m_SipItem(NULL), m_DbpItem(NULL), m_sel_index(-1)
	{
	}
	virtual int getCount()
	{
		return m_deviceList->GetCount();
	}
	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		CameraAddr info = m_deviceList->GetAt(position);
		SImageWnd* device_icon = pItem->FindChildByName2<SImageWnd>(L"device_icon");
		SStatic *device_name = pItem->FindChildByName2<SStatic>(L"device_name");
		device_name->SetWindowTextW(Get_index_Desc(position));

		pItem->SetUserData(position);
		pItem->GetEventSet()->subscribeEvent(EventItemPanelClick::EventID, Subscriber(&CDeviceListWnd::OnButtonclick, this));
		pItem->GetEventSet()->subscribeEvent(EventItemPanelDbclick::EventID, Subscriber(&CDeviceListWnd::OnButtonDbclick, this));

	}

	bool OnButtonclick(EventArgs *pEvt)
	{
		if (m_SipItem)
		{
			m_SipItem->FindChildByName2<SStatic>(L"device_name")->SetAttribute(L"colorText", L"@color/white", FALSE);
		}

		m_SipItem = sobj_cast<SWindow>(pEvt->sender);
		m_sel_index = m_SipItem->GetUserData();

		m_SipItem->FindChildByName2<SStatic>(L"device_name")->SetAttribute(L"colorText", L"@color/blue", FALSE);

		return true;
	}

	bool OnButtonDbclick(EventArgs *pEvt)
	{
		if (m_DbpItem)
		{
			m_SipItem->FindChildByName2<SStatic>(L"device_name")->SetAttribute(L"colorText", L"@color/white", FALSE);
			m_DbpItem->FindChildByName2<SGifPlayer>(L"playing_device")->SetVisible(FALSE, TRUE);
			m_DbpItem->FindChildByName2<SImageWnd>(L"device_icon")->SetVisible(TRUE, TRUE);
		}

		m_DbpItem = sobj_cast<SWindow>(pEvt->sender);
		m_sel_index = m_DbpItem->GetUserData();
		m_dlg->m_playdevice_index = m_sel_index;

		m_DbpItem->FindChildByName2<SStatic>(L"device_name")->SetAttribute(L"colorText", L"@color/blue", FALSE);
		m_DbpItem->FindChildByName2<SImageWnd>(L"device_icon")->SetVisible(FALSE, TRUE);
		m_DbpItem->FindChildByName2<SGifPlayer>(L"playing_device")->SetVisible(TRUE, TRUE);

		RELEASEPLAYER(m_dlg->m_hplayer[1]);

		m_dlg->PlayLan(m_deviceList->GetAt(m_sel_index).url);
		return true;
	}

	void ADD_Tags(CMainDlg*	dlg, SArray<CameraAddr>  *cList)
	{
		m_dlg = dlg;
		m_deviceList = cList;
		notifyDataSetChanged();
	}

	void DELL_ALL()
	{
		m_deviceList->RemoveAll();
		m_sel_index = -1;
		notifyDataSetChanged();
	}

	int Get_Play_index()
	{
		return m_sel_index;
	}

	void Set_Play_index(int items)
	{
		m_sel_index = items;
	}

	SStringT Get_index_Desc(int items)
	{
		SStringA description = m_deviceList->GetAt(items).cameraip;
		return S_CA2T(description);
	}

	SStringW GetColumnName(int iCol) const {
		return SStringW().Format(L"col%d", iCol + 1);
	}

private:

	HWND  _m_hwnd;

	SArray<CameraAddr>*		m_deviceList;
	SWindow*				m_SipItem;			//记住上一次单击选择的item
	SWindow*				m_DbpItem;			//记住上一次双击选择的item
	int						m_sel_index;		//当前选择的index
};

