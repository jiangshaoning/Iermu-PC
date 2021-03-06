// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <time.h>
#include <Iphlpapi.h>
#include <shellapi.h>
#include "MainDlg.h"
#include "WinClientTask.h"
#include "SerializeObj.h"
#include "FixListAdapter.h"
#include "CameraListAdapter.h"
#include "DeviceLIstAdapter.h"
#include "FileListAdapter.h"
#include "UpnpTool.h"
#include "json.h"
#include "CppSQLite3.h"
#include <boost/thread.hpp>

#pragma comment(lib,"Iphlpapi.lib")

const TCHAR STR_MOVE_FILE_FILTER[] =
_T("媒体文件(所有类型)\0*.asf;*.avi;*.wm;*.wmp;*.wmv;*.ram; *.rm; *.rmvb; *.rp; *.rpm; *.rt; *.smi; *.smil;*.dat; *.m1v; *.m2p; *.m2t; *.m2ts; *.m2v; *.mp2v; *.mpeg; *.mpe; *.mpg; *.mpv2; *.pss; *.pva; *.tp; *.tpr; *.ts;*.m4b; *.m4p; *.m4v; *.mp4; *.mpeg4; *.mov; *.qt; *.f4v; *.flv; *.hlv; *.swf; *.ifo; *.vob;*.3g2; *.3gp; *.3gp2; *.3gpp; *.amv; *.bik; *.csf; *.divx; *.evo; *.ivm; *.mkv; *.mod; *.mts; *.ogm; *.pmp; *.scm; *.tod; *.vp6; *.webm; *.xlmv;*.aac; *.ac3; *.amr; *.ape; *.cda; *.dts; *.flac; *.mla; *.m2a; *.m4a; *.mid; *.midi; *.mka; *.mp2; *.mp3; *.mpa; *.ogg; *.ra; *.tak; *.tta; *.wav; *.wma; *.wv;\0")
_T("window媒体(*.asf; *.avi; *.wm; *.wmp; *.wmv;)\0*.asf; *.avi; *.wm; *.wmp; *.wmv;\0")
_T("real媒体(*.ram;*.rm;*.rmvb;*.rp;*.rpm;*.rt;*.smi;*.smil;)\0*.ram;*.rm;*.rmvb;*.rp;*.rpm;*.rt;*.smi;*.smil;\0")
_T("MPEG1/2媒体(*.dat;*.m1v;*.m2p;*.m2t;*.m2ts;*.m2v;*.mp2v;*.mpeg;*.mpe;*.mpg;*.mpv2;*.pss;*.pva;*.tp;*.tpr;*.ts;)\0*.dat; *.m1v; *.m2p; *.m2t; *.m2ts; *.m2v; *.mp2v; *.mpeg; *.mpe; *.mpg; *.mpv2; *.pss; *.pva; *.tp; *.tpr; *.ts;\0")
_T("MPEG4媒体(*.m4b;*.m4p;*.m4v;*.mp4;*.mpeg4;)\0*.m4b;*.m4p;*.m4v;*.mp4;*.mpeg4;\0")
_T("3GPP媒体(*.3g2;*.3gp;*.3gp2;*.3gpp;)\0*.3g2;*.3gp;*.3gp2;*.3gpp;\0")
_T("APPLE媒体(*.mov;*.qt;)\0*.mov;*.qt;\0")
_T("Flash媒体(*.f4v;*.flv;*.hlv;*.swf;)\0*.f4v;*.flv;*.hlv;*.swf;\0")
_T("DVD媒体(*.ifo;*.vob;)\0*.ifo;*.vob;\0")
_T("其它视频文件(*.amv;*.bik;*.csf;*.*.divx;*.evo;*.ivm;*.mkv;*.mod;*.mts;*.ogm;*.pmp;*.scm;*.tod;*.vp6;*.webm;*.xlmv;)\0*.amv;*.bik;*.csf;*.*.divx;*.evo;*.ivm;*.mkv;*.mod;*.mts;*.ogm;*.pmp;*.scm;*.tod;*.vp6;*.webm;*.xlmv;\0")
_T("其它音频文件(*.aac;*.ac3;*.amr;*.ape;*.cda;*.dts;*.flac;*.mla;*.m2a;*.m4a;*.mid;*.midi;*.mka;*.mp2;*.mp3;*.mpa;*.ogg;*.ra;*.tak;*.tta;*.wav;*.wma;*.wv;)\0*.aac;*.ac3;*.amr;*.ape;*.cda;*.dts;*.flac;*.mla;*.m2a;*.m4a;*.mid;*.midi;*.mka;*.mp2;*.mp3;*.mpa;*.ogg;*.ra;*.tak;*.tta;*.wav;*.wma;*.wv;\0");


const TCHAR STR_SUPPORT_FILE_EXT[] =
_T("*.asf;*.avi;*.wm;*.wmp;*.wmv;*.ram;*.rm;*.rmvb;*.rp;*.rpm;*.rt;*.smi;*.smil;*.dat;*.m1v;*.m2p;*.m2t;*.m2ts;*.m2v;*.mp2v;*.mpeg;*.mpe;*.mpg;*.mpv2;*.pss;*.pva;*.tp;*.tpr;*.ts;*.m4b;*.m4p;*.m4v;*.mp4;*.mpeg4;*.mov;*.qt;*.f4v;*.flv;*.hlv;*.swf;*.ifo;*.vob;*.3g2;*.3gp;*.3gp2;*.3gpp;*.amv;*.bik;*.csf;*.divx;*.evo;*.ivm;*.mkv;*.mod;*.mts; *.ogm; *.pmp; *.scm; *.tod; *.vp6; *.webm; *.xlmv;*.aac; *.ac3; *.amr; *.ape; *.cda; *.dts; *.flac; *.mla; *.m2a; *.m4a; *.mid; *.midi; *.mka; *.mp2; *.mp3; *.mpa; *.ogg; *.ra; *.tak; *.tta; *.wav; *.wma; *.wv;");


//增加文件结构体
struct Thread_add
{
	SListView* plist;
	vector<SStringT> files;
	HWND hwnd;
};

//增加文件线程提高UI响应速度
static DWORD WINAPI threadadd(LPVOID lpParameter)
{
	struct Thread_add* prama1 = (struct Thread_add *)lpParameter;
	SListView  *mclist = prama1->plist;
	vector<SStringT> pfiles = prama1->files;
	HWND phwnd = prama1->hwnd;
	delete prama1;
	CFileListWnd *pAdapter = (CFileListWnd*)mclist->GetAdapter();
	for (vector<SStringT>::iterator it = pfiles.begin(); it != pfiles.end(); ++it)
	{
		SStringT  path_name = *it;
		pAdapter->ADD_files(path_name);
	}
	::PostMessage(phwnd, MS_ADD_FILESED, 0, 0);
	return 0;
}

