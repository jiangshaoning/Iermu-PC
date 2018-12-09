/** @file include/virtual_eapil_server.h
 *  @ingroup VESGroup
 *  @brief 虚拟Eapil服务库对外接口文件
 *  @date 2018-1-29
 *  @author 杨康
 */

#ifndef VIRTUALEAPILSERVER_H
#define VIRTUALEAPILSERVER_H

#include <string>
#include <vector>

#ifdef _WIN32
#   ifdef VIRTUALEAPILSERVER_EXPORT
#       define VIRTUALEAPILSERVER_LIB __declspec(dllexport)
#   else
#       define VIRTUALEAPILSERVER_LIB __declspec(dllimport)
#   endif
#else
#   define VIRTUALEAPILSERVER_LIB
#endif
class VirtualEapilServerCallback;

class VIRTUALEAPILSERVER_LIB VirtualEapilServer
{
public:
    /**
     * @brief 客户端列表类型，内部是一个std::vector<std::string>
     * 封装这个类的目的是为了让这些结构体的分配和释放在库内部
     */
	struct VIRTUALEAPILSERVER_LIB ClientListType
	{
		ClientListType();
		~ClientListType();
		std::vector<std::string> list;
	};

public:
	//创建与销毁
    /**
     * @brief Create 创建VirtualEapilServer的静态方法
     * @return 创建好的VirtualEapilServer指针
     */
	static VirtualEapilServer *Create();

    /**
     * @brief Destory 销毁VirtualEapilServer的静态方法
     * @param ves 待销毁的VirtualEapilServer指针
     */
    static void Destory(VirtualEapilServer *ves);
public:
	//参数设置接口
    /**
     * @brief SetDataSource 设置播放fileName
     * url实例:
     * 1. "eapil://192.168.31.10"
     * 2. "rtsp://192.168.31.222:554/1/h264major"
     * 3. "uvc://video=UVC Camera"
     * 4. "uvc://"
     * 5. "langtao:@2@875967048@18@928@576@0@31269@1@8000@16"
     * 6. "F:/ffmpeg/movie/bb.epv"
     * @param fileName 播放fileName
     */
	virtual bool SetDataSource(const std::string &fileName) = 0;
	//virtual void RegisterCallback(VirtualEapilServerCallback *cb) = 0;

	//流程控制接口
    /**
     * @brief Start 开始
     * @return
     */
	virtual bool Start() = 0;

    /**
     * @brief Stop 结束
     * @return
     */
	virtual bool Stop() = 0;

    /**
     * @brief Started 是否已开始
     * @return
     */
	virtual bool Started() const = 0;

    /**
     * @brief GetClientList 返回在线客户端列表
     * @param list 待修改的数据指针
     * @return 是否获取成功
     */
	virtual bool GetClientList(ClientListType *list) = 0;
public:
    virtual ~VirtualEapilServer() {}
};

class VirtualEapilServerCallback
{
public:
    /**
     * @brief OnError 错误回调
     */
	virtual void OnError() = 0;
protected:
    virtual ~VirtualEapilServerCallback() {}
};

#endif // VIRTUALEAPILSERVER_H
