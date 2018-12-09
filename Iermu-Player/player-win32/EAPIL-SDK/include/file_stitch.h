/** @file include/file_stitch.h
 *  @ingroup FileStitchGroup
 *  @brief 文件拼接库对外接口文件
 *  @details 提供文件拼接库的所有接口
 *  @date 2018-1-29
 *  @author 杨康
 */

#ifndef FILE_STITCH_H
#define FILE_STITCH_H
#include <string>

#ifdef _WIN32
#   ifdef FILE_STITCH_EXPORT
#       define FILE_STITCH_LIB __declspec(dllexport)
#   else
#       define FILE_STITCH_LIB __declspec(dllimport)
#   endif
#else
#   define FILE_STITCH_LIB
#endif

class FileStitchCallback;
class StitchAVFrame;

class FILE_STITCH_LIB FileStitch
{
public:
	//创建与销毁
    /**
     * @brief Create FileStitch指针静态创建函数
     * @return FileStitch指针
     */
	static FileStitch *Create();

    /**
     * @brief Destory FileStitch指针静态销毁函数
     * @param file_stitch FileStitch指针
     */
    static void Destory(FileStitch *file_stitch);
public:

    /**
     * @brief SetParams   设置参数
     * @param input_file  输入文件
     * @param output_file 输出文件
     * @param dst_width   目标宽度
     * @param dst_height  目标高度
     * @param cb          回调函数
     * @return 是否设置成功
     */
	virtual bool SetParams(
				const char *input_file,
				const char *output_file,
				const int dst_width,
				const int dst_height,
				FileStitchCallback *cb) = 0;

    /**
     * @brief SetParams   设置参数
     * @param input_file  输入文件
     * @param output_file 输出文件
     * @param dst_width   目标宽度
     * @param dst_height  目标高度
     * @param dst_bitrate 目标码率
     * @param cb          回调函数
     * @return 是否设置成功
     */
	virtual bool SetParams(
		const char *input_file,
		const char *output_file,
		const int dst_width,
		const int dst_height,
		const int dst_bitrate,
		FileStitchCallback *cb) = 0;

    /**
     * @brief Start 开始,设置好参数之后调用
     * @return 调用是否成功
     */
    virtual bool Start() = 0;

    /**
     * @brief Stop 结束，调用之前请用@ref FileStitch::IsBufferEmpty() 确定内部缓存已全部处理完成
     */
	virtual void Stop() = 0;

    /**
     * @brief Started
     * @return 返回是否开始
     */
	virtual bool Started() const = 0;

    /**
     * @brief IsBufferEmpty 确定内部缓存已全部处理完成
     * @return
     */
	virtual bool IsBufferEmpty() const = 0;

    /**
     * @brief GetPosition 当前输出的视频文件长度
     * @return
     */
	virtual int64_t GetPosition() const = 0;


    /**
     * @brief PushAVData 发现处理过了的音视频帧
     * @param yuv
     */
	virtual void PushAVData(const StitchAVFrame &yuv) = 0;
public:
    virtual ~FileStitch() {}
};

/**
 * @brief 解码后音视频帧的封装类
 */
struct StitchAVFrame
{
    /**
     * @brief type 0是视频，1是音频
     */
	int type; //0是视频，1是音频

    /**
     * @brief data 数据指针
     */
	unsigned char *data;

    /**
     * @brief len 数据长度
     */
	int len;

    /**
     * @brief timestamp 时间戳
     */
	long long timestamp;

    /**
     * @brief width 宽度
     */
	int width;

    /**
     * @brief height 高度
     */
	int height;
};

/**
 * @brief FileStitch接口的回调
 */
class FileStitchCallback
{
public:
    /**
     * @brief OnGetAVData 解码得到音视频帧的回调
     * @param yuv 帧数据
     */
	virtual void OnGetAVData(StitchAVFrame yuv) = 0;

    /**
     * @brief OnReadFinish 解复用和解码结束
     */
	virtual void OnReadFinish() = 0;

    /**
     * @brief OnError 错误回调
     * @param code 错误码，目前没有用到具体的错误码，一旦出错客户端应该 @ref FileStitch::Stop()
     */
	virtual void OnError(int code) = 0;
protected:
	virtual ~FileStitchCallback() {}
};
#endif // FILE_STITCH_H