static unsigned short AMF_DecodeInt16(const char *data)
{
	unsigned char *c = (unsigned char *)data;
	unsigned short val;
	val = (c[0] << 8) | c[1];
	return val;
}

static int BinaryBytes2String(const unsigned char* pSrc, int nSrcLength, char* pDst)
{
	if (pSrc == 0 || pDst == 0)
		return 0;

	const char tab[] = "0123456789abcdef";

	for (int i = 0; i<nSrcLength; i++)
	{
		*pDst++ = tab[*pSrc >> 4];
		*pDst++ = tab[*pSrc & 0x0f];
		pSrc++;
	}

	*pDst = (char)'/0';

	return nSrcLength * 2;
}


//文件文件夹拖入
class CDropTarget : public CTestDropTarget
{
protected:
	SWindow *m_pmanwindow;
	HWND m_hwnd;
	SListView  *mclist;

public:
	CDropTarget(SWindow *pwindow, HWND hwnd, SListView *pmclist) :m_pmanwindow(pwindow), m_hwnd(hwnd), mclist(pmclist)
	{
		if (m_pmanwindow) m_pmanwindow->AddRef();
	}
	~CDropTarget()
	{
		if (m_pmanwindow) m_pmanwindow->Release();
	}
public:
	virtual HRESULT STDMETHODCALLTYPE Drop(
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		FORMATETC format =
		{
			CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
		};
		STGMEDIUM medium;
		if (FAILED(pDataObj->GetData(&format, &medium)))
		{
			return S_FALSE;
		}

		HDROP hdrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));

		if (!hdrop)
		{
			return S_FALSE;
		}

		//////////////////////////////////////////////////////////////////////////
		UINT nFileCount = ::DragQueryFile(hdrop, -1, NULL, 0);

		TCHAR szFileName[_MAX_PATH] = _T("");
		DWORD dwAttribute;
		std::vector<SStringT> vctString;

		// 获取拖拽进来文件和文件夹
		for (UINT i = 0; i < nFileCount; i++)
		{
			::DragQueryFile(hdrop, i, szFileName, sizeof(szFileName));
			dwAttribute = ::GetFileAttributes(szFileName);

			// 是否为文件夹
			if (dwAttribute & FILE_ATTRIBUTE_DIRECTORY)
			{
				::SetCurrentDirectory(szFileName);
				CFileHelp::EnumerateFiles(vctString, STR_SUPPORT_FILE_EXT);
			}
			else
			{
				if (CFileHelp::FindFileExt(szFileName, STR_SUPPORT_FILE_EXT))
					vctString.push_back(szFileName);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		GlobalUnlock(medium.hGlobal);
		if (m_pmanwindow)
		{
			struct Thread_add *prama1 = new Thread_add;
			prama1->files = vctString;
			prama1->plist = mclist;
			prama1->hwnd = m_hwnd;
			HANDLE hThread = CreateThread(NULL, 0, &threadadd, (LPVOID)prama1, 0, 0);
		}
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}
};

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
	m_code = "";
	m_hplayer[0] = NULL;
	m_hplayer[1] = NULL;
	m_hplayer[2] = NULL;
	m_eapilType = 0;
	m_ctrl_down = FALSE;
	m_bOpenPlayList = FALSE;
	m_isplaying = FALSE;
	m_pUserWnd = NULL;
	m_pAddWnd = NULL;
	m_isLogin = FALSE;
	m_aiplay = FALSE;
	m_otherWidth = APPWND_LEFT_WIDTH + 2;
	m_otherHeight = APPWND_TOP_HEIGHT + APPWND_TOOLS_HEIGHT + APPWND_BORDER + 1;
}

