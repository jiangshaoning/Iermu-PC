/** @file include/simple_player.h
 *  @ingroup SimplePlayerGroup
 *  @brief SimplePlayer播放库对外接口文件
 *  @details 提供播放库的所有接口，也是核心头文件，定义了项目主页、相关页面、模块
 *  @date 2018-1-29
 *  @author 杨康
 */

/**
 * @mainpage
 *
 * # 项目简介
 * 这是一个跨平台音视频底层库，以 @ref BaseFacilityGroup "基础工具" 为基础，实现了4大功能：
 * @ref SimplePlayerGroup "音视频播放器",
 * @ref SimpleRecoderGroup "音视频录像",
 * @ref FileStitchGroup "文件拼接",
 * @ref VESGroup "虚拟Eapil流媒体分发服务"
 *
 * ### 音视频播放器
 * - 支持ffmpeg所支持的格式，Eapil协议，浪涛数据格式，UVC Camera
 * - 手机端播放过程中对讲，回音消除
 * - PC端支持边播边录
 *
 * ### 音视频录像
 * - 输入音视频帧数据，输出到文件或者推流的服务器
 *
 * ### 文件拼接
 * - 可以让用户编辑了yuv和pcm数据后再写入文件或者推流
 *
 * ### 虚拟Eapil流媒体分发服务
 * - 提供虚拟Eapil流媒体分发服务，供测试人员使用
 *
 * @defgroup BaseFacilityGroup         BaseFacility
 * 基础工具
 * @defgroup SimplePlayerGroup         SimplePlayer
 * 音视频播放器
 * @defgroup SimpleRecoderGroup        SimpleRecoder
 * 音视频录像
 * @defgroup FileStitchGroup           FileStitch
 * 文件拼接
 * @defgroup VESGroup                  VES
 * 虚拟Eapil流媒体分发服务
 */

/**
 * @page changelogpage 版本更新日志

 * ## V1.0.2(git-2018-03-26-ecc3ffa): 新增部分接口
 * - SimplePlayer新增char* 版本的ReadTemplate和SetDataSource接口(PC需求)
 * - SimplePlayer新增设置缓存SetMaxCacheVideoPktCount、强制直播丢包模式SetForceLiveMode、关闭AV同步SetNotNeedAVSync接口(星云需求)
 * - FileStitch新增支持码率设置的SetParams接口(IOS需求)


 * ## V1.0.1(git-2018-03-19-2931fab): 解决BUG
 * - 解决文件拼接库，对于只有视频没有音频文件无法解码bug
 * - 播放位置更新改为1秒10次
 * - 录像码率控制在1M以内

 * ## V1.0.0(git-2018-02-02-20e3aa3): 基本改完所有已知BUG
 * - 添加注释以便生成文档
 * - 解决ios对讲声音变调bug
 * - 再次优化拖动
 *
 * ## V0.0.1: 最初版本
 */

/**
 * @page APIChangelogpage 接口更新日志
 * ## V1.0.0: 新增Version接口
 * - 新增SimplePlayer::Version()接口,返回类似"V1.0.0@git-2018-02-02-20e3aa3"版本信息
 */


#ifndef SIMPLEPLAYER_H
#define SIMPLEPLAYER_H
#include <string>

#ifdef _WIN32
#   ifdef SIMPLEPLAYER_EXPORT
#       define SIMPLEPLAYER_LIB __declspec(dllexport)
#   else
#       define SIMPLEPLAYER_LIB __declspec(dllimport)
#   endif
#else
#   define SIMPLEPLAYER_LIB
#endif

namespace simpleplayer
{

class SimplePlayerCallback;

class SIMPLEPLAYER_LIB  SimplePlayer
{
public:
    /**
     * @brief 错误类型枚举
     */
	enum ErrorCode {
		UNKNOWN,
		//input
		OPEN_STREAM_FAILED,

		//codec
		VIDEO_DECODE_ERROR,
		AUDIO_DECODE_ERROR,

		//device
		VIDEO_DEVICE_ERROR,
		AUDIO_DEVICE_ERROR,

		//output
		OPEN_OUTPUT_FAILED,
		//encode
		VIDEO_ENCODE_ERROR,
		AUDIO_ENCODE_ERROR,
	};

    /**
     * @brief 播放状态枚举
     */
	enum PlayStatus{
		kStoped,
		kPlaying,
		kPaused,
	};

