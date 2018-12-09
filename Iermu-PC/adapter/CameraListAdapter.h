#pragma once
#include "stdafx.h"
#include <helper/SAdapterBase.h>
#include "MainDlg.h"
#include <algorithm>

//本地播放list窗口
class CCameraListWnd : public SMcAdapterBase
{
public:
	CMainDlg*				m_dlg;

	CCameraListWnd(HWND _hwnd) :_m_hwnd(_hwnd), m_SipItem(NULL), m_DbpItem(NULL), m_sel_index(-1)
	{
	}
	virtual int getCount()
	{
		return m_cameraList->GetCount();
	}
	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		CameraItem info = m_cameraList->GetAt(position);
		SImageWnd* camera_icon = pItem->FindChildByName2<SImageWnd>(L"camera_icon");
		SStatic *camera_name = pItem->FindChildByName2<SStatic>(L"camera_name");
		camera_name->SetWindowTextW(Get_index_Desc(position));
		if (info.status != TYPE_STATUS_ONLINE)
		{
			camera_icon->SetAttribute(L"skin", L"_icon.camera_gray");
			camera_name->SetAttribute(L"colorText", L"@color/graytext");
		}

		pItem->SetUserData(position);
		pItem->GetEventSet()->subscribeEvent(EventItemPanelClick::EventID, Subscriber(&CCameraListWnd::OnButtonclick, this));
		pItem->GetEventSet()->subscribeEvent(EventItemPanelDbclick::EventID, Subscriber(&CCameraListWnd::OnButtonDbclick, this));

	}

	bool OnButtonclick(EventArgs *pEvt)
	{
		if (m_SipItem)
		{
			if (m_cameraList->GetAt(m_sel_index).status != TYPE_STATUS_ONLINE)
				m_SipItem->FindChildByName2<SStatic>(L"camera_name")->SetAttribute(L"colorText", L"@color/graytext", FALSE);
			else
				m_SipItem->FindChildByName2<SStatic>(L"camera_name")->SetAttribute(L"colorText", L"@color/white", FALSE);
		}

		m_SipItem = sobj_cast<SWindow>(pEvt->sender);
		m_sel_index = m_SipItem->GetUserData();

		m_SipItem->FindChildByName2<SStatic>(L"camera_name")->SetAttribute(L"colorText", L"@color/blue", FALSE);

		return true;
	}

	bool OnButtonDbclick(EventArgs *pEvt)
	{
		if (m_DbpItem)
		{
			if (m_cameraList->GetAt(m_sel_index).status != TYPE_STATUS_ONLINE)
				m_SipItem->FindChildByName2<SStatic>(L"camera_name")->SetAttribute(L"colorText", L"@color/graytext", FALSE);
			else
				m_SipItem->FindChildByName2<SStatic>(L"camera_name")->SetAttribute(L"colorText", L"@color/white", FALSE);

			m_DbpItem->FindChildByName2<SGifPlayer>(L"playing_camera")->SetVisible(FALSE, TRUE);
			m_DbpItem->FindChildByName2<SImageWnd>(L"camera_icon")->SetVisible(TRUE, TRUE);
		}

		m_DbpItem = sobj_cast<SWindow>(pEvt->sender);
		m_sel_index = m_DbpItem->GetUserData();
		m_dlg->m_playcamera_index = m_sel_index;

		m_DbpItem->FindChildByName2<SStatic>(L"camera_name")->SetAttribute(L"colorText", L"@color/blue", FALSE);
		m_DbpItem->FindChildByName2<SImageWnd>(L"camera_icon")->SetVisible(FALSE, TRUE);
		m_DbpItem->FindChildByName2<SGifPlayer>(L"playing_camera")->SetVisible(TRUE, TRUE);

		string data = "access_token=";
		data.append(m_dlg->m_loginInfo.token).append("&deviceid=").append(m_cameraList->GetAt(m_sel_index).deviceid).append("&method=liveplay");
		m_dlg->SendCMD(OPT_GETPLAYURL, m_dlg->m_hWnd, POST, GETDEVICEINFO_URL, data);
		RELEASEPLAYER(m_dlg->m_hplayer[0]);

		//m_dlg->Play("d:\\1.mp4");

		//string  _pathname = m_cameraList->GetAt(m_sel_index).description;	
		//::PostMessageW(_m_hwnd, MS_PLAYING_INDEX, 0, (LPARAM)_pathname.c_str());	
		return true;
	}

	void ADD_Tags(CMainDlg*	dlg, SArray<CameraItem>  *cList)
	{
		m_dlg = dlg;
		m_cameraList = cList;
		Sort_Camera_list();
		notifyDataSetChanged();
	}

	void DELL_ALL()
	{
		m_cameraList->RemoveAll();
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
		SStringA description = S_CA2A(m_cameraList->GetAt(items).description.c_str());
		return S_CA2T(description);
	}

	void Sort_Camera_list()//排序
	{
		qsort(m_cameraList->GetData(), m_cameraList->GetCount(), sizeof(CameraItem), TagCheckCmp);
	}

	SStringW GetColumnName(int iCol) const {
		return SStringW().Format(L"col%d", iCol + 1);
	}
	
private:
	
	HWND  _m_hwnd;
	struct PlaylistInfo
	{
		SStringT m_FULL_Path, m_name, m_ext, m_file_size;
	};

	static int __cdecl TagCheckCmp(const void * p1, const void*p2)
	{
		const CameraItem *tag1 = (const CameraItem*)p1;
		const CameraItem *tag2 = (const CameraItem*)p2;
		return tag1->tag.Compare(tag2->tag);
	}

	SArray<CameraItem>*		m_cameraList;
	SWindow*				m_SipItem;			//记住上一次单击选择的item
	SWindow*				m_DbpItem;			//记住上一次双击选择的item
	int						m_sel_index;		//当前选择的index
};