CMainDlg::~CMainDlg()
{
	delete m_pUserWnd;
	delete m_pAddWnd;
	m_WorkQueue.Destroy();
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	m_WorkQueue.Create(NUMBEROFTHREADS);

	SRealWnd  *p_RealWnd = FindChildByName2<SRealWnd>(L"ffplaywnd");
	m_playHwnd[0] = p_RealWnd->GetRealHwnd();
	::SendMessageW(m_playHwnd[0], MS_INIT_REALWND, 0, (LPARAM)(void *)this);

	//SRealWnd  *d_RealWnd = FindChildByName2<SRealWnd>(L"ffplaywnd_dev");
	//m_playHwnd[1] = d_RealWnd->GetRealHwnd();
	//::SendMessageW(m_playHwnd[1], MS_INIT_REALWND, 0, (LPARAM)(void *)this);

	//SRealWnd  *f_RealWnd = FindChildByName2<SRealWnd>(L"ffplaywnd_file");
	//m_playHwnd[2] = f_RealWnd->GetRealHwnd();
	//::SendMessageW(m_playHwnd[2], MS_INIT_REALWND, 0, (LPARAM)(void *)this);

	InitSetEvent();
	SetDeviceAdapter();
	//设置为磁吸主窗口
	SetMainWnd(m_hWnd);

	m_file_List_Wnd = FindChildByName2<SListView>(L"file_list");
	if (m_file_List_Wnd)
	{
		HRESULT hr = ::RegisterDragDrop(m_hWnd, GetDropTarget());
		RegisterDragDrop(m_file_List_Wnd->GetSwnd(), new CDropTarget(m_file_List_Wnd, m_hWnd, m_file_List_Wnd));	//注册拖动
		CFileListWnd *pTvAdapter = new CFileListWnd(m_hWnd);
		m_file_List_Wnd->SetAdapter(pTvAdapter);
		pTvAdapter->Release();
	}

	//SWindow *pWndRgn = FindChildByName(L"user_link");
	//if (pWndRgn)
	//{
	//	CRect rc = pWndRgn->GetWindowRect();
	//	rc.MoveToXY(0, 0);
	//	HRGN hRgn = ::CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, rc.CenterPoint().x, rc.CenterPoint().y);

	//	CAutoRefPtr<IRegion> pRgn;
	//	GETRENDERFACTORY->CreateRegion(&pRgn);
	//	pRgn->SetRgn(hRgn);
	//	pWndRgn->SetWindowRgn(pRgn, TRUE);

	//	DeleteObject(hRgn);
	//}

	return 0;
	m_Sliderbarpos = FindChildByName2<SSliderBar>(L"sliderbarpos");
	if (m_Sliderbarpos)
	{
		m_Sliderbarpos->SetRange(0, 1000);
	}

	return 0;
}
//TODO:消息映射
void CMainDlg::OnClose()
{
	RELEASEPLAYER(m_hplayer[0]);
	RELEASEPLAYER(m_hplayer[1]);
	RELEASEPLAYER(m_hplayer[2]);
	CSimpleWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;

	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if (!pBtnMax || !pBtnRestore) return;

	if (nType != SIZE_MINIMIZED) {
		m_rtClient = size;
		for (int i = 0; i < 3; i++)
		{
			if (m_hplayer[i])
			{
				player_setrect(m_hplayer[i], 0, 0, 0, size.cx - m_otherWidth, size.cy - m_otherHeight);
				player_setrect(m_hplayer[i], 1, 0, 0, size.cx - m_otherWidth, size.cy - m_otherHeight);
			}
		}

	}
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

void CMainDlg::InitSetEvent()
{
	STabCtrl *pTabmain = FindChildByName2<STabCtrl>(L"tab_main");
	pTabmain->GetEventSet()->subscribeEvent(EventTabSelChanged::EventID, Subscriber(&CMainDlg::OnListenTabSelChangePage, this));

	SComboView *pComboView = FindChildByName2<SComboView>(L"cbv_iplist");
	pComboView->GetEventSet()->subscribeEvent(EventCBDropdown::EventID, Subscriber(&CMainDlg::OnListenIPDropdownBox, this));

}

int CMainDlg::GetLocalIPInfo(SArray<NetInfo> &Info)
{
	//PIP_ADAPTER_INFO结构体指针存储本机网卡信息
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//得到结构体大小,用于GetAdaptersInfo参数
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	//记录网卡数量
	int netCardNum = 0;
	//记录每张网卡上的IP地址数量
	int IPnumPerNetCard = 0;

	NetInfo ui;

	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//如果函数返回的是ERROR_BUFFER_OVERFLOW
		//则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
		//这也是说明为什么stSize既是一个输入量也是一个输出量
		//释放原来的内存空间
		delete pIpAdapterInfo;
		//重新申请内存空间用来存储所有网卡信息
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}
	if (ERROR_SUCCESS == nRel)
	{
		Info.RemoveAll();
		//输出网卡信息
		//可能有多网卡,因此通过循环去判断
		while (pIpAdapterInfo)
		{
			//cout << "网卡数量：" << ++netCardNum << endl;
			//cout << "网卡描述：" << pIpAdapterInfo->Description << endl;
			ui.strName = S_CA2T(pIpAdapterInfo->Description);
			ui.strAdapterName = S_CA2T(pIpAdapterInfo->AdapterName);
			//cout << "网卡IP地址如下：" << endl;

			//可能网卡有多IP,因此通过循环去判断
			IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
			do
			{
				//cout << "该网卡上的IP数量：" << ++IPnumPerNetCard << endl;
				//cout << "IP 地址：" << pIpAddrString->IpAddress.String << endl;			
				if (strcmp(pIpAdapterInfo->GatewayList.IpAddress.String, "0.0.0.0") == 0)
					break;
				ui.dhcpEnabled = pIpAdapterInfo->DhcpEnabled;
				ui.strIp = S_CA2T(pIpAddrString->IpAddress.String);
				ui.strMask = pIpAddrString->IpMask.String;
				ui.strGateway = pIpAdapterInfo->GatewayList.IpAddress.String;
				ui.strDns = pIpAdapterInfo->DhcpServer.IpAddress.String;
				Info.Add(ui);
				pIpAddrString = pIpAddrString->Next;
			} while (pIpAddrString);
			pIpAdapterInfo = pIpAdapterInfo->Next;
		}
	}

	//释放内存空间
	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}
	return 0;
}

bool CMainDlg::OnListenTabSelChangePage(EventArgs *pEvtBase)
{
	static bool sel_first = true;;
	static bool sel_sencond = true;

	int sel = sobj_cast<STabCtrl>(pEvtBase->sender)->GetCurSel();;
	switch (sel)
	{
	case 1:
	{
		if (sel_first)
		{
			SRealWnd  *d_RealWnd = FindChildByName2<SRealWnd>(L"ffplaywnd_dev");
			m_playHwnd[1] = d_RealWnd->GetRealHwnd();
			::SendMessageW(m_playHwnd[1], MS_INIT_REALWND, 0, (LPARAM)(void *)this);
			sel_first = false;
		}

	}
	break;
	case 2:
	{
		if (sel_sencond)
		{
			SRealWnd  *f_RealWnd = FindChildByName2<SRealWnd>(L"ffplaywnd_file");
			m_playHwnd[2] = f_RealWnd->GetRealHwnd();
			::SendMessageW(m_playHwnd[2], MS_INIT_REALWND, 0, (LPARAM)(void *)this);
			sel_sencond = false;
		}

	}
	break;
	default:
		break;
	}
	return true;
}

bool CMainDlg::OnListenIPDropdownBox(EventArgs *pEvtBase)
{
	SArray<NetInfo> netInfo;
	GetLocalIPInfo(netInfo);
	SComboView *pComboView = sobj_cast<SComboView>(pEvtBase->sender);
	if (pComboView)
	{
		SListView *pLstView = pComboView->GetListView();
		CFixListWnd* pAdapter = new CFixListWnd(netInfo);
		pLstView->SetAdapter(pAdapter);
		pAdapter->Release();
		pComboView->SetCurSel(-1);
	}
	return true;
}

void CMainDlg::OnPlayProgress()
{
	SetTimer(TIMER_ID_PLAYING_PROGRESS, 1000);
}

void CMainDlg::PlayLive(const char *url)
{
	SRealWnd  *p_RealWnd = FindChildByName2<SRealWnd>(L"ffplaywnd");
	if (p_RealWnd)
	{
		unsigned char temp[1024] = { 0 };
		char templet[1024] = { 0 };
		int temp_len = m_cameraList.GetAt(m_playcamera_index).panoTemplate.size();
		PLAYER_INIT_PARAMS	Params;

		memset(&Params, 0, sizeof(Params));
		Params.adev_render_type = ADEV_RENDER_TYPE_WAVEOUT;
		Params.vdev_render_type = VDEV_RENDER_TYPE_GDI;
		Params.init_timeout = 20000;

		if (temp_len > 1)
		{
			// load fanplayer init params
			Params.vdev_render_type = VDEV_RENDER_TYPE_EAPIL;
			memcpy(Params.eapil_template, m_cameraList.GetAt(m_playcamera_index).panoTemplate.c_str(), temp_len);
		}
		RELEASEPLAYER(m_hplayer[0]);
		m_hplayer[0] = player_open((char *)url, m_playHwnd[0], &Params);

		//::SendMessageW(m_playHwnd, MS_OPENVIDEO_REALWND, 0, (LPARAM)(void *)this);
	}
}

