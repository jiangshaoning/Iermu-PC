// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#define  _CRT_SECURE_NO_WARNINGS
#define	 DLL_SOUI
#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/souictrls.h>
#include <res.mgr/sobjdefattr.h>
#include <event/notifycenter.h>
#include <com-cfg.h>
#include "resource.h"
#define R_IN_CPP	//定义这个开关来
#include "res\resource.h"
#include "SGifPlayer.h"
using namespace SOUI;


#define INT64_MIN       (-9223372036854775807i64 - 1)
#define INT64_MAX       9223372036854775807i64
#define SIZE_T_MAX      0xffffffff
#define DEBUG_NEW		new(THIS_FILE, __LINE__)

/*
* 禁止复制基类
*/
class INoCopy
{
private:
	INoCopy(const INoCopy& rhs);
	INoCopy& operator = (const INoCopy& rhs);

public:
	INoCopy() {};
	~INoCopy() {};
};



#define RELEASEPLAYER(player) \
if (player) \
{ \
	player_close(player); \
	player = NULL; \
}

#define NUMBEROFTHREADS				5

#define LOGIN_DAT					"login.dat"
#define IERMU_DB					"iermu.db"
#define DEVICETABLE					"deviceTable"

#define QRCODE_PATH					"uires\\config\\qrcode.jpg"
#define LQRCODE_PATH				L"uires\\config\\qrcode.jpg"

#define AVATAR_PATH					"uires\\config\\"					
#define LAVATAR_PATH				L"uires\\config\\"

#define FACE_PATH					"uires\\facecover\\"

#define TIMER_ID_PLAYING_PROGRESS	1
#define TIMER_ID_HIDE_TEXT			2
#define TIMER_ID_QRCODE_STATUS		3


#define APPWND_BORDER				4
#define APPWND_LEFT_WIDTH			250						// 普通播放器需要减掉左边的宽度
#define APPWND_TOP_HEIGHT			64						// 普通播放器需要减掉上面的高度
#define APPWND_TOOLS_HEIGHT			40						// 普通播放器需要减掉下边工具栏的高度

#define APPWND_RIGHT_WIDTH			222						// AI播放器需要减掉右边的宽度
#define APPWND_BOTTOM_HEIGHT		184						// AI播放器需要减掉下面的高度

#define WM_USER_PLAYING				WM_USER + 1				// 开始播放文件
#define WM_USER_POS_CHANGED			WM_USER + 2				// 文件播放位置改变
#define WM_USER_END_REACHED			WM_USER + 3				// 播放完毕
#define MS_PLAYING_PATHNAME			WM_USER + 4				// 通知播放文件
#define MS_ADD_FILESED				WM_USER + 5				// 增加文件后的通知
#define WM_ADD_FILESED				WM_USER + 6				// 增加文件后的切换界面通知

#define WM_TCPREQUEST_TASK			WM_USER+100				// TCP请求完成后通知
#define WM_WINREQUEST_TASK			WM_USER+101				// 网络请求完成后通知
#define MS_SHOW_LOGINWND			WM_USER+102				// 显示播放dialog
#define WM_QUIT_LOGIN				WM_USER+103				// 退出登录


#define MS_INIT_REALWND				WM_USER+200				// 初始化真窗口
#define MS_OPEN_TIPPAGE				WM_USER+201				// 打开第一个界面
#define MS_OPEN_LOGIN				WM_USER+202				// 打开登录界面
#define MS_OPENVIDEO_REALWND		WM_USER+203				// 打开播放器
#define MS_CLOSEVIDEO_REALWND		WM_USER+204				// 关闭播放器
#define MS_SAVE_TOKEN				WM_USER+205				// 保存token