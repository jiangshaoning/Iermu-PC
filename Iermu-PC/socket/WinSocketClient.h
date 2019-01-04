#pragma once
#include <iostream>
#include <ws2tcpip.h> 
#include "UpnpTool.h"
#include "HttpConnect.h"

#pragma comment(lib, "ws2_32.lib") 

#define QRCODEIMAGE_URL		"https://api.iermu.com/oauth2/authorize?"
#define QRCODESTATUS_URL	"https://api.iermu.com/oauth2/qrcode/status?"
#define AUTHORIZATION_URL	"https://api.iermu.com/oauth2/token"
#define GETUSERINFO_URL		"https://api.iermu.com/v2/passport/user?"
#define GETDEVICEINFO_URL	"https://api.iermu.com/v2/device"
#define GETSERVICEPUSH_URL	"https://api.iermu.com/v2/service/push"

#define CAMERA_DEFAULTIP				"192.168.1.10"
#define CAMERA_USERNAME					"88888888"
#define CAMERA_USERPWD					"88888888"
#define EVT_SOCKET_BEGIN				(EVT_EXTERNAL_BEGIN + 30000)

#define DOMAIN_LEN						(61)
#define CPDOMAIN_LEN					(64)
#define LIBNAME_LEN						(110)

#define NASUSER_LEN						(33)
#define NASPASSWD_LEN					(33)
#define NASPATH_LEN						(51)
#define NASNAME_LEN						(16)
enum
{
	SENDER_MAINDLG_ID = 30000,
};

typedef enum
{
	OPT_PLAY_VIDEO,					//播放视频操作（不走网络）
	OPT_GETCAMERA_INFO,				//从摄像机IP获取信息
	OPT_GETCAMERA_LIST,				//从本机IP获取列表信息
	OPT_SETCAMERA_GENERAL,			//设置通用
	OPT_SETCAMERA_SERVER,			//设置服务配置
	OPT_SETCAMERA_STORE,			//设置储存配置
	OPT_SETCAMERA_OTHER,			//设置其他
	OPT_SETTIME_SYNC,				//时间同步
	OPT_FORMAT_SDCARD,				//格式化SD卡

	OPT_GET_QRCODE,					//获取二维码(http协议)
	OPT_QRCODE_STATUS,				//二维码状态
	OPT_PASSWD_LOGIN,				//密码登录
	OPT_QUIT_LOGIN,					//注销登录
	OPT_REGISTRE,					//注册
	OPT_CANCELLATION,				//注消(http协议)
	OPT_GETONLINE,					//获取上线状态
	OPT_GETDEVICEINFO,				//获取设备列表
	OPT_GETUSERINFO,				//获取用户信息
	OPT_GETPLAYURL,					//获取摄像机播放地址
}SOCKETOPTION;


typedef struct
{
	UINT32 dhcp;
	UINT8 hostIP[4];
	UINT8 ipmask[4];
	UINT8 gateway[4];
	UINT8 dnsip1[4];
	UINT8 dnsip2[4];
	UINT16 cmdPort;
	UINT16 dataPort;
	UINT16 httpPort;
	UINT16 talkPort;
	UINT16 autoConnect;
	UINT16 reserved;
	UINT32 centerIp0;
	UINT32 centerIp1;
}CameraNet;

typedef struct
{
	UINT8 cp_domain[CPDOMAIN_LEN];
	UINT8 domain[DOMAIN_LEN];
	UINT16 port;
	UINT16 cp_port;
	UINT8 sw_id[LIBNAME_LEN];
	UINT8 lib_type;
}IPPort;

typedef struct
{
	SOCKETOPTION opt;
	HWND hwnd;
	REQUEST_TYPE type;
	string url;
	string data;
}SocketData;

typedef struct
{
	SOCKETOPTION opt;
	HWND hwnd;
	bool  retOK;
	int  retValue;
	string hData;
	string nData;
	int progress; //进度值
}SocketRetData;

typedef struct
{
	int date;
	int time;
}DateTime;

typedef struct
{
	const char* errMsg;
}HttpErrorResponse;

typedef struct __tagCommand
{
	char main;
	char child;
	unsigned short len;
}COMMAND, *LPCOMMAND;