void CMainDlg::PlayLan(const char *url)
{
	SRealWnd  *p_RealWnd = FindChildByName2<SRealWnd>(L"ffplaywnd_dev");
	if (p_RealWnd)
	{
		unsigned char temp[1024] = { 0 };
		char templet[1024] = { 0 };
		PLAYER_INIT_PARAMS	Params;

		memset(&Params, 0, sizeof(Params));
		Params.adev_render_type = ADEV_RENDER_TYPE_WAVEOUT;
		Params.vdev_render_type = VDEV_RENDER_TYPE_GDI;
		Params.init_timeout = 10000;
		//if (voiceType)
		//{
		//	Params.audio_stream_cur = -1;
		//	Params.low_delay = 1;
		//}
		RELEASEPLAYER(m_hplayer[1]);
		m_hplayer[1] = player_open((char *)url, m_playHwnd[1], &Params);

		//::SendMessageW(m_playHwnd, MS_OPENVIDEO_REALWND, 0, (LPARAM)(void *)this);
	}
}

void CMainDlg::PlayFile(const char *url)
{
	SRealWnd  *p_RealWnd = FindChildByName2<SRealWnd>(L"ffplaywnd");
	if (p_RealWnd)
	{
		unsigned char temp[1024] = { 0 };
		char templet[1024] = { 0 };
		int temp_len = 0;
		PLAYER_INIT_PARAMS	Params;
		memset(&Params, 0, sizeof(Params));
		Params.adev_render_type = ADEV_RENDER_TYPE_WAVEOUT;
		Params.vdev_render_type = VDEV_RENDER_TYPE_GDI;
		Params.init_timeout = 5000;

		temp_len = irmFlvReadFileTemplate((char *)url, temp);
		if (temp_len > 1)
		{
			temp_len = BinaryBytes2String(temp, temp_len, templet);
			// load fanplayer init params
			Params.vdev_render_type = VDEV_RENDER_TYPE_EAPIL;
			memcpy(Params.eapil_template, templet, temp_len);
		}
		RELEASEPLAYER(m_hplayer[2]);
		m_hplayer[2] = player_open((char *)url, m_playHwnd[2], &Params);

		::SendMessageW(m_playHwnd[2], MS_OPENVIDEO_REALWND, 0, (LPARAM)(void *)this);
	}
}

//增加文件完成后的通知 如果有文件则显示列表
LRESULT CMainDlg::OnMsg_ADD_FILED(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	CFileListWnd *pAdapter = (CFileListWnd*)m_file_List_Wnd->GetAdapter();
	if (pAdapter->getCount()>0)
	{
		::PostMessage(m_playHwnd[2], WM_ADD_FILESED, 0, 0);
		//SWindow   *playlist_wnd = FindChildByID(8000);
		//if (!playlist_wnd->IsVisible())
		//{
		//	playlist_wnd->SetVisible(true, true);
		//}
	}
	pAdapter->notifyDataSetChanged();
	return 0;
}

LRESULT CMainDlg::OnMsg_QUIT_LOGIN(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	string data = "method=logout&access_token=";
	data.append(m_loginInfo.token);
	SendCMD(OPT_QUIT_LOGIN, m_hWnd, POST, GETUSERINFO_URL, data);

	return 0;
}

BOOL CMainDlg::IsFileExist(const SStringT& csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

//双击列表通知播放文件
LRESULT CMainDlg::OnMsg_PLAY_FILE(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	PlayFile((char *)lp);
	return 0;
}

//接受键盘输入
void CMainDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_CONTROL:
		m_ctrl_down = TRUE;
		break;
	}
}

void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
	int64_t pos = 0;
	int64_t total = 1;
	int value = 0;
	SetMsgHandled(FALSE);
	switch (nIDEvent)
	{
	case TIMER_ID_PLAYING_PROGRESS:
		KillTimer(TIMER_ID_PLAYING_PROGRESS);
		//if (!m_hplayer)
		//{
		//	KillTimer(TIMER_ID_PLAYING_PROGRESS);
		//	break;
		//}
		//player_getparam(m_hplayer, PARAM_MEDIA_DURATION, &total);
		//player_getparam(m_hplayer, PARAM_MEDIA_POSITION, &pos);
		//value = (int)(1000 * pos / total);
		//m_Sliderbarpos->SetValue(value);

		break;
	}
}

void CMainDlg::OnBtnOpen()	//打开文件
{
	vector<SStringT> names;
	CFileHelp::OpenFile(STR_MOVE_FILE_FILTER, GetHostHwnd(), names);
	if (names.empty()) return;
	struct Thread_add *prama1 = new Thread_add;
	prama1->files = names;
	prama1->plist = m_file_List_Wnd;
	prama1->hwnd = m_hWnd;
	HANDLE hThread = CreateThread(NULL, 0, &threadadd, (LPVOID)prama1, 0, 0);
	SWindow   *playlist_wnd = FindChildByID(8000);
	if (!playlist_wnd->IsVisible())
	{
		playlist_wnd->SetVisible(true, true);
	}
}

void CMainDlg::OnPlaySwitchPause()
{
	SImageButton *pbtnPlay = FindChildByID2<SImageButton>(200);
	SImageButton *pbtnPause = FindChildByID2<SImageButton>(201);
	if (pbtnPlay && pbtnPause)
	{
		int status = 0;
		player_getparam(m_hplayer[2], PARAM_PLAYER_STATUS, &status);

		pbtnPlay->SetVisible(!m_isplaying, TRUE);
		pbtnPause->SetVisible(m_isplaying, TRUE);
	}
}

void CMainDlg::OnBtnPlay()
{
	int status = 0;
	CFileListWnd *pAdapter = (CFileListWnd*)m_file_List_Wnd->GetAdapter();
	int m_items = m_file_List_Wnd->GetSel();
	if (m_items < 0) return;

	string path = S_CT2A(pAdapter->Get_index_Path(m_items));
	player_getparam(m_hplayer[2], PARAM_PLAYER_STATUS, &status);
	if (status >> 2 & 1)
	{
		player_play(m_hplayer[2]);
		m_isplaying = TRUE;
		OnPlaySwitchPause();
	}
	else
	{
		m_Sliderbarpos->SetValue(0);
		PlayFile(path.c_str());
	}
}

void CMainDlg::OnBtnPause()
{
	player_pause(m_hplayer[2]);
	m_isplaying = FALSE;
	OnPlaySwitchPause();
}

void CMainDlg::OnBtnStop()
{
	RELEASEPLAYER(m_hplayer[2]);
	m_isplaying = FALSE;
	OnPlaySwitchPause();
}

void CMainDlg::OnPlayList()
{
	m_bOpenPlayList ? m_otherWidth = 0 : m_otherWidth = 260;
	player_setrect(m_hplayer[2], 0, 0, 0, m_rtClient.cx - m_otherWidth, m_rtClient.cy - m_otherHeight);
	player_setrect(m_hplayer[2], 1, 0, 0, m_rtClient.cx - m_otherWidth, m_rtClient.cy - m_otherHeight);

	SWindow  *wnd = FindChildByID2<SWindow>(8000);
	wnd->SetVisible(!wnd->IsVisible(), TRUE);
	m_bOpenPlayList = !m_bOpenPlayList;
}

