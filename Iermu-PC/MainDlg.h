// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <iostream>
#include <iomanip>
#include "threadObject.h"
#include "WorkQueue.h"
#include "WinSocketClient.h"
#include "UpnpTool.h"
#include "magnetframe.h"
#include "UserPopupWnd.h"
#include "AddPopupWnd.h"
#include "SImageMaskWnd.h"
#include "ffplayer.h"

//网卡信息
typedef struct{
	SStringT strName;			//网卡名
	SStringT strIp;				//IP地址
	SStringT strAdapterName;	//网卡物理地址
	string	 strMask;			//掩码
	string	 strGateway;		//网关
	string   strDns;			//dns
	int		 dhcpEnabled;		//是否自动获取ip
}NetInfo;

//摄像机状态
typedef enum
{
	TYPE_STATUS_OFFLINE,
	TYPE_STATUS_POWEROFF,
	TYPE_STATUS_ONLINE
}CameraStatus;

//摄像机列表信息
typedef struct
{
	SStringT		tag;
	CameraStatus	status;			//摄像机状态 在线离线关机
	int				con_type;		//摄像机平台 羚羊或百度
	string			deviceid;
	string			thumbnail;		//封面图片
	string			description;
	string			panoTemplate;	//全景模板
}CameraItem;

//登录后保存的用户密码token等信息
typedef struct
{
	int				type;				// 0:不勾选 1：保存密码 2:自动登录 3：保存密码和自动登录
	string          user;
	string          passwd;
	string			uid;				// 登录后获取的uid
	string			token;				// 登录后获取的token
	string          refToken;			// 刷新的token
}LoginInfo;

//用户个人信息
typedef struct
{
	SStringT username;
	SStringT phone;
	SStringT email;
	SStringT bd_account;
}UserInfo;


class CMainDlg : public SHostWnd
				,public CMagnetFrame	//磁力吸附
				//, public CThreadObject	//线程对象
				//, public TAutoEventMapReg<CMainDlg>	//通知中心自动注册
{
public:
	CMainDlg();
	~CMainDlg();

	void*			m_hplayer[3];		// 播放句柄0：外网 1：内网 2：文件 
	BOOL            m_bOpenPlayList;    // 是否打开播放列表
	BOOL			m_isplaying;        // 是否正在播放
	LoginInfo		m_loginInfo;		// 登录的相关信息
	string			m_code;				// 请求二维码的code值
	HWND			m_playHwnd[3];		// 播放器真窗口
	int				m_otherWidth;		// 除掉播放器以外的宽度
	int				m_otherHeight;		// 除掉播放器以外的高度
	BOOL			m_isLogin;			// 是否已经登录
	CSize			m_rtClient;			// 播放窗口resize使用
	int             m_playcamera_index;	// 当前摄像机播放的index
	int             m_playdevice_index;	// 当前本地设备播放的index

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, CSize size);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	void OnPlayProgress();
	void OnPlaySwitchPause();
	bool SendCMD(SOCKETOPTION opt, HWND hwnd, REQUEST_TYPE type, string url, string data);			//发送异步网络请求
	void TcpRequestTask(LPVOID data);					//TCP处理任务
	void HttpRequestTask(LPVOID data);					//Http处理任务
	BOOL LoadAvatar();									//加载头像
	void loadDeviceInfo();								//自动登录直接读取数据库设备列表
	void GetCameraList();								//请求获取设备列表信息
	void GetUserInfoRequest();							//请求获取用户信息	
	void RefDeviceAdapterView();						//刷新本地设备列表
	void GetDeviceList();								//通过本机IP获取列表
	void PlayFile(const char *url);
	void PlayLan(const char *url);

