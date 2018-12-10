#include "StdAfx.h"
#include "SouiRealWndHandler.h"
#include "RealWndDlg.h"
#include "RealWndDlg_Dev.h"
#include "RealWndDlg_File.h"

//#include "RealWndDlg_URL.h"
namespace SOUI
{
    CSouiRealWndHandler::CSouiRealWndHandler(void)
    {
    }

    CSouiRealWndHandler::~CSouiRealWndHandler(void)
    {
    }

    HWND CSouiRealWndHandler::OnRealWndCreate( SRealWnd *pRealWnd )
    {
        const SRealWndParam &param=pRealWnd->GetRealWndParam();
        if(param.m_strClassName==_T("CRealWndDlg"))
        {
			CRealWndDlg *wndDlg = new CRealWndDlg;
			wndDlg->Create(pRealWnd->GetContainer()->GetHostHwnd(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_CLIPSIBLINGS, 0, 0, 0, 0, 0);

			//��pbtn��ָ��ŵ�SRealWnd��Data�б��棬�Ա��ڴ���destroyʱ�ͷ�pbtn����
            pRealWnd->SetData(wndDlg);
            //���سɹ�������Ĵ��ھ��
            return wndDlg->m_hWnd;
		}
		else  if (param.m_strClassName == _T("CRealWndDlg_Dev"))
		{
			CRealWndDlg_Dev *wndDlg_Dev = new CRealWndDlg_Dev;
			wndDlg_Dev->Create(pRealWnd->GetContainer()->GetHostHwnd(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_CLIPSIBLINGS, 0, 0, 0, 0, 0);
			pRealWnd->SetData(wndDlg_Dev);
			return wndDlg_Dev->m_hWnd;
		}
		else  if (param.m_strClassName == _T("CRealWndDlg_File"))
		{
			SHostWnd *wndDlg_File = new CRealWndDlg_File;
			wndDlg_File->Create(pRealWnd->GetContainer()->GetHostHwnd(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_CLIPSIBLINGS, 0, 0, 0, 0, 0);
			pRealWnd->SetData(wndDlg_File);
			return wndDlg_File->m_hWnd;
		}
		else
        {
            return 0;
        }
    }

    void CSouiRealWndHandler::OnRealWndDestroy( SRealWnd *pRealWnd )
    {
        const SRealWndParam &param=pRealWnd->GetRealWndParam();
        if(param.m_strClassName==_T("CRealWndDlg"))
        {//�����洰�ڣ��ͷŴ���ռ�õ��ڴ�
            CRealWndDlg *pbtn=(CRealWndDlg*) pRealWnd->GetData();
            if(pbtn)
            {
                pbtn->DestroyWindow();
                delete pbtn;
            }
		}
		else if (param.m_strClassName == _T("CRealWndDlg_Dev"))
		{
			CRealWndDlg_Dev *pbtn_url = (CRealWndDlg_Dev*)pRealWnd->GetData();
			if (pbtn_url)
			{
				pbtn_url->DestroyWindow();
				delete pbtn_url;
			}
		}
		//else if (param.m_strClassName == _T("CRealWndDlg_ABOUT"))
		//{
		//	CRealWndDlg_URL *pbtn_about = (CRealWndDlg_URL*)pRealWnd->GetData();
		//	if (pbtn_about)
		//	{
		//		pbtn_about->DestroyWindow();
		//		delete pbtn_about;
		//	}
		//}
		//else if (param.m_strClassName == _T("CRealWndDlg_DEPOT"))
		//{
		//	SHostWnd *pbtn_depot = (SHostWnd*)pRealWnd->GetData();
		//	if (pbtn_depot)
		//	{
		//		pbtn_depot->DestroyWindow();
		//		delete pbtn_depot;
		//	}
		//}
		//else if (param.m_strClassName == _T("CRealWndDlg_SKIN"))
		//{
		//	SHostWnd *pbtn_skin = (SHostWnd*)pRealWnd->GetData();
		//	if (pbtn_skin)
		//	{
		//		pbtn_skin->DestroyWindow();
		//		delete pbtn_skin;
		//	}
		//}
		 
    }
    
    //����������FALSE
    BOOL CSouiRealWndHandler::OnRealWndSize( SRealWnd *pRealWnd )
    {
        return FALSE;
    }

    //����������FALSE
    BOOL CSouiRealWndHandler::OnRealWndInit( SRealWnd *pRealWnd )
    {
        return FALSE;
    }
}
