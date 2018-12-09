/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once

#include "fastcgi.h"
#include "ATW.h"
#include "WINFile.h"
#include "IOCPNetwork.h"


#define MAX_WINIO 16384 // ϵͳ��������ͬʱʹ�� WINFile() �򿪵��ļ���
#define G_BYTES (1024 * 1024 * 1024) // 1GB
#define M_BYTES (1024 * 1024)		 // 1MB
#define K_BYTES 1024				 // 1KB
#define MAX_SOCKBUFF_SIZE 4096 // ���ջ�����. 
#define FCGI_BUFFER_SIZE 4096 // ����С�� 24, ���� FCGI ����������. һ��Ӧ�ñ�֤��512����.
#define MAX_METHODSIZE 100 // ���ڱ���HTTP�����ַ����ĳ���,���˳��� 200
#define MAX_REQUESTHEADERSIZE (K_BYTES * 10) // �������ͷ��������
#define MAX_IP_LENGTH 50 // IP��ַ����󳤶�
#define MIN_SIZE_ONSPEEDLIMITED 512 // �ﵽ�ٶ�����ʱ,���͵���С���ֽ���.
#define MAX_WAITTIME_ONSPEEDLIMITED 2000 // �ﵽ�ٶ�����ʱ,���ȴ����ٺ��뷢��һ����.���ֵ������õù���,�п��ܵ��¿ͻ������������û��Ӧ
#define ETAG_BYTE_SIZE 5 // �����ڴ�����,����ETagʱ��ȡ���ֽ���.
#define FCGI_CONNECT_TIMEO 5000 // ����FCGI�����ܵ��ĳ�ʱʱ��
#define FCGI_MAX_IDLE_SECONDS 5
#define SERVER_SOFTWARE "Q++ HTTP Server/0.20"
#define MAX_POST_DATA (G_BYTES) // ������1G
#define POST_DATA_CACHE_SIZE (1 * K_BYTES) // ����1KB��POST DATA����д���ļ�
#define FCGI_CACHE_SIZE (16 * K_BYTES)
#define FCGI_PIPE_BASENAME "\\\\.\\pipe\\fast_cgi_ques" // FCGI �����ܵ���

// ��������Ӧ��
#define SC_UNKNOWN 0
#define	SC_OK 200
#define	SC_NOCONTENT 204
#define	SC_PARTIAL 206
#define SC_OBJMOVED 302
#define	SC_BADREQUEST 400
#define	SC_FORBIDDEN 403
#define	SC_NOTFOUND 404
#define	SC_BADMETHOD 405
#define	SC_SERVERERROR 500
#define SC_SERVERBUSY 503

// ����ֵ����(�������Ͷ���)
#define SE_SUCCESS 0
#define SE_RUNING 1 // ��������
#define SE_STOPPED 2 // �Ѿ�ֹͣ
#define SE_NETWORKFAILD 3
#define SE_CREATESOCK_FAILED 100 // �׽��ִ���ʧ��
#define SE_BIND_FAILED 101 // �󶨶˿�ʧ��
#define SE_LISTEN_FAILED 102 // listen() ��������ʧ��.
#define SE_CREATETIMER_FAILED 103 // �޷�������ʱ��
#define SE_CREATE_IOCP_FAILED 104
#define SE_INVALID_PARAM 105
#define SE_UNKNOWN 1000


// HTTP ���Ӷ����˳���
typedef enum HTTP_CLOSE_TYPE
{
	CT_SUCESS = 0,

	CT_CLIENTCLOSED = 10, // �ͻ��˹ر�������
	CT_SENDCOMPLETE, // �������
	CT_SEND_TIMEO,
	CT_RECV_TIMEO,
	CT_SESSION_TIMEO,
	CT_BADREQUEST,
	
	CT_FCGI_SERVERERROR = 20,
	CT_FCGI_CONNECT_FAILED,
	CT_FCGI_SEND_FAILED,
	CT_FCGI_RECV_FAILED,
	CT_FCGI_RECV_TIMEO,
	CT_FCGI_SEND_TIMEO,
	CT_FCGI_ABORT,

	CT_NETWORK_FAILED = 50,
	CT_INTERNAL_ERROR,

	CT_UNKNOWN = 999 // δ֪.	
}HTTP_CONNECTION_EXITCODE;