    /**
     * @brief 原始视频帧类型枚举
     */
	enum VideoFrame
	{
		kRGB24,
		kRGB32,
		kYUV420P,
		kNV12,
	};

    /**
     * @brief 模板参数
     */
	struct SIMPLEPLAYER_LIB TempParam
	{
		TempParam();
		~TempParam();
		std::string temp;
	};

    /**
     * @brief UVC相机唯一标识
     */
	struct SIMPLEPLAYER_LIB UVCCameraUniqueName
	{
		UVCCameraUniqueName();
		~UVCCameraUniqueName();
		std::string name;
	};

    /**
     * @brief 音频编码类型
     */
	enum AUDIO_CODEC
	{
		kALAW = 0,
		kMULAW,
		kAAC
	};

public:
	//创建与销毁
    /**
     * @brief Create 创建SimplePlayer指针
     * @return SimplePlayer指针
     */
	static SimplePlayer * Create();

    /**
     * @brief Destory 销毁SimplePlayer指针
     * @param sample_player SimplePlayer指针
     */
	static void Destory(SimplePlayer * sample_player);

    /**
     * @brief Version 返回版本信息
     * @return
     */
	static const char* Version();

    /**
     * @brief ReadTemplate 读取本地文件的模板
     * @param path 本地文件路径
     * @param strTemplate 返回的模板
     * @return 调用是否成功
     */
	static bool ReadTemplate(const std::string &path, TempParam &strTemplate);

	//调用者确保strTemplate包含足够内存存放模板
	static bool ReadTemplate(const char* path, char strTemplate[]);

    /**
     * @brief GetVideoFirstRGB24Frame 获取视频文件的第一帧rgb24格式数据
     * @param filename 文件名
     * @param width 宽度
     * @param height 高度
     * @param data rgb数据指针
     * @param data_size rgb数据长度
     * @return 调用是否成功
     */
	static bool GetVideoFirstRGB24Frame(char *filename, int *width, int *height, unsigned char **data, int *data_size);

    /**
     * @brief GetUVCCameraUniqueName 获取uvc相机的唯一标码
     * @param name 用来返回结果的参数
     * @return 调用是否成功
     */
    static bool GetUVCCameraUniqueName(UVCCameraUniqueName &name);
public:
	//基础接口
	//url实例:
	//1. "eapil://192.168.31.10"
	//2. "rtsp://192.168.31.222:554/1/h264major"
	//3. "uvc://video=UVC Camera"
	//4. "uvc://"
	//5. "langtao:@2@875967048@18@928@576@0@31269@1@8000@16"
	//6. "F:/ffmpeg/movie/bb.epv"
    /**
     * @brief SetDataSource 设置播放url
     * url实例:
     * 1. "eapil://192.168.31.10"
     * 2. "rtsp://192.168.31.222:554/1/h264major"
     * 3. "uvc://video=UVC Camera"
     * 4. "uvc://"
     * 5. "langtao:@2@875967048@18@928@576@0@31269@1@8000@16"
     * 6. "F:/ffmpeg/movie/bb.epv"
     * @param url 播放url
     * @param auto_reconnect 是否自动重连，默认为true
     */
    virtual void SetDataSource(const std::string & url,bool auto_reconnect = true) = 0;
	virtual void SetDataSource(const char* url, bool auto_reconnect = true) = 0;

    /**
     * @brief RegisterCallback 注册回调函数
     * @param cb 回调对象指针
     */
    virtual void RegisterCallback(SimplePlayerCallback *cb) = 0;
	
	//播放与停止接口
    /**
     * @brief Play 开始播放或者由暂停状态编码播放状态
     * @return 调用是否成功
     */
	virtual bool Play() = 0;

    /**
     * @brief Stop 结束播放
     * @return 调用是否成功
     */
	virtual bool Stop() = 0;

    /**
     * @brief Started 获取是否已经开始
     * @return 是否已经开始
     */
	virtual bool Started() const = 0;//暂停或者播放状态,Play()为true,Stop()后为false

	//暂停与恢复接口
    /**
     * @brief Pause 暂停
     * @return 调用是否成功
     */
	virtual bool Pause() = 0;

    /**
     * @brief Resume 从暂停恢复到播放状态
     * @return 调用是否成功
     */
	virtual bool Resume() = 0;

    /**
     * @brief Playing 获取是否正在播放
     * @return 是否正在播放
     */
	virtual bool Playing() const = 0;

	//seek接口
    /**
     * @brief Seekable 获取是否可以拖动
     * @return 是否可妥当
     */
	virtual bool Seekable() const = 0;

    /**
     * @brief Seekto 拖动
     * @param position 妥当位置
     * @return 调用是否成功
     */
	virtual bool Seekto(int64_t position) = 0;
	
	//循环播放接口,直播流不能循环播放
    /**
     * @brief SetPlayLoop 设置选好播放开关
     * @param lp true开启，false关闭
     * @return 调用是否成功
     */
	virtual bool SetPlayLoop(bool lp) = 0;

    /**
     * @brief IsPlayLoop 获取是否循环播放开关状态
     * @return 是否开启循环播放
     */
	virtual bool IsPlayLoop() const = 0;
	
	//获取状态接口
    /**
     * @brief GetStatus 获取播放器状态
     * @return 播放器状态
     */
	virtual PlayStatus GetStatus() const = 0 ;
    /**
     * @brief GetDuration 获取总时长
     * @return 总时长
     */
	virtual int64_t GetDuration() const = 0;

    /**
     * @brief GetPostion 获取当前播放位置
     * @return 当前播放位置
     */
    virtual int64_t GetPostion() const = 0;
	
    //系统音量控制接口,0-300
    /**
     * @brief SetVolumeLevel 设置音量大小，0-300，300是3倍
     * @param level 音量大小
     * @return 调用是否成功
     */
	virtual bool SetVolumeLevel(int level) = 0;

    /**
     * @brief SetMute 静音开关
     * @param mute true开启静音 false关闭静音
     * @return 调用是否成
     */
    virtual bool SetMute(bool mute) = 0;

    /**
     * @brief GetVolumeLevel 获取音量
     * @param level 音量
     * @return  调用是否成功
     */
	virtual bool GetVolumeLevel(int *level) const = 0;

    /**
     * @brief IsMute 获取静音状态
     * @return 是否静音
     */
    virtual bool IsMute() const = 0;

	//硬件编解码开关,下次开始播放时生效，只对android和win有效,ios性能高直接开启
    /**
     * @brief EnableHWCodec 硬件编解码开关,下次开始播放时生效，只对android和win有效,ios性能高直接开启
     * @param enable true开启，false关闭
     */
	virtual void EnableHWCodec(bool enable) = 0;

    /**
     * @brief IsHWCodecEnable 获取硬解编码开关
     * @return
     */
	virtual bool IsHWCodecEnable() const = 0;

    /**
     * @brief SetMaxCacheVideoPktCount 设置最大的视频缓存帧数
     * @param count
     */
	virtual void SetMaxCacheVideoPktCount(int count) = 0;
	virtual int GetMaxCacheVideoPktCount() const = 0;


    /**
     * @brief SetForceLiveMode 设置强制直播模式
     * @param enable
     */
	virtual void SetForceLiveMode(bool enable) = 0;
	virtual bool IsForceLiveMode() const = 0;

    /**
     * @brief SetNotNeedAVSync 设置关闭音视频同步
     * @param enable
     */
	virtual void SetNotNeedAVSync(bool enable) = 0;
	virtual bool IsNotNeedAVSync() const = 0;

	//浪涛接口, 浪涛的url格式："langtao:@2@875967048@18@928@576@0@31269@1@8000@16"
    /**
     * @brief PushLangTaoVideoData 放入浪涛视频数据
     * @details 播放浪涛需要手动传入音视频数据，浪涛的url格式："langtao:@2@875967048@18@928@576@0@31269@1@8000@16"
     * @param data 数据指针
     * @param len 数据长度
     * @param timestamp 时间戳
     * @param iframe 是否关键帧
     * @param flush 是否需要刷新状态
     * @return 调用是否成功
     */
    virtual bool PushLangTaoVideoData(uint8_t *data, int len, int timestamp, bool iframe, bool flush)= 0;

    /**
     * @brief PushLangTaoAudioData 放入浪涛音频数据
     * @param data 数据指针
     * @param len 数据长度
     * @param timestamp 时间戳
     * @return 调用是否成
     */
    virtual bool PushLangTaoAudioData(uint8_t *data, int len, int timestamp) = 0;

	//录像接口：适合边播边录，录制的是看到的内容（pc客户端）
	//1. 首先Play()；
	//2. 接着StartRec(url)；
	//3. 结束StopRec();
    /**
     * @brief StartRec 录像接口：适合边播边录，录制的是看到的内容（pc客户端）
     * @param url 录像输出地址
     * @return 调用是否成功
     */
	virtual bool StartRec(const std::string & url) = 0;//不支持重复录制，如果已经在录像会返回失败