void CMainDlg::OnEapilType()
{
	int type = 0;
	player_getparam(m_hplayer[2], PARAM_PLAYER_RENDER_TYPE, &type);
	if (type != VDEV_RENDER_TYPE_EAPIL) return;

	m_eapilType++;
	if (m_eapilType >= 4) m_eapilType = 0;
	player_setrendertype(m_hplayer[2], m_eapilType);

}

//删除列表文件
void CMainDlg::OnDellfiles_MenuBtn()
{
	CFileListWnd *pAdapter = (CFileListWnd*)m_file_List_Wnd->GetAdapter();
	int m_items = m_file_List_Wnd->GetSel();
	if (m_items < 0) return;
	if (m_items == pAdapter->getCount() - 1 && pAdapter->getCount()>1)
	{
		pAdapter->Dll_File(m_items);
		m_file_List_Wnd->SetSel(m_items - 1);
	}
	else
		pAdapter->Dll_File(m_items);
}

//增加文件
void CMainDlg::OnAddfiles_MenuBtn()
{
	OnBtnOpen();
}

void CMainDlg::OnLButtonUp(UINT nFlags, CPoint pt)//处理进度条鼠标事件 播放指定位置 此处没有用EventSliderPos事件
{
	//if (m_Sliderbarpos->GetWindowRect().PtInRect(pt) && m_LButtonDown == 1)
	//{
	//	LONGLONG inter = 0;
	//	LONGLONG total = 1;
	//	player_getparam(m_hplayer, PARAM_MEDIA_DURATION, &total);
	//	inter = total / 1000;
	//	player_seek(m_hplayer, (m_Sliderbarpos->GetValue()*inter) + 1, 0);
	//}
	//m_LButtonDown = 0;
	SetMsgHandled(false);
}

void CMainDlg::OnLButtonDown(UINT nFlags, CPoint pt)//处理进度条鼠标事件
{
	//if (m_Sliderbarpos->GetWindowRect().PtInRect(pt))//处理进度条
	//	m_LButtonDown = 1;

	SetMsgHandled(false);

}

//-182, 73
void CMainDlg::OnMouseMove(UINT nFlags, CPoint pt)
{
	//if (m_VolumeSlider->GetWindowRect().PtInRect(pt) && m_LButtonDown == 1 && !m_voiceType)
	//{
	//	int volume = m_VolumeSlider->GetValue();
	//	switch (volume)
	//	{
	//	case -182:
	//		OnVolume();
	//		break;
	//	default:
	//		OnVolumeZero();
	//		break;
	//	}
	//}
	SetMsgHandled(false);
}

BOOL CMainDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	if (m_hplayer[0])
	{
		player_mousewheel(m_hplayer[0], zDelta);
	}

	if (m_hplayer[2])
	{
		player_mousewheel(m_hplayer[2], zDelta);
	}
	SetMsgHandled(false);
	return TRUE;
}

int CMainDlg::irmFlvReadFileTemplate(char *FlvFile, unsigned char *FileLink)
{
	char tbuff[1024], *tp = tbuff;
	FILE* tfd = NULL;
	int tlen = 1024;
	unsigned short len = 0;
	tfd = fopen(FlvFile, "rb");
	if (tfd >0)
	{
		tlen = fread(tbuff, 1, tlen, tfd);
		fclose(tfd);
		while (tlen > 0)
		{
			if (!memcmp(tp, "Templet", 7))
				break;
			tp++;
			tlen--;
		}

		if (tlen > 0)
		{
			len = AMF_DecodeInt16(tp + 8);
			memcpy(FileLink, tp + 10, len);
			return len;
		}
	}

	return -1;
}

//获取摄像机的在线状态
CameraStatus CMainDlg::GetCameraStatus(int liveStatus)
{
	CameraStatus status = TYPE_STATUS_OFFLINE;

	if (liveStatus == 0) {
		//离线
		status = TYPE_STATUS_OFFLINE;
	}
	else if ((liveStatus > 0) && (liveStatus & 4) == 0) {
		//关机
		status = TYPE_STATUS_POWEROFF;
	}
	else if ((liveStatus> 0) && ((liveStatus & 4) == 4)) {
		//开机
		status = TYPE_STATUS_ONLINE;
	}

	return status;
}

//摄像机在线状态比较
SStringT CMainDlg::GetCameraCmpTag(CameraStatus status)
{
	SStringT tag = L"3";

	switch (status)
	{
	case TYPE_STATUS_OFFLINE:
		tag = L"3";
		break;
	case TYPE_STATUS_POWEROFF:
		tag = L"2";
		break;
	case TYPE_STATUS_ONLINE:
		tag = L"1";
		break;
	}

	return tag;
}

void CMainDlg::OnUserInfoDialog()
{
	if (!m_isLogin)
	{
		::PostMessage(m_playHwnd[0], MS_OPEN_LOGIN, 0, 0);
		return;
	}

	if (m_pUserWnd)
	{
		if (!m_pUserWnd->IsVisible())
			m_pUserWnd->SetVisible(TRUE, TRUE);
	}
	else
	{
		m_pUserWnd = new UserPopupWnd(_T("LAYOUT:XML_USERWND"), m_hWnd);
		m_pUserWnd->Create(m_hWnd, WS_POPUP, 0, 0, 0, 0, 0);

		//选择一种吸附模式
		CMagnetFrame::ATTACHMODE am = AM_CENTER;
		CMagnetFrame::ATTACHALIGN aa = AA_CENTER;

		AddSubWnd(m_pUserWnd->m_hWnd, am, aa);
		m_pUserWnd->ShowWindow(SW_SHOW);
	}

	SStringT avatarPath;
	avatarPath.Format(_T("%s%s.jpg"), LAVATAR_PATH, S_CA2T(m_loginInfo.user.c_str()));

	if (IsFileExist(avatarPath))
	{
		SImageMaskWnd *avatar = m_pUserWnd->FindChildByName2<SImageMaskWnd>(L"user_avatar");
		avatar->SetSkinFormFile(avatarPath);
	}

	m_pUserWnd->FindChildByName2<SStatic>(L"user_name")->SetWindowTextW(m_userInfo.username);
	m_pUserWnd->FindChildByName2<SStatic>(L"name")->SetWindowTextW(m_userInfo.username);
	m_pUserWnd->FindChildByName2<SStatic>(L"email")->SetWindowTextW(m_userInfo.email);
	m_pUserWnd->FindChildByName2<SStatic>(L"phone")->SetWindowTextW(m_userInfo.phone);
	m_pUserWnd->FindChildByName2<SStatic>(L"bd_account")->SetWindowTextW(m_userInfo.bd_account);

}