// HTTP ����
enum HTTP_METHOD
{
	METHOD_UNDEFINE = 0,
	METHOD_GET = 1,
	METHOD_POST,
	METHOD_PUT,
	METHOD_HEAD, // ֻ������Ӧͷ
	METHOD_DELETE, // ɾ��
	METHOD_TRACE,
	METHOD_CONNECT,

	METHOD_UNKNOWN = 100
};

// Fast CGI ����������
typedef struct 
{
	char name[MAX_PATH + 1];
	bool status;
	char path[MAX_PATH + 1]; // ip��ַ(Զ��ģʽ)����������(����ģʽ)
	u_short port; // �˿�. 0��ʾ�Ǳ���ģʽ
	char exts[MAX_PATH + 1]; // �ļ���չ��,���ŷָ�
	size_t maxConnections; // ���������
	size_t maxWaitListSize; // �ȴ����д�С
	bool cacheAll;	// �Ƿ񻺴�����
}fcgi_server_t;

typedef std::map<std::string, unsigned int> str_int_map_t;
typedef std::vector<std::string> str_vec_t;

// �ⲿ������ַ���
extern const char* g_HTTP_Content_NotFound;
extern const char* g_HTTP_Bad_Request;
extern const char* g_HTTP_Bad_Method;
extern const char* g_HTTP_Server_Error;
extern const char* g_HTTP_Forbidden;
extern const char* g_HTTP_Server_Busy;

// ��һ��ʱ���ʽ��Ϊ HTTP Ҫ���ʱ���ʽ(GMT).
std::string format_http_date(__int64* ltime);
std::string to_hex(const unsigned char* pData, int nSize);
std::string decode_url(const std::string& inputStr);
bool map_method(HTTP_METHOD md, char *str);
bool is_end(const byte *data, size_t len);
std::string get_field(const char* buf, const char* key);
void get_file_ext(const std::string &fileName, std::string &ext);
bool match_file_ext(const std::string &ext, const std::string &extList);
std::string get_last_error(DWORD errCode = 0);
size_t split_strings(const std::string &str, str_vec_t &vec, const std::string &sp);
bool get_ip_address(std::string& str);
std::string format_size(__int64 bytes);
std::string format_speed(__int64 bytes, unsigned int timeUsed);

/*
* HTTP ���ýӿ�
*/
class IHTTPConfig
{
public:
	IHTTPConfig() {};
	virtual ~IHTTPConfig() {};

	virtual std::string docRoot() = 0;
	virtual std::string tmpRoot() = 0;
	virtual std::string defaultFileNames() = 0;
	virtual std::string ip() = 0;
	virtual u_short port() = 0;
	virtual bool dirVisible() = 0;
	virtual size_t maxConnections() = 0;
	virtual size_t maxConnectionsPerIp() = 0;
	virtual size_t maxConnectionSpeed() = 0;
	virtual size_t sessionTimeout() = 0;
	virtual size_t recvTimeout() = 0;
	virtual size_t sendTimeout() = 0;
	virtual size_t keepAliveTimeout() = 0;
	virtual bool getFirstFcgiServer(fcgi_server_t *serverInf) = 0;
	virtual bool getNextFcgiServer(fcgi_server_t *serverInf) = 0;
};

/*
* HTTP Server �ĳ���ӿ�
*/
typedef void* conn_id_t;
const conn_id_t INVALID_CONNID = NULL;
class IRequest;
class IResponder;
class IHTTPServer
{
public:
	virtual ~IHTTPServer() {};

	/*
	* �ص������ӿ�
	*/
	virtual int onRequestDataReceived(IRequest* request, size_t bytesTransfered) = 0;
	virtual int onResponderDataSent(IResponder *responder,size_t bytesTransfered) = 0;
	virtual void onRequest(IRequest* request, int status) = 0;
	virtual void onResponder(IResponder *responder, int status) = 0;

