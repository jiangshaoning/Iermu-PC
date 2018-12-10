#pragma once
#include "stdafx.h"
#include <helper/SAdapterBase.h>
#include "FilesHelp.h"
#include <algorithm>

//���ز���list����
class CFileListWnd : public SMcAdapterBase
{
public:
	CFileListWnd(HWND _hwnd):_m_hwnd(_hwnd),m_sel_index(-1)
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
		SStatic *pBtnsave = pItem->FindChildByName2<SStatic>(L"filename");
		pBtnsave->SetWindowTextW(SStringT().Format(L"%s", info.m_name));
		//pBtnsave->SetAttribute(L"tip", S_CT2W(info.m_name));
		pItem->SetUserData(position);
		pItem->GetEventSet()->subscribeEvent(EventItemPanelDbclick::EventID, Subscriber(&CFileListWnd::OnButtonDbclick, this));

	}

	bool OnButtonDbclick(EventArgs *pEvt)//�����ļ�
	{
		SWindow *btn = sobj_cast<SWindow>(pEvt->sender);
		string  _pathname = S_CT2A(m_db[btn->GetUserData()].m_FULL_Path);
		m_sel_index=btn->GetUserData();
		::PostMessage(_m_hwnd, MS_PLAYING_PATHNAME, 0, (LPARAM)_pathname.c_str());
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
	void Sort_Play_list(int id)//����
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
	
	SArray<PlaylistInfo> m_db;
	int					 m_sel_index;
};