void CMainDlg::OnAddCameraDialog()
{
	if (m_pAddWnd)
	{
		if (!m_pAddWnd->IsVisible())
			m_pAddWnd->SetVisible(TRUE, TRUE);
	}
	else
	{
		m_pAddWnd = new AddPopupWnd(_T("LAYOUT:XML_ADDWND"), this);
		m_pAddWnd->Create(m_hWnd, WS_POPUP, 0, 0, 0, 0, 0);

		//选择一种吸附模式
		CMagnetFrame::ATTACHMODE am = AM_CENTER;
		CMagnetFrame::ATTACHALIGN aa = AA_CENTER;

		AddSubWnd(m_pAddWnd->m_hWnd, am, aa);
		m_pAddWnd->ShowWindow(SW_SHOW);
	}
	STabCtrl *pTab = m_pAddWnd->FindChildByName2<STabCtrl>(L"add_tab");
	pTab->SetCurSel(_T("add_first_page"));
}

bool CMainDlg::CheckIp(int type, LPCWSTR pszName, LPCWSTR tipName, string &ip)
{
	int a, b, c, d;
	SStringT cameraIp;
	if (type)
	{
		SComboView *pComboView = FindChildByName2<SComboView>(L"cbv_iplist");
		if (!pComboView)  return false;
		cameraIp = pComboView->GetWindowTextW();
	}
	else
	{
		SEdit *edit_cameraIp = FindChildByName2<SEdit>(pszName);
		if (!edit_cameraIp)  return false;
		cameraIp = edit_cameraIp->GetWindowTextW();

	}
	if (!cameraIp.GetLength() && tipName != NULL)
	{
		SMessageBox(NULL, tipName, _T("提示"), MB_OK | MB_ICONERROR);
		return false;
	}
	if (!((swscanf(cameraIp, L"%d.%d.%d.%d", &a, &b, &c, &d) == 4) && (a >= 0 && a <= 255)
		&& (b >= 0 && b <= 255)
		&& (c >= 0 && c <= 255)
		&& (d >= 0 && d <= 255)))
	{
		SMessageBox(NULL, _T("ip格式不正确，请输入正确的ipv4"), _T("提示"), MB_OK | MB_ICONERROR);
		return false;
	}
	ip = S_CT2A(cameraIp);

	return true;
}

void CMainDlg::GetDeviceList()
{
	string localIp;
	if (CheckIp(1, L"edit_localIp", _T("请输入本机IP"), localIp))
	{
		SendCMD(OPT_GETCAMERA_LIST, m_playHwnd[1], POST, localIp, "");
	}
}

void CMainDlg::SetCameraAdapter()
{
	SListView* camera_List_Wnd = FindChildByName2<SListView>("camera_list");
	if (camera_List_Wnd)
	{
		CCameraListWnd *cameraAdapter = new CCameraListWnd(m_hWnd);
		cameraAdapter->ADD_Tags(this, &m_cameraList);
		camera_List_Wnd->SetAdapter(cameraAdapter);
		cameraAdapter->Release();
	}
	//SetRefreshDisplay(refdisplay);
}

void CMainDlg::RefCameraAdapterView()
{
	SListView* camera_List_Wnd = FindChildByName2<SListView>("camera_list");
	if (camera_List_Wnd)
	{
		CCameraListWnd *cameraAdapter = (CCameraListWnd *)camera_List_Wnd->GetAdapter();
		cameraAdapter->ADD_Tags(this, &m_cameraList);
	}
}

void CMainDlg::SetDeviceAdapter()
{
	SListView* device_List_Wnd = FindChildByName2<SListView>("device_list");
	if (device_List_Wnd)
	{
		CDeviceListWnd *deviceAdapter = new CDeviceListWnd(m_hWnd);
		deviceAdapter->ADD_Tags(this, &m_devicelist);
		device_List_Wnd->SetAdapter(deviceAdapter);
		deviceAdapter->Release();
	}
}

void CMainDlg::RefDeviceAdapterView()
{
	SListView* device_List_Wnd = FindChildByName2<SListView>("device_list");
	if (device_List_Wnd)
	{
		CDeviceListWnd *deviceAdapter = (CDeviceListWnd *)device_List_Wnd->GetAdapter();
		deviceAdapter->ADD_Tags(this, &m_devicelist);
	}
}

BOOL CMainDlg::LoadAvatar()
{
	SStringT avatarPath;
	avatarPath.Format(_T("%s%s.jpg"), LAVATAR_PATH, S_CA2T(m_loginInfo.user.c_str()));

	if (IsFileExist(avatarPath))
	{
		FindChildByName2<SImageMaskWnd>(L"user_link")->SetSkinFormFile(avatarPath);
		return TRUE;
	}
	return FALSE;

}

//自动登录直接读取数据库设备列表
void CMainDlg::loadDeviceInfo()
{
	CppSQLite3DB db;
	try
	{
		char *sql = "CREATE TABLE if not exists deviceTable(  \
						uid					varchar(255)	 NOT NULL, \
						deviceid			varchar(255)	 NOT NULL,\
						shareid				varchar(255), \
						uk					varchar(255), \
						description			TEXT, \
						share				integer, \
						status				integer, \
						thumbnail			varchar(255), \
						data_type			integer,\
						connect_type		integer, \
						stream_id			varchar(255), \
						cvr_day				varchar(255), \
						cvr_end_time		varchar(255), \
						avatar				varchar(255), \
						username			varchar(255), \
						viewnum				integer, \
						approvenum			integer, \
						subscribe			integer, \
						grantnum			integer, \
						force_upgrade		integer, \
						need_upgrade		integer, \
						device_type			integer, \
						cvr_type			integer, \
						cvr_free			integer, \
						cid					TEXT, \
						intro				varchar(255), \
						commentnum			integer, \
						location			TEXT, \
						showlocation		integer, \
						needpassword		BOOL, \
						share_end_time		DOUBLE, \
						share_expires_in	DOUBLE, \
						timezone			varchar(255), \
						reportstatus		integer, \
						pano_config			varchar(255), \
						PRIMARY KEY(uid, deviceid, data_type));";

		db.open(IERMU_DB);
		db.execDML(sql);

	}
	catch (CppSQLite3Exception& e){
		printf("%s", e.errorMessage());
	}

	try
	{
		int row = 0;
		char queSql[128] = { 0 };

		sprintf(queSql, "SELECT deviceid, thumbnail, description, status, connect_type, pano_config FROM %s WHERE uid=\'%s\'", DEVICETABLE, m_loginInfo.uid.c_str());
		CppSQLite3Table query = db.getTable(queSql);  //执行查询 	
		row = query.numRows();
		CameraItem item;
		for (int i = 0; i < row; i++)
		{
			string temp;
			query.setRow(i);

			item.deviceid = query.fieldValue("deviceid");
			item.thumbnail = query.fieldValue("thumbnail");
			item.description = query.fieldValue("description");
			item.con_type = atoi(query.fieldValue("connect_type"));
			item.status = GetCameraStatus(query.getIntField("status"));
			if (query.fieldValue("pano_config"))
				item.panoTemplate = query.fieldValue("pano_config");
			item.tag = GetCameraCmpTag(item.status);
			m_cameraList.Add(item);
			//SLOGFMTE("deviceid : %s   status: %d ", item.deviceid.c_str(), query.getIntField("status"));
		}

		query.finalize();
	}
	catch (CppSQLite3Exception& e){
		printf("%s", e.errorMessage());
	}

	SetCameraAdapter();
	db.close();
}