typedef struct
{
	UINT8 face_rec;
	UINT8 face_cod;
	UINT8 face_cou;
}FaceFuncSetting;


typedef struct
{
	UINT8 api_key[32];
	UINT8 secret_key[32];
	UINT8 facial;
	UINT8 faces;
	UINT8 face_img;
	UINT16 box;
	UINT16 width;
	UINT16 height;
	UINT16 reliability;
	UINT16 res_time;
	UINT8 groupID[4];
}FaceParameter;


typedef struct
{
	UINT8 head[4]; //固定 0x72,0x2a,0x3c,0x6e 写死了
	UINT8 lan;
	UINT8 comparison;
	UINT16 jpgmem;
}AIFunction;

//NAS参数 62
typedef struct
{
	UINT8 status; //nas 使能（两次开关才能打开NAS）
	UINT8 user[NASUSER_LEN];
	UINT8 passwd[NASPASSWD_LEN];
	UINT8 path[NASPATH_LEN];
	UINT8 name[NASNAME_LEN];
	UINT8 ip[4];
	UINT32 space;
}NASParameter;

//录像参数 43
typedef struct
{
	UINT8 rec; //最低位为1表示禁止了卡录和NAS录像  最高位为1表示事件录像 0表示持续录像
}RECParameter;

typedef struct
{
	CameraAddr cad;
	IPPort	ipp;
	FaceParameter fp;
	FaceFuncSetting ffs;
	AIFunction af;
	NASParameter np;
	RECParameter rec;
	int totalspace;
}AICameraInfo;

//在MainDlg中使用SNotifyCenter的通知的异步事件 
class MainSocketThread : public TplEventArgs<MainSocketThread>
{
	SOUI_CLASS_NAME(MainSocketThread, L"on_main_socket_thread")
public:
	MainSocketThread(SObject *pSender) :TplEventArgs<MainSocketThread>(pSender){}
	enum{ EventID = EVT_SOCKET_BEGIN };

	SOCKETOPTION opt;
	bool  retOK;
	int  retValue;
	string hData;
	SStringT nData;
	int progress; //进度值
};

class WinSocketClient
{
private:
	long long Convert(char* data);
	int ll2str(char *s, long long value);
	int ull2str(char *s, unsigned long long v);
	int cmsSend(SOCKET sockfd, char *buff, int len, int mode);
	int cmsRecv(SOCKET sockfd, char *buff, int len, int mode);
	bool SendBuffToHost(const char *ip, char *sendbuff, int sendlen, char *outbuff, bool save);
public:
	//save: true保存菜单   （走65 3协议）

	bool GetDeviceID(const char *ip, CameraAddr &cad);
	bool SetCameraTime(const char *ip, DateTime &dt, bool save);
	bool GetHostIPAddr(const char *ip, IPPort &ipp);
	bool SetHostIPAddr(const char *ip, IPPort &ipp, bool save);
	bool GetFaceParameter(const char *ip, FaceParameter &fpar);
	bool SetFaceParameter(const char *ip, FaceParameter &fpar, bool save);
	bool GetAIFunction(const char *ip, AIFunction &aifunc);
	bool SetAIFunction(const char *ip, AIFunction &aifunc, bool save);

	//设置、获取录像类型
	bool GetRec(const char *ip, RECParameter &rec);
	bool SetRec(const char *ip, RECParameter &rec, bool save);

	bool GetFaceFuncSetting(const char *ip, FaceFuncSetting &ffset);
	bool SetFaceFuncSetting(const char *ip, FaceFuncSetting &ffset, bool save);

	//设置、获取NAS参数
	bool GetNAS(const char *ip, NASParameter &nas);
	bool SetNAS(const char *ip, NASParameter &nas, bool save);

	//重启设备
	bool RestartDevice(const char *ip);

	//格式化SD卡
	bool SetFormateSDCard(const char *ip);
	bool GetFormateSDCard(const char *ip, int &progress);

	//非dhcp下设置摄像机网络信息
	bool GetCameraNET(const char *ip, CameraNet &net);
	bool SetCameraNET(const char *ip, CameraNet &net, bool save);

	//获取磁盘总空间
	bool GetCameraStore(const char *ip, int totalSpace);
	//bool GetPanorama(const char *ip, string &panoTemplate);
};