	/*
	* ��ȡSERVER��Ϣ
	*/
	virtual bool mapServerFilePath(const std::string& url, std::string& serverPath) = 0;
	virtual std::string tmpFileName() = 0;
	virtual std::string jpgFileName(std::string& deviceIDdir, std::string& filedir) = 0;
	virtual const std::string& docRoot() = 0;
	virtual bool isDirectoryVisible() = 0;
	virtual const std::string& defaultFileNames() = 0;
	virtual const std::string& ip() = 0;
	virtual u_short port() = 0;
	virtual size_t maxConnectionsPerIp() = 0;
	virtual size_t maxConnections() = 0;
	virtual size_t maxConnectionSpeed() = 0;
	virtual unsigned long sessionTimeout() = 0;
	virtual unsigned long recvTimeout() = 0;
	virtual unsigned long sendTimeout() = 0;
	virtual unsigned long keepAliveTimeout() = 0;
};

class IRequest
{
public:
	virtual ~IRequest(){};

	/* �������� */
	virtual conn_id_t getConnectionId() = 0;
	virtual size_t getTotalRecvBytes() = 0;
	virtual HTTP_METHOD method() = 0;
	virtual std::string uri(bool decode) = 0;
	virtual std::string field(const char* key) = 0;
	virtual bool keepAlive() = 0;
	virtual bool range(__int64 &from, __int64 &to) = 0;
	virtual bool isValid() = 0;
	virtual bool isBoundary() = 0;
	virtual size_t headerSize() = 0;
	virtual size_t size() = 0;
	virtual size_t contentLength() = 0;
	virtual __int64 startTime() = 0;

	/* ��������ͷ */
	virtual std::string getHeader() = 0;

	/* ��ȡ POST DATA */
	virtual size_t read(byte* buf, size_t len) = 0;
	virtual bool eof() = 0;

	/* �������� */
	virtual int run(conn_id_t connId, iocp_key_t clientSock, size_t timeout) = 0;
	virtual bool stop(int ec) = 0;
	virtual bool reset() = 0;
};

class IResponder
{
public:
	virtual ~IResponder() {};

	/* �������� */
	virtual conn_id_t getConnectionId() = 0;
	virtual __int64 getTotalSentBytes() = 0;
	virtual __int64 getTotalRecvBytes() = 0;
	virtual int getServerCode() = 0;

	/* ������Ӧͷ */
	virtual std::string getHeader() = 0;

	/* �������� */
	virtual int run(conn_id_t connId, iocp_key_t clientSock, IRequest *request) = 0;
	virtual bool stop(int ec) = 0;
	virtual bool reset() = 0;
};

// ������״̬��Ϣ�ӽӿ�
// HTTP�������������ڼ���������ӿڵķ�����ʹ�ýӿڵ�ʵ�ֿ��Ի�ȡʱʱ��HTTP������
class IHTTPServerStatusHandler
{
public:
	// ��������ʱ����,�ɲ��� bRefused ��ʶ�Ƿ񱻷������ܾ�.
	virtual void onNewConnection(const char *ip, unsigned int port, bool refused, bool kicked) = 0;
	virtual void onConnectionClosed(const char *ip, unsigned int port, HTTP_CLOSE_TYPE rr) = 0;

	// ���ݷ������ʱ����
	virtual void onDataSent(const char *ip, unsigned int port, unsigned int bytesSent) = 0;
	virtual void onDataReceived(const char *ip, unsigned int port, unsigned int bytesReceived) = 0;

	// ����ͻ�������,�ڷ��ͻ�Ӧ���ͻ���ǰ����.
	virtual void onRequestBegin(const char *ip, unsigned int port, const char *url, HTTP_METHOD hm) = 0;
	virtual void onRequestEnd(const char *ip, unsigned int port, const char *url, int svrCode, __int64 bytesSent, __int64 bytesRecved, unsigned int timeUsed, bool completed) = 0;
};