private:
	void PlayLive(const char *url);
	bool OnListenTabSelChangePage(EventArgs *pEvtBase);
	bool OnListenIPDropdownBox(EventArgs *pEvtBase);
	int GetLocalIPInfo(SArray<NetInfo> &Info);
	BOOL IsFileExist(const SStringT& csFile);
	void InitSetEvent();
	void OnTimer(UINT_PTR nIDEvent);
	void OnEapilType();	
	void OnPlayList();
	void OnBtnOpen();
	void OnBtnPlay();
	void OnBtnPause();
	void OnBtnStop();
	void OnUserInfoDialog();
	void OnAddCameraDialog();
	bool CheckIp(int type, LPCWSTR pszName, LPCWSTR tipName, string &ip);

	void saveDeviceInfo(string &response);				//保存到设备列表数据库
	CameraStatus GetCameraStatus(int liveStatus);		//获取摄像机的在线状态
	SStringT GetCameraCmpTag(CameraStatus status);		//摄像机在线状态比较


	void SetCameraAdapter();
	void RefCameraAdapterView();						//刷新摄像机列表
	void SetDeviceAdapter();


	//接受键盘输入
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLButtonUp(UINT nFlags, CPoint pt);
	void OnLButtonDown(UINT nFlags, CPoint pt);
	void OnMouseMove(UINT nFlags, CPoint pt);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	
	LRESULT OnMsg_HTTP_TASK(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled);
	LRESULT OnMsg_PLAY_FILE(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled);		//播放文件
	LRESULT OnMsg_ADD_FILED(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled);		//增加文件完成后的通知
	LRESULT OnMsg_QUIT_LOGIN(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled);		//退出登录

	void OnDellfiles_MenuBtn();														//打开删除菜单
	void OnAddfiles_MenuBtn();														//打开增加菜单
	int irmFlvReadFileTemplate(char *FlvFile, unsigned char *FileLink);

protected:

	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_NAME_COMMAND(L"btn_eapiltype", OnEapilType)
		EVENT_NAME_COMMAND(L"btn_playlist", OnPlayList)
		EVENT_NAME_COMMAND(L"user_link", OnUserInfoDialog)
		EVENT_NAME_COMMAND(L"ref_camera", GetCameraList)
		EVENT_NAME_COMMAND(L"ref_device", GetDeviceList)
		EVENT_NAME_COMMAND(L"add_camera", OnAddCameraDialog)
		EVENT_ID_COMMAND(200, OnBtnPlay)
		EVENT_ID_COMMAND(201, OnBtnPause)
		EVENT_ID_COMMAND(202, OnBtnStop)
		EVENT_ID_COMMAND(700, OnAddfiles_MenuBtn)
		EVENT_ID_COMMAND(701, OnDellfiles_MenuBtn)
		//EVENT_ID_HANDLER(SENDER_MAINDLG_ID, MainSocketThread::EventID, OnMainSocketThread)
		EVENT_MAP_END()

		//HostWnd真实窗口消息处理
		BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_TIMER(OnTimer)
		//MSG_WM_LBUTTONDOWN(OnLButtonDown)
		//MSG_WM_LBUTTONUP(OnLButtonUp)
		//MSG_WM_MOUSEMOVE(OnMouseMove)
		//MSG_WM_MOUSEWHEEL(OnMouseWheel)
		//MSG_WM_KEYDOWN(OnKeyDown)
		MESSAGE_HANDLER(WM_WINREQUEST_TASK, OnMsg_HTTP_TASK)
		MESSAGE_HANDLER(MS_PLAYING_PATHNAME, OnMsg_PLAY_FILE)
		MESSAGE_HANDLER(MS_ADD_FILESED, OnMsg_ADD_FILED)
		MESSAGE_HANDLER(WM_QUIT_LOGIN, OnMsg_QUIT_LOGIN)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	BOOL				m_bLayoutInited;
	CWorkQueue			m_WorkQueue;
	BOOL				m_ctrl_down;
	SSliderBar*			m_Sliderbarpos;
	int					m_eapilType;		//全景模式
	//SListView*			m_device_List_Wnd;	//本地设备播放列表控件
	SListView*			m_file_List_Wnd;	//本地文件播放列表控件
	SArray<CameraItem>  m_cameraList;		//我的摄像机列表
	SArray<CameraAddr>	m_devicelist;		//本地设备列表id, ip, url
	UserPopupWnd*		m_pUserWnd;			//个人设置popup窗口
	AddPopupWnd*		m_pAddWnd;			//添加设备popup窗口
	UserInfo			m_userInfo;			//个人信息
	BOOL				m_aiplay;			//当前窗口是ai摄像头播放
};