//保存到设备列表数据库
void CMainDlg::saveDeviceInfo(string &response)
{
	CppSQLite3DB db;
	Json::Reader reader;
	Json::Value jsonobj;

	if (!reader.parse(response, jsonobj))
		return;

	if (!jsonobj["list"].isNull())
	{
		CameraItem item;
		int list_size = jsonobj["list"].size();
		if (list_size > 0)
		{
			m_cameraList.RemoveAll();
			db.open(IERMU_DB);
		}
					
		for (int i = 0; i < list_size; i++)
		{	
			char sql[4096] = { 0 };
			try
			{
				item.deviceid = jsonobj["list"][i]["deviceid"].asString();
				item.thumbnail = jsonobj["list"][i]["thumbnail"].asString();
				item.description = jsonobj["list"][i]["description"].asString();
				item.con_type = atoi(jsonobj["list"][i]["connect_type"].asString().c_str());
				item.status = GetCameraStatus(atoi(jsonobj["list"][i]["status"].asString().c_str()));
				item.panoTemplate = jsonobj["list"][i]["pano_config"].asString();
				item.tag = GetCameraCmpTag(item.status);
				m_cameraList.Add(item);

				sprintf(sql, "REPLACE INTO %s \
							 (uid, deviceid, shareid, uk \
							 ,description, share, status \
							 ,thumbnail, data_type, connect_type \
							 ,stream_id, cvr_day, cvr_end_time \
							 ,avatar, username, viewnum \
							 ,approvenum, subscribe, grantnum \
							 ,force_upgrade, need_upgrade, device_type \
							 ,cvr_type, cvr_free, cid \
							 ,intro, commentnum, location \
							 ,showlocation, needpassword, share_end_time \
							 ,share_expires_in, timezone, reportstatus, pano_config) values \
							 (\'%s\', \'%s\', \'%s\', \'%s\' \
							 ,\'%s\', %d,  %d \
							 ,\'%s\', %d,  %d \
							 ,\'%s\', \'%s\', \'%s\' \
							 ,\'%s\', \'%s\', %d \
							 ,%d,  %d,  %d \
							 ,%d,  %d,  %d \
							 ,%d,  %d, \'%s\' \
							 ,\'%s\', %d, \'%s\' \
							 ,%d, %d, %f \
							 ,%f, \'%s\', %d,  \'%s\')", DEVICETABLE
							 , m_loginInfo.uid.c_str(), item.deviceid.c_str(), jsonobj["list"][i]["shareid"].asString().c_str(), jsonobj["list"][i]["uk"].asString().c_str()
							 , item.description.c_str(), atoi(jsonobj["list"][i]["share"].asString().c_str()), atoi(jsonobj["list"][i]["status"].asString().c_str())
							 , item.thumbnail.c_str(), jsonobj["list"][i]["data_type"].asInt(), atoi(jsonobj["list"][i]["connect_type"].asString().c_str())
							 ,jsonobj["list"][i]["stream_id"].asString().c_str(), jsonobj["list"][i]["cvr_day"].asString().c_str(), jsonobj["list"][i]["cvr_end_time"].asString().c_str()
							 ,jsonobj["list"][i]["avatar"].asString().c_str(), jsonobj["list"][i]["username"].asString().c_str(), atoi(jsonobj["list"][i]["viewnum"].asString().c_str())
							 ,atoi(jsonobj["list"][i]["approvenum"].asString().c_str()), jsonobj["list"][i]["subscribe"].asInt(), atoi(jsonobj["list"][i]["grantnum"].asString().c_str())
							 ,atoi(jsonobj["list"][i]["force_upgrade"].asString().c_str()), atoi(jsonobj["list"][i]["need_upgrade"].asString().c_str()), atoi(jsonobj["list"][i]["device_type"].asString().c_str())
							 ,atoi(jsonobj["list"][i]["cvr_type"].asString().c_str()), jsonobj["list"][i]["cvr_free"].asInt(), "[]"
							 ,jsonobj["list"][i]["intro"].asString().c_str(), atoi(jsonobj["list"][i]["commentnum"].asString().c_str()), ""
							 ,jsonobj["list"][i]["showlocation"].asInt(), jsonobj["list"][i]["needpassword"].asInt(), jsonobj["list"][i]["share_end_time"].asFloat()
							 , jsonobj["list"][i]["share_expires_in"].asFloat(), jsonobj["list"][i]["timezone"].asString().c_str(), 0, item.panoTemplate.c_str());
												
				db.execDML(sql);
			}
			catch (CppSQLite3Exception& e)
			{
				printf("%s", e.errorMessage());
			}
		}

		if (list_size > 0)
		{
			db.close();
		}			
	}
}

//请求获取设备列表信息
void CMainDlg::GetCameraList()
{
	string data = "access_token=";
	data.append(m_loginInfo.token).append("&data_type=my&device_type=1&lang=en&list_type=all&method=list");

	SendCMD(OPT_GETDEVICEINFO, m_hWnd, POST, GETDEVICEINFO_URL, data);
}

//请求获取用户信息
void CMainDlg::GetUserInfoRequest()
{
	string data = "access_token=";
	data.append(m_loginInfo.token).append("&method=info&connect=1");

	SendCMD(OPT_GETUSERINFO, m_hWnd, POST, GETUSERINFO_URL, data);
}


bool CMainDlg::SendCMD(SOCKETOPTION opt, HWND hwnd, REQUEST_TYPE type, string url, string data)
{
	//开启线程发送命令
	SocketData	*socket_data = new SocketData;
	memset(socket_data, 0, sizeof(SocketData));
	socket_data->opt = opt;
	socket_data->hwnd = hwnd;
	socket_data->type = type;
	socket_data->url = url;
	socket_data->data = data;
	LPVOID param = (LPVOID)socket_data;

	return m_WorkQueue.InsertWorkItem(new WinClientTask(this, param));
}

