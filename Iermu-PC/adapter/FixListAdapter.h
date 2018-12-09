#include <helper/SAdapterBase.h>
#include "MainDlg.h"

class CFixListWnd : public SAdapterBase
{
protected:

	SArray<NetInfo>    m_netInfo;
public:
	CFixListWnd(SArray<NetInfo>  &ui)
	{
		m_netInfo.RemoveAll();
		for (unsigned int i = 0; i < ui.GetCount(); i++)
		{
			m_netInfo.Add(ui.GetAt(i));
		}
	}
	virtual int getCount()
	{
		return m_netInfo.GetCount();
	}

	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}
		SWindow *pName = pItem->FindChildByID(1);
		pName->SetWindowText(m_netInfo.GetAt(position).strIp);
		//SWindow *pAccount = pItem->FindChildByID(2);
		//pAccount->SetWindowText(m_netInfo.GetAt(position).strName);
	}

	SStringT getItemDesc(int position)
	{
		return m_netInfo.GetAt(position).strIp;
	}


	NetInfo getItem(int position)
	{
		SASSERT(position >= 0 && position < (int)m_netInfo.GetCount());
		return m_netInfo[position];
	}
};