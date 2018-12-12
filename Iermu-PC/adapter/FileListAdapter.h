#pragma once
#include "stdafx.h"
#include <helper/SAdapterBase.h>
#include "FilesHelp.h"
#include <algorithm>

//本地播放list窗口
class CFileListWnd : public SMcAdapterBase
{
public:
	CFileListWnd(HWND _hwnd) :_m_hwnd(_hwnd), m_SipItem(NULL), m_DbpItem(NULL), m_sel_index(-1)
	{
	}
	virtual int getCount()
	{
		return m_db.GetCount();
	}
	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}
		PlaylistInfo info = m_db[position];
		SStatic *pBtnsave = pItem->FindChildByName2<SStatic>(L"file_name");
		pBtnsave->SetWindowTextW(SStringT().Format(L"%s", info.m_name));
		pItem->SetUserData(position);
		pItem->GetEventSet()->subscribeEvent(EventItemPanelClick::EventID, Subscriber(&CFileListWnd::OnButtonclick, this));
		pItem->GetEventSet()->subscribeEvent(EventItemPanelDbclick::EventID, Subscriber(&CFileListWnd::OnButtonDbclick, this));

	}

	bool OnButtonclick(EventArgs *pEvt)
	{
		if (m_SipItem)
		{
			m_SipItem->FindChildByName2<SStatic>(L"file_name")->SetAttribute(L"colorText", L"@color/white", FALSE);
		}

		m_SipItem = sobj_cast<SWindow>(pEvt->sender);
		m_sel_index = m_SipItem->GetUserData();

		m_SipItem->FindChildByName2<SStatic>(L"file_name")->SetAttribute(L"colorText", L"@color/blue", FALSE);

		return true;
	}

	bool OnButtonDbclick(EventArgs *pEvt)//播放文件
	{
		if (m_DbpItem)
		{
			m_SipItem->FindChildByName2<SStatic>(L"file_name")->SetAttribute(L"colorText", L"@color/white", FALSE);
			m_DbpItem->FindChildByName2<SGifPlayer>(L"playing_file")->SetVisible(FALSE, TRUE);
			m_DbpItem->FindChildByName2<SImageWnd>(L"file_icon")->SetVisible(TRUE, TRUE);
		}

		m_DbpItem = sobj_cast<SWindow>(pEvt->sender);
		m_sel_index = m_DbpItem->GetUserData();
		string  _pathname = S_CT2A(m_db[m_sel_index].m_FULL_Path);

		m_DbpItem->FindChildByName2<SStatic>(L"file_name")->SetAttribute(L"colorText", L"@color/blue", FALSE);
		m_DbpItem->FindChildByName2<SImageWnd>(L"file_icon")->SetVisible(FALSE, TRUE);
		m_DbpItem->FindChildByName2<SGifPlayer>(L"playing_file")->SetVisible(TRUE, TRUE);
		//STabCtrl *pTab = m_DbpItem->FindChildByName2<STabCtrl>(L"file_tab");
		//pTab->SetCurSel(_T("file_player_page"));

		::SendMessageW(_m_hwnd, MS_PLAYING_PATHNAME, 0, (LPARAM)_pathname.c_str());
		return true;
	}
	void ADD_files(SStringT  m_path)
	{
		PlaylistInfo info;
		info.m_FULL_Path = m_path;
		info.m_file_size = CFileHelp::FileSizeToString(CFileHelp::GetFileSize(m_path));
		CFileHelp::SplitPathFileName(m_path, info.m_name, info.m_ext);
		m_db.Add(info);
	}
	void Dll_File(int _items)
	{
		m_db.RemoveAt(_items);
		if(m_sel_index> (int)m_db.GetCount()-1)
			m_sel_index=m_db.GetCount()-1;
		notifyDataSetChanged();
	}
	
	
	void DELL_ALL()
	{
		m_db.RemoveAll();
		m_sel_index=-1;
		notifyDataSetChanged();

	}
	int Get_Play_index()
	{
		return m_sel_index;
	}
	void Set_Play_index(int items)
	{
		m_sel_index=items;
	}
	SStringT Get_index_Path(int items)
	{
		return m_db[items].m_FULL_Path;
	}
	struct SORTCTX
	{
		int iCol;
	};
	void Sort_Play_list(int id)//排序
	{
		SORTCTX ctx = { 1 };
		if(id==1)
			qsort_s(m_db.GetData(), m_db.GetCount(), sizeof(m_db), SortCmp_name, &ctx);
		else qsort_s(m_db.GetData(), m_db.GetCount(), sizeof(m_db), SortCmp_ext, &ctx);
		notifyDataSetChanged();
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
	static int __cdecl SortCmp_name(void *context, const void * p1, const void * p2)
	{
		SORTCTX *pctx = (SORTCTX*)context;
		const PlaylistInfo *pSI1 = (const PlaylistInfo*)p1;
		const PlaylistInfo *pSI2 = (const PlaylistInfo*)p2;
		int nRet = 0;
		nRet = wcscmp(pSI1->m_name, pSI2->m_name);
		return nRet;
	}
	static int __cdecl SortCmp_ext(void *context, const void * p1, const void * p2)
	{
		SORTCTX *pctx = (SORTCTX*)context;
		const PlaylistInfo *pSI1 = (const PlaylistInfo*)p1;
		const PlaylistInfo *pSI2 = (const PlaylistInfo*)p2;
		int nRet = 0;
		nRet = wcscmp(pSI1->m_ext, pSI2->m_ext);
		return nRet;
	}
	
	SArray<PlaylistInfo>	m_db;
	SWindow*				m_SipItem;			//记住上一次单击选择的item
	SWindow*				m_DbpItem;			//记住上一次双击选择的item
	int						m_sel_index;
};