void CMainDlg::TcpRequestTask(LPVOID data)
{
	SocketData *param = (SocketData *)data;
	SocketRetData *pEvt = new SocketRetData;
	memset(pEvt, 0, sizeof(SocketRetData));

	pEvt->opt = param->opt;
	pEvt->hwnd = param->hwnd;
	switch (pEvt->opt)
	{
	case OPT_GETCAMERA_LIST:
		{
			UpnpTool Supnp;
			pEvt->retOK = Supnp.upnpDiscover(3000, param->url, m_devicelist);
			if (!m_devicelist.GetCount()) pEvt->retOK = false;
		}
		break;
	}
	delete param;
	::PostMessage(pEvt->hwnd, WM_TCPREQUEST_TASK, (WPARAM)pEvt, 0);
}

void CMainDlg::HttpRequestTask(LPVOID data)
{
	CHttpConnect WinClient;
	Json::Reader reader;
	Json::Value jsonobj;

	SocketData *param = (SocketData *)data;
	SocketRetData *pEvt = new SocketRetData;
	memset(pEvt, 0, sizeof(SocketRetData));

	pEvt->opt = param->opt;
	pEvt->hwnd = param->hwnd;
	pEvt->hData = WinClient.Request(param->url, param->type, param->data);
	pEvt->retOK = WinClient.GetStatusIsOK();

	//access token invalid or no longer valid
	if (!pEvt->retOK)
	{
		if (!reader.parse(pEvt->hData, jsonobj))
		{
			pEvt->retValue = -1;
			goto end;
		}
		int error_code = jsonobj["error_code"].asInt();
		if (error_code == 110)
		{
			string refdata = "grant_type=refresh_token&client_id=Ba3OkUvTJBzzDhc8yJlW&client_secret=KnPLP84MRtsC9UwUiU1BQUxxQpe2ddMwf8qu3ItU&refresh_token=";
			refdata.append(m_loginInfo.refToken);
			pEvt->hData = WinClient.Request(AUTHORIZATION_URL, POST, refdata);
			pEvt->retOK = WinClient.GetStatusIsOK();
			if (!reader.parse(pEvt->hData, jsonobj))
			{
				pEvt->retValue = -1;
				goto end;
			}
			string token = jsonobj["access_token"].asString();
			int bg = param->data.find("access_token=")+13;
			param->data = param->data.replace(bg, bg + m_loginInfo.token.length(), token);
			m_loginInfo.token = token;
			m_loginInfo.refToken = jsonobj["refresh_token"].asString();

			::PostMessage(m_playHwnd[0], MS_SAVE_TOKEN, 0, 0);
			pEvt->hData = WinClient.Request(param->url, param->type, param->data);
			pEvt->retOK = WinClient.GetStatusIsOK();
		}
	}

	switch (pEvt->opt)
	{
	case OPT_GET_QRCODE:
		if (pEvt->retOK)
		{
			char path[256] = { 0 };
			if (!reader.parse(pEvt->hData, jsonobj))
			{
				pEvt->retValue = -1;
				break;
			}
			m_code = jsonobj["code"].asString();
			string qrcode_url = jsonobj["qrcode_url"].asString();
			sprintf(path, QRCODE_PATH);
			WinClient.Download(qrcode_url, path);
		}
		break;
	case OPT_QRCODE_STATUS:
		if (pEvt->retOK)
		{
			if (!reader.parse(pEvt->hData, jsonobj))
			{
				pEvt->retValue = -1;
				break;
			}

			int status = jsonobj["status"].asInt();
			switch (status)
			{
			case 0:
			case 1:
				pEvt->retValue = 0;
				break;
			case 2:
				pEvt->retValue = 1;
				{
					string data = "grant_type=qrcode";
					data.append("&code=").append(m_code).append("&client_id=Ba3OkUvTJBzzDhc8yJlW&client_secret=KnPLP84MRtsC9UwUiU1BQUxxQpe2ddMwf8qu3ItU&scope=basic");
					pEvt->hData = WinClient.Request(AUTHORIZATION_URL, POST, data);
					pEvt->retOK = WinClient.GetStatusIsOK();
				}
				break;
			case -1:
			case -2:
			case -3:
			case -4:
				pEvt->retValue = -1;
				break;
			}
		}
		break;
	case OPT_GETDEVICEINFO:
		if (pEvt->retOK)
		{
			saveDeviceInfo(pEvt->hData);
		}
		break;
	case OPT_GETUSERINFO:
		if (pEvt->retOK)
		{
			char path[256] = { 0 };
			if (!reader.parse(pEvt->hData, jsonobj))
			{
				pEvt->retValue = -1;
				break;
			}
			string avatar_url = jsonobj["avatar"].asString();
			SStringA username = S_CA2A(jsonobj["username"].asString().c_str());
			m_userInfo.username = S_CA2T(username);
			m_userInfo.email = S_CA2T(jsonobj["email"].asString().c_str());
			m_userInfo.phone = S_CA2T(jsonobj["mobile"].asString().c_str());
			const Json::Value connect = jsonobj["connect"];
			SStringA bd_account = connect[0]["username"].asString().c_str();
			m_userInfo.bd_account = S_CA2T(bd_account);

			sprintf(path, "%s%s.jpg", AVATAR_PATH, m_loginInfo.user.c_str());
			WinClient.Download(avatar_url, path);
		}
		break;
	}

end:
	delete param;
	::PostMessage(pEvt->hwnd, WM_WINREQUEST_TASK, (WPARAM)pEvt, 0);
}

LRESULT CMainDlg::OnMsg_HTTP_TASK(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
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

		break;
	case OPT_GETDEVICEINFO:
		if (pEvt->retOK)
		{
			RefCameraAdapterView();
			//GetThumbnailImg();

		}
		break;

	case OPT_GETPLAYURL:
		if (pEvt->retOK)
		{
			int stat = atoi(jsonobj["status"].asString().c_str());
			if (!(stat > 0 && (stat & 4) == 4))
			{
				SMessageBox(NULL, _T("设备不在线，播放失败！"), _T("提示"), MB_OK | MB_ICONERROR);
				return false;
			}
			string url = jsonobj["url"].asString();
			if (url.empty())
			{
				SMessageBox(NULL, _T("数据异常，获取播放地址失败！"), _T("提示"), MB_OK | MB_ICONERROR);
				return false;
			}
			PlayLive(url.c_str());
		}
		break;
	case OPT_GETUSERINFO:
		if (pEvt->retOK)
		{
			LoadAvatar();
		}
		break;
	case OPT_QUIT_LOGIN:
		if (pEvt->retOK)
		{
			m_isLogin = FALSE;
			m_cameraList.RemoveAll();
			RefCameraAdapterView();
			::PostMessage(m_playHwnd[0], MS_OPEN_TIPPAGE, 0, 0);
		}
		
		break;
	default:
		break;

	}

	delete pEvt;
	return true;
}