	virtual bool StartRec(const char* url) = 0;

    /**
     * @brief StopRec 结束录像
     * @return
     */
    virtual bool StopRec() = 0;

    /**
     * @brief Recording 获取是否正在录像
     * @return 是否正在录像
     */
	virtual bool Recording() const = 0;

    /**
     * @brief GetRecPosition 获取当前录像时长
     * @return 当前录像时长
     */
	virtual int64_t GetRecPosition() const = 0;

	//android ios录音接口
	//#define WAVE_FORMAT_G711                          0x7A19
	//#define WAVE_FORMAT_AAC                           0x7A26
	//#define WAVE_FORMAT_G711U                         0x7A25
	//format:0(g711a),1(g711u),2(aac).如果是浪涛format要转一下
    /**
     * @brief StartAudioCapture android ios录音接口
     * @param dst_format 目标格式
     * #define WAVE_FORMAT_G711                          0x7A19
     * #define WAVE_FORMAT_AAC                           0x7A26
     * #define WAVE_FORMAT_G711U                         0x7A25
     * format:0(g711a),1(g711u),2(aac).如果是浪涛format要转一下
     * @param dst_samplerate 目标采样率
     * @param dst_channel 目标声道数
     * @return 调用是否成功
     */
    virtual bool StartAudioCapture(const AUDIO_CODEC dst_format,int dst_samplerate,int dst_channel) = 0;

    /**
     * @brief AudioCapture 获取是否正在录音
     * @return 是否正在录音
     */
    virtual bool AudioCapture() = 0;

    /**
     * @brief StopAudioCapture 停止录音
     */
	virtual void StopAudioCapture() = 0;
public:
	virtual ~SimplePlayer() {}
};

class SimplePlayerCallback {
public:
    /**
     * @brief OnYuvData yuv数据回调接口
     * @param data 数据指针
     * @param size 数据长度
     * @param width 宽度
     * @param height 高度
     * @param timestamp 时长
     */
	virtual void OnYuvData(uint8_t *data, int size, int width, int height,int64_t timestamp) = 0;


    /**
     * @brief OnPlayStateChanged 播放状态回调
     * @param status 播放状态
     */
	virtual void OnPlayStateChanged(SimplePlayer::PlayStatus status) = 0;

    /**
     * @brief OnDurationChanged 总时长变化回调，第一次获取到也会回调
     * @param duration 总时长
     */
	virtual void OnDurationChanged(int64_t duration) = 0;

    /**
     * @brief OnSeekableChanged 是否可拖动回调，第一次获取到也会回调
     * @param seekable 是否可拖动
     */
	virtual void OnSeekableChanged(bool seekable) = 0;

    /**
     * @brief OnGotFirstYUV 获取到第一帧yuv(可以渲染，画面即将可见)时回调
     */
	virtual void OnGotFirstYUV() = 0;

    /**
     * @brief OnPositionChanged 播放位置变化回调，每隔0.5秒一次
     * @param position 当前播放位置
     */
	virtual void OnPositionChanged(int64_t position) = 0;

    /**
     * @brief OnComplete 播放完成回调
     */
    virtual void OnComplete() = 0;

    /**
     * @brief OnConnectStateChanged 重连状态回调
     * @param connecting true表示正在重连，false结束重连
     */
	virtual void OnConnectStateChanged(bool connecting) = 0;//connecting为true表示正在重连

    /**
     * @brief OnError 播放器内部错误
     * @param err 错误
     */
	virtual void OnError(SimplePlayer::ErrorCode err) = 0;
	//新增录像回调
    /**
     * @brief OnRecPositionChanged 边播边录输出视频时长变化回调
     * @param position 当前输出视频时长
     */
	virtual void OnRecPositionChanged(int64_t position) = 0;
	//新增对讲音频编码回调
    /**
     * @brief OnAudioEncodedData 录音数据回调
     * @param data 数据指针
     * @param size 数据长度
     * @param timestamp 时间戳
     */
	virtual void OnAudioEncodedData(uint8_t *data, int size,int timestamp) = 0;
protected:
	virtual ~SimplePlayerCallback() {}
};

} //namespace simpleplayer
#endif // SIMPLEPLAYER_H
