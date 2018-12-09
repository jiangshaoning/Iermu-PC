/** @file include/simple_recorder.h
 *  @ingroup SimpleRecoderGroup
 *  @brief SimpleRecoder录像库对外接口文件
 *  @details 提供录像库的所有接口
 *  @date 2018-1-29
 *  @author 杨康
 */

#ifndef SIMPLE_RECORDER_H
#define SIMPLE_RECORDER_H
#include <string>

#include "../types.h"

#ifdef _WIN32
#   ifdef SIMPLERECORDER_EXPORT
#       define SIMPLERECORDER_LIB __declspec(dllexport)
#   else
#       define SIMPLERECORDER_LIB __declspec(dllimport)
#   endif
#else
#   define SIMPLERECORDER_LIB
#endif
namespace simpleplayer
{
class SimpleRecorderCallback;

class SimpleRecorder
{
public:
    /**
     * @brief 视频类型枚举
     */
	enum AV_VIDEO_TYPE
	{
		kAV_VIDEO_NONE = 0,

		//video codeID
		kAV_DATA_H264,
		kAV_DATA_H265,

		//raw video type
		kAV_DATA_YUV420,//不能有多余的对齐字节
		kAV_DATA_RGB24,
		kAV_DATA_NV12
	};


    /**
     * @brief 音频类型枚举
     */
	enum AV_AUDIO_TYPE
	{
		kAV_AUDIO_NONE,

		//audio codeID
		kAV_DATA_AAC,
		kAV_DATA_ALAW,      //音频G711 Alaw
		kAV_DATA_MULAW,     //音频G711 Ulaw
		kAV_DATA_G722,      //音频G722
		kAV_DATA_G726,      //音频G726
		//kAV_DATA_PCM16,     //音频8位PCM

		//raw audio
		kAV_DATA_PCM         //16位pcm
	};

    /**
     * @brief 视频参数
     */
	struct VideoParm
	{
        /**
         * @brief srcType 类型
         */
		AV_VIDEO_TYPE srcType;
		int srcWidth;
		int srcHeight;
		int srcFps;
        /**
         * @brief srcExtraData sps-pps
         */
		uint8_t srcExtraData[128];

        /**
         * @brief srcExtraDataLen sps-pps长度
         */
        int srcExtraDataLen;

        /**
         * @brief dstBitrate 码率
         */
		int dstBitrate;
		VideoParm()
		{
			srcType = kAV_VIDEO_NONE;
			srcWidth = 0 ;
			srcHeight = 0;
			srcFps = 0;
			memset(srcExtraData,0,128);
			srcExtraDataLen = 0;
			dstBitrate = 1024 * 1024;
		}
	};
    /**
     * @brief 音频参数
     */
	struct AudioParm
	{
        /**
         * @brief srcType 音频类型
         */
		AV_AUDIO_TYPE srcType;

        /**
         * @brief srcSampleRate 采样率
         */
        int srcSampleRate;

        /**
         * @brief srcChannel 声道数
         */
        int srcChannel;
		AudioParm()
		{
			srcType = kAV_AUDIO_NONE;
			srcSampleRate = 0;
			srcChannel = 0;
		}
	};

public:
	//创建与销毁
    /**
     * @brief Create 创建SimpleRecorder的静态方法
     * @return 创建完成后的指针
     */
	static SimpleRecorder *Create();

    /**
     * @brief Destory 销毁SimpleRecorder的静态方法
     * @param sr 待销毁的指针
     */
    static void Destory(SimpleRecorder *sr);
public:
	//参数设置接口
    /**
     * @brief SetDataDestination 设置目标地址
     * @param fileName 本地文件名，也可以是远程服务推流地址
     * @param auto_reconnect 自动重连，默认true
     * @return 是否设置成功
     */
	virtual bool SetDataDestination(const std::string &fileName, bool auto_reconnect = true) = 0;

    /**
     * @brief RegisterCallback 注册回调
     * @param cb 回调类型指针
     */
    virtual void RegisterCallback(SimpleRecorderCallback *cb) = 0;

    /**
     * @brief AddVieoStream 设置录像数据的视频参数
     * @param video_param 视频参数
     * @return 是否成功
     */
    virtual bool AddVieoStream(const VideoParm &video_param) = 0;

    /**
     * @brief AddAudioStream 设置录像数据的音频参数
     * @param audio_param 音频参数
     * @return 是否成功
     */
    virtual bool AddAudioStream(const AudioParm &audio_param) = 0;

    /**
     * @brief AddMetaInfo 添加meta信息，可添加多个
     * @param key 键
     * @param val 值
     */
    virtual void AddMetaInfo(const std::string &key, const std::string &val) = 0;

    /**
     * @brief RemoveMetaInfo 删除指定的meta item
     * @param key 带删除的mete item的键
     */
    virtual void RemoveMetaInfo(const std::string &key) = 0;

	//流程控制接口
    /**
     * @brief Start 开始
     * @return 调用是否成功
     */
	virtual bool Start() = 0;

    /**
     * @brief Stop 结束
     * @return 调用是否成功
     */
	virtual bool Stop() = 0;

    /**
     * @brief Started 是否已经开始
     * @return
     */
	virtual bool Started() const = 0;

    /**
     * @brief IsBufferEmpty 获取内部缓存是否已经处理完成
     * @return 是否已经处理完成
     */
	virtual bool IsBufferEmpty()  = 0;

    /**
     * @brief GetPosition 获取当前音视频输出时长
     * @return  输出时长
     */
    virtual int64_t GetPosition() const = 0;

	//放入数据接口
    /**
     * @brief PushVideoData 放入视频帧数据
     * @param data 数据指针
     * @param len 数据长度
     * @param timestamp 时间戳
     * @param dts 解码时间戳
     * @param iframe 是否是关键帧
     * @param position 获取当前输出位置的参数指针
     * @return 调用是否成功
     */
	virtual bool PushVideoData(const uint8_t *data, int len, int64_t timestamp, int64_t dts, bool iframe, int *position = NULL) = 0;

    /**
     * @brief PushAudioData 放入音频数据
     * @param data 数据指针
     * @param len 数据长度
     * @param timestamp 时间戳
     * @param position 获取当前输出位置的参数指针
     * @return 调用是否成功
     */
    virtual bool PushAudioData(const uint8_t *data, int len, int64_t timestamp, int *position = NULL) = 0;

public:
    virtual ~SimpleRecorder() {}
};

class SimpleRecorderCallback
{
public:
    /**
     * @brief OnOutputError 出错回调
     */
	virtual void OnOutputError() = 0;
protected:
    virtual ~SimpleRecorderCallback() {}
};
} //namespace simpleplayer

#endif // SIMPLE_RECORDER_H
