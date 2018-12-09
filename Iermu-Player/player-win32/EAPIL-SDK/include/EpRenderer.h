/**
* @file		接口文件
* @author	eapil
*/

#ifndef _EPRENDERER_H_
#define _EPRENDERER_H_

#ifdef _WIN32
#   ifdef EPRENDER_EXPORT
#       define EPRENDER_LIB __declspec(dllexport)
#   elif defined(EPRENDER_IMPORT)
#       define EPRENDER_LIB __declspec(dllimport)
#   else
#       define EPRENDER_LIB
#   endif
#else
#   define EPRENDER_LIB
#endif
#include <string>

#include "GLRenderHead.h"

#if defined(Q_OS_ANDROID)
#include "EapilTemplateParser/EpTemplateHead.h"
#else
#include "EpTemplateHead.h"
#endif

class GLRenderControl;

class GLRenderPBO;

#if !(defined(Q_OS_IOS) || defined(Q_OS_ANDROID))
class EPRENDER_LIB EpRenderer
#else
class  EpRenderer
#endif
{
public:

	/**
	* @brief 注册SDK
	* @param key 易瞳颁发的key
	*/
	static bool RegisterSDK(char * key);
	/**
	* @brief	构造函数
	*/
	EpRenderer();

	/**
	* @brief	析构函数
	*/
	~EpRenderer();

	/**
	* @brief	初始化opengl相关资源
	*/
	void InitOpenGL();

	/**
	* @brief	释放opengl相关资源
	*/
	void UnInitOpenGL();

	/**
	* @brief 渲染
	* @return true：成功 false：失败
	*/
	bool Render();

	/**
	* @brief 左键摁下消息
	* @param x 鼠标横坐标
	* @param y 鼠标纵坐标
	*/
	void OnLButtonDown(int x, int y);

	/**
	* @brief 左键抬起消息
	*/
	void OnLButtonUp();

	/**
	* @brief 鼠标移动消息
	* @param x 鼠标横坐标
	* @param y 鼠标纵坐标
	*/
	void OnMouseMove(int x, int y);

	/**
	* @brief 鼠标滚轮消息
	* @param delta 滚轮滑动值
	*/
	void OnMouseWheel(short delta);

	/**
	* @brief 传输YUV数据
	* @param yuvData 数据指针
	* @param width 数据宽
	* @param height 数据高
	* @param time 此帧显示时间
	*/
	void TranslateVideoData(unsigned char * yuvData, int width, int height, long long time = 0);

	/**
	* @brief 传入欧拉角
	* @param yaw 绕x轴旋转角度（单位：度）
	* @param pitch 绕y轴旋转角度（单位：度）
	* @param roll 绕z轴旋转角度（单位：度）
	*/
	void TranslateEulerRotate(float yaw, float pitch, float roll);

	/**
	* @brief 传输RGBA数据
	* @param RGBA 数据指针
	* @param width 数据宽
	* @param height 数据高
	*/
	void TranslateImageData(unsigned char * rgbaData, int width, int height);

	/**
	* @brief 设置窗口宽高
	* @param width		窗口宽
	* @param height		窗口高
	*/
	void SetWindow(int width, int height);

	/**
	* @brief 加载模板数据
	* @param path 模板数据
	* @param type 模板类型
	*/
	void LoadTemplate(std::string path, TemplateType type = CAMERAEN);

	/**
	* @brief 加载模板数据
	* @param path 模板数据
	* @param type 模板类型
	*/
	void LoadTemplate(char * path, TemplateType type = CAMERAEN);

	/**
	* @brief 设置渲染帧率
	* @param rate 帧率
	*/
	void SetTargetFrameRate(float rate);

	/**
	* @brief 设置播放模式
	* @param type 播放模式
	*/
	void SetPlayerType(PlayerType type);

	/**
	* @brief	设置VR模式下旋转的绝对角度
	* @param rotateX 绕x轴旋转绝对角度(单位：度)
	* @param rotateY 绕x轴旋转绝对角度(单位：度)
	* @param rotateZ 绕x轴旋转绝对角度(单位：度)
	*/
	void SetRotateVR(float rotateX, float rotateY, float rotateZ);

	/**
	* @brief 设置fbo画布id
	* @param id fboid
	*/
	void TransOpenGLFboID(int id);

	/**
	* @brief 判断渲染器是否支持此图片解析
	* @param fileName 图片文件名
	* @return true：支持 false：不支持
	*/
	bool WhetherSupportImage(std::string name);

	/**
	* @brief 开启录屏
	*/
	void EnableRecordScreen();

	/**
	* @brief 关闭录屏
	*/
	void DisableRecordScreen();

	/**
	* @brief 设置录屏数据回调
	* @param callBack 回调函数指针
	* @param arg 数据参数
	*/
	void SetRecordScreenCallBack(RecordScreenCallBack callBack, void *arg);

	/**
	* @brief 截屏
	* @param fileName 存储路径名
	* @param wideScreen true：宽屏截取，false：截取当前屏幕
	* @return 0：成功 1：失败
	*/
	int  SaveScreenImage(std::string fileName, bool wideScreen = true);

	int  SaveScreenImage(char * fileName, bool wideScreen = true);

	/**
	* @brief 将当前屏幕图像存人内存
	* @return 0：成功 1：失败
	*/
	int  SaveScreenImageToMemory();

	/**
	* @brief 将存入内存的图片存入指定硬盘位置
	* @param fileName 存储路径名
	* @return 0：成功 1：失败
	*/
	int  SaveScreenImageToFile(std::string fileName);

	/**
	* @brief	进入标定模式
	* @return true：成功 false：失败
	*/
	bool EnterStandardizationState();

	/**
	* @brief	离开标定模式
	* @return true：成功 false：失败
	*/
	bool LeaveStandardizationState();

	/**
	* @brief	取消标定模式
	* @return true：成功 false：失败
	*/
	bool CancelStandardizationState();

	/**
	* @brief	进入自动旋转模式
	* @param speed 旋转速度（°/s）
	*/
	void EnterAutoRotationState(float speed = 0.3);

	/**
	* @brief	退出自动旋转模式
	*/
	void LeaveAutoRotationState();

	/**
	* @brief	设置标定时的旋转速度
	* @param	rate 旋转速度
	*/
	void SetStandardizationRate(float rate);

	/**
	* @brief 设置数据渲染通道类型
	* @param colorType 通道类型 0：RGB,1：BGR
	*/
	void SetColorType(ColorType colorType);

	/**
	* @brief	设置GUI渲染回调函数
	* @param	callBack 函数指针
	*/
	void SetGUIRenderCallBack(CommonCallBack callBack);

	/**
	* @brief 重新打开视频时重置播放状态（清除视频数据）
	*/
	void ResetPlayState();

	/**
	* @brief 判断当前是否是球模式渲染
	* @return true：球模式 false：非球模式
	*/
	bool IsRenderBall();

	/**
	* @brief 获取当前标定矩阵和球旋转缩放状态数据
	* @return 返回矩阵和球旋转缩放状态
	*/
	PlayerMatrixState GetPlayerMatrixState(PlayerType type, int num);

	/**
	* @brief 设置当前标定矩阵和球旋转缩放状态数据
	* @param state 标定矩阵和旋转缩放状态数据
	* @param num 分屏号
	*/
	void SetPlayerMatrixState(PlayerMatrixState * state, PlayerType type, int num);

	/**
	* @brief 设置图像是否可以缩放
	* @param scale true：开启缩放, false：关闭缩放
	* @param beginScale 图像缩放值
	*/
	void SetImageScalable(bool scale = false, float beginScale = 1.0);

	/**
	* @brief 设置图像缩放值（用于宽屏模式）
	* @param scale 缩放量
	*/
	void SetImageScaleForWS(float scale);

	/**
	* @brief 宽屏模式下设置纵向拖动方式
	* @param state 0 旋转拖动  1 平动 2 不能动
	*/
	void SetWideScreenYCanMove(int state);

	/**
	* @brief 清除模板
	*/
	void ClearTemplete();

	/**
	* @brief 获取NV12数据指针
	* @param data 数据指针
	* @param width 数据宽
	* @param height 数据高
	* @return true：成功 false：失败
	*/
	bool GetNV12Data(unsigned char *&data, int &width, int &height);

	/**
	* @brief 传入IJK纹理
	* @param width 纹理宽
	* @param height 纹理高
	*/
	void IJKTranslateTexture(int width, int height);

	/**
	* @brief 设置颜色转换矩阵
	* @param mat 矩阵
	*/
	void IJKSetUm3ColorConversion(void * mat);

	/**
	* @brief 设置图像转换类型（为了适配ijk）
	* @param type 具体类型参照shader
	*/
	void IJKSetTextureType(int type);

	/**
	* @brief 设置陀螺仪输入是否开启
	* @param state true：开启 false：关闭
	*/
	void SetGyroInputState(bool state);

	/**
	* @brief 获取播放模式
	* @return 播放模式
	*/
	PlayerType GetPlayerType();

	/**
	* @brief 设置球缩放范围和初始缩放大小
	* @param minDisToEye 最小缩放值
	* @param maxDisToEye 最大缩放值
	* @param initDis	 当前初始缩放值
	*/
	void SetBallPosRange(float minDisToEye, float maxDisToEye, float initDis);

	/**
	* @brief 设置指定模式缩放范围和初始缩放大小
	* @param type 播放模式
	* @param minDisToEye 最小缩放值
	* @param maxDisToEye 最大缩放值
	* @param initDis	 当前初始缩放值
	*/
	void SetBallPosRangeType(PlayerType type, float minDisToEye, float maxDisToEye, float initDis);

	/**
	* @brief 获取模视矩阵和投影矩阵
	* @param modelViewMatrix 模视矩阵
	* @param projectMatrix 投影矩阵
	*/
	void GetCurrentMVPMatrix(float * modelViewMatrix, float * projectMatrix);

	/**
	* @brief 设置标定图标是否显示
	* @param show true：显示 false：不显示
	*/
	void SetStandardIcon(bool show);

	/**
	* @brief 设置截屏类型
	* @param type 0：宽屏 1：八面体
	*/
	void SetSaveScreenType(SaveScreenType type);

	/**
	* @brief 设置适配类型
	* @param type 适配类型
	*/
	void SetAdaptationType(AdaptationType type);

	/**
	* @brief 重置渲染状态（重置渲染矩阵等）
	*/
	void ResetRendererState();

	/**
	* @brief 设置是否能截屏
	* @param canSaveImage true：能截屏 false：不能截屏
	*/
	void SetCanSaveImage(bool canSaveImage);

	/**
	* @brief 设置是否显示logo
	* @param visable true：显示 false：不显示
	*/
	void SetLogoVisable(bool visable);

	/**
	* @brief 获取SDK版本号
	* @return 版本号字符串
	*/
	std::string GetSDKVersion();


private:
	/**
	* @brief 渲染器管理类
	*/
    GLRenderControl *	m_pRender;

	/**
	* @brief PBO用于图像拼接
	*/
	static GLRenderPBO *m_pPBORenderer;
};


#endif  // _EPRENDERER_H_
