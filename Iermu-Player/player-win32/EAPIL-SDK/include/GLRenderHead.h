/**
* @file		枚举等结构定义
* @author	eapil
*/

#ifndef	GLRENDERHEAD_H
#define GLRENDERHEAD_H
#include "QSystemDetection.h"

#ifdef _WIN32
#   ifdef EPRENDER_EXPORT
#       define EPRENDER_LIB __declspec(dllexport)
#   elif defined(EPRENDER_IMPORT)
#       define EPRENDER_LIB __declspec(dllimport)
#   else
#       define EPRENDER_LIB
#   endif
#else
#define EPRENDER_LIB
#endif

/**
* @brief 截屏成功与否
* @param state 0：成功 1：失败
* @param width 输出的rgba数据宽
* @param height 输出的rgba数据高
* @param rgbaData 输出的rgba数据
*/
typedef struct
{
	int state;
	int width;
	int height;
	unsigned char * rgbaData;
}CallBackState;

/**
* @brief 录屏回调
* @param rgbData rgb数据
* @param width 数据宽
* @param height 数据高
* @param arg 用户输入参数
*/
typedef void(*RecordScreenCallBack)(unsigned char *rgbData, int width, int height, void *arg);

/**
* @brief 用于一些不需要参数的回调，譬如gui渲染回调
*/
typedef void(*CommonCallBack)();

/**
* @brief 目前用于截图完后的回调
* @param arg 用户输入参数
* @param state 截图成功与否状态
*/
typedef void(*EapilCallBack)(void *arg, CallBackState * state);

/**
* @brief 用于PBO数据处理完后数据导出回调
* @param rgbData rgb数据
* @param width 数据宽
* @param height 数据高
*/
typedef void(*SaveScreenCallBack)(unsigned char *rgbaData, int width, int height);

/**
* @brief 渲染模式
* @param RENDERBALL 球模式
* @param RENDERSMALLPLANET 小星球模式
* @param RENDERWIDESCREEN 宽屏模式
* @param RENDERVR VR模式
* @param RENDERBALLORTHO 正交模式
* @param RENDERBALLTWOSCREEN 两分屏模式
* @param RENDERBALLFOURSCREEN 四分屏模式
* @param RENDERBALLTHREESCREEN 三分屏模式
* @param RENDERCYLINDER 滚筒
* @param RENDERMULT 综合
*/
typedef enum
{
	RENDERBALL = 0,
	RENDERSMALLPLANET,
	RENDERWIDESCREEN,
	RENDERVR,
	RENDERBALLORTHO,
	RENDERBALLTWOSCREEN,
	RENDERBALLFOURSCREEN,
	RENDERBALLTHREESCREEN,
	RENDERBALLCYLINDER,
	RENDERBALLMULT
}PlayerType;

/**
* @brief 颜色模式
* @param COLORRGB RGB类型
* @param COLORBGR BGR类型
*/
typedef enum
{
	COLORRGB,
	COLORBGR,
}ColorType;

/**
* @brief 当前模式渲染位置状态
* @param mat 初始标定矩阵
* @param lon 横向旋转角度（单位：度）
* @param lat 纵向旋转角度（单位：度）
* @param translateZ 缩放值
* @param fov 视场角
*/
typedef struct
{
	//行主序 4*4 矩阵
	float mat[16];
	float lon;
	float lat;
	float translateZ;
	float fov;
}PlayerMatrixState;

/**
* @brief 截屏类型
* @param WIDESCREEN 宽屏图
* @param OCTSCREEN 八面图
*/
typedef enum
{
	WIDESCREEN = 0,
	OCTSCREEN
}SaveScreenType;

/**
* @brief shader适配类型
* @param ADAPT_NONE 无适配
* @param ADAPT_CAL_PRECISION 计算精度过低
*/
typedef enum
{
	ADAPT_NONE = -1,
	ADAPT_CAL_PRECISION,
}AdaptationType;

/**
* @brief 检测框位置信息（坐标圆心为1920*1080图像左上角）
* @param topLeftCorner 左上角坐标
* @param bottomRightCorner 右下角坐标
*/
typedef struct
{
	float topLeftCorner[2];
	float bottomRightCorner[2];
}DynamicDetection;

#if !(defined(Q_OS_IOS) || defined(Q_OS_ANDROID))
/**
* @brief 渲染回调（目前主要用于渲染后的GUI叠加）
*/
struct EPRENDER_LIB RenderStateCallBackBase
{
	/**
	* @brief 回调接口
	*/
	virtual void Run();

	/**
	* @brief 是否是球模式
	*/
	bool m_bIsRenderBall;
};
#endif
#endif  // GLRENDERHEAD_H
