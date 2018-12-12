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

			//把pbtn的指针放到SRealWnd的Data中保存，以便在窗口destroy时释放pbtn对象。
            pRealWnd->SetData(wndDlg);
            //返回成功创建后的窗口句柄
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
			CRealWndDlg_File *wndDlg_File = new CRealWndDlg_File;
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
        {//销毁真窗口，释放窗口占用的内存
            CRealWndDlg *pbtn=(CRealWndDlg*) pRealWnd->GetData();
            if(pbtn)
            {
                pbtn->DestroyWindow();
                delete pbtn;
            }
		}
		else if (param.m_strClassName == _T("CRealWndDlg_Dev"))
		{
			CRealWndDlg_Dev *pbtn_dev = (CRealWndDlg_Dev*)pRealWnd->GetData();
			if (pbtn_dev)
			{
				pbtn_dev->DestroyWindow();
				delete pbtn_dev;
			}
		}
		else if (param.m_strClassName == _T("CRealWndDlg_File"))
		{
			CRealWndDlg_File *pbtn_file = (CRealWndDlg_File*)pRealWnd->GetData();
			if (pbtn_file)
			{
				pbtn_file->DestroyWindow();
				delete pbtn_file;
			}
		}
		 
    }
    
    //不处理，返回FALSE
    BOOL CSouiRealWndHandler::OnRealWndSize( SRealWnd *pRealWnd )
    {
        return FALSE;
    }

    //不处理，返回FALSE
    BOOL CSouiRealWndHandler::OnRealWndInit( SRealWnd *pRealWnd )
    {
        return FALSE;
    }
}
