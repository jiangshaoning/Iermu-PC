/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#if !defined(_HTTPFILESERVER_H_)
#define _HTTPFILESERVER_H_

#include "winsock2.h"
#pragma comment(lib, "ws2_32.lib")


#include "HTTPDef.h"
#include "FCGIFactory.h"

/*
* HTTPServer��
* Ŀ��: ����һ��HTTP Server���ڴ��еĴ���.
* 1. ���������׽���.
* 2. ����HTTP���Ӱ��� HTTPRequest, HTTPResponder ��,���������ӽ���ʱ���� HTTPRequest ���󲢵���������ں��� run()
*	 ����������������ӵĴ������.
* 3. �ṩ������ȡ������.
* 
* by ������ - Que's C++ Studio
* 2011.7.7
*/

class HTTPServer : public IHTTPServer, public INoCopy
{
protected:
	/*
	* �����׽��������¼�
	*/
	typedef struct
	{
		byte* buf;
		size_t len;
	}accept_context_t;

	/*
	* ���Ӷ���
	*/
	typedef struct			
	{
		iocp_key_t		clientSock;				// �ͻ����׽���
		IRequest*		request;				// HTTP����������
		IResponder*		responder;				// HTTP��Ӧ������

		__int64			startTime;				// ���ӿ�ʼʱ��ʱ��
		char			ip[MAX_IP_LENGTH + 1];	// �ͻ�������IP��ַ
		unsigned int	port;					// �ͻ������Ӷ˿�
	}connection_context_t;
	typedef std::map<iocp_key_t, connection_context_t*> connection_map_t;

	/*
	* ��������
	*/
	std::string _docRoot; /*��Ŀ¼*/
	std::string _tmpRoot; /* ��ʱ�ļ���Ŀ¼ */
	bool _isDirectoryVisible; /*�Ƿ��������Ŀ¼*/
	std::string _dftFileName; /*Ĭ���ļ���*/
	std::string _ip; /*������IP��ַ*/
	u_short _port; /*�����������˿�*/
	size_t _maxConnections; /*���������*/
	size_t _maxConnectionsPerIp; /*ÿ��IP�����������*/
	size_t _maxConnectionSpeed; /*ÿ�����ӵ��ٶ�����,��λ b/s.*/

	unsigned long _sessionTimeout; /*�Ự��ʱ*/
	unsigned long _recvTimeout; /*recv, connect, accept �����ĳ�ʱ*/
	unsigned long _sendTimeout; /*send �����ĳ�ʱ*/
	unsigned long _keepAliveTimeout; /* keep-alive ��ʱ */

	/*
	* �ڲ�����
	*/
	bool _isRuning;
	HighResolutionTimer _hrt;
	FCGIFactory *_fcgiFactory;
	IOCPNetwork _network;
	iocp_key_t _sListen;
	SOCKET _sockNewClient;
	accept_context_t _acceptContext;
	connection_map_t _connections; /* �ͻ���Ϣ�б�,ÿ������(�ͻ�)��Ӧһ����¼(connection_context_t*)ָ�� */
	str_int_map_t _connectionIps; /* �ͻ���IP��ַ��(ÿIP��Ӧһ����¼,��������ÿ�ͻ������������ */
	IHTTPServerStatusHandler *_statusHandler; /*״̬�ص��ӿ�,ʵ������ӿڿ��Ի�÷����������е�״̬ */
	Lock _lock; /* Windowsͬ�����ƶ��� */
	int _tmpFileNameNo; /* ��ʱ�ļ������ */
	char _tmpFileNamePre[5]; /* ��ʱ�ļ�����ǰ׺(����**������**������HTTPServer����ͬһ����ʱĿ¼ʱ��������ͻ��ʹϵͳ�����������) */
	 
	/*
	* �ڲ�ʹ�õĹ��ߺ���.
	*/
	connection_context_t* allocConnectionContext(const std::string &strIP, unsigned int nPort); /*��ʼ��һ���ͻ�������.*/
	void freeConnectionContext(connection_context_t* client);	/*���տͻ�������.*/
	int initListenSocket(const std::string& strIP, int nPort, SOCKET& hListenSock); /*��ʼ�������׽���*/
	void doStop();	/*���շ�������Դ,��Run()����ʧ�ܵ�����µ���.*/
	int doAccept(); /*����һ���µ��׽���,��ִ�е���AcceptEx*/
	void doRequestDone(connection_context_t* conn, int status);
	void doConnectionClosed(connection_context_t* conn, int status);

	/*
	* IOCPCallback ���յ������¼�����ݲ�������,�ɷ������崦�������¼��ĺ���.
	*/
	static void IOCPCallback(iocp_key_t s, int flags, bool result, int transfered, byte* buf, size_t len, void* param);
	void onAccept(bool sucess);
	
public:
	HTTPServer();
	~HTTPServer();

	/*
	* ������Ϣ
	*/
	bool mapServerFilePath(const std::string& url, std::string& serverPath);
	std::string tmpFileName();
	std::string jpgFileName(std::string& deviceIDdir, std::string& filedir);
	inline const std::string& docRoot() { return _docRoot; }
	inline bool isDirectoryVisible() { return _isDirectoryVisible; }
	inline const std::string& defaultFileNames() { return _dftFileName; }
	inline const std::string& ip() { return _ip; }
	inline u_short port() { return _port; }
	inline size_t maxConnectionsPerIp() { return _maxConnectionsPerIp; }
	inline size_t maxConnections() { return _maxConnections; }
	inline size_t maxConnectionSpeed() { return _maxConnectionSpeed; }
	inline unsigned long sessionTimeout() { return _sessionTimeout; }
	inline unsigned long recvTimeout() { return _recvTimeout; }
	inline unsigned long sendTimeout() { return _sendTimeout; }
	inline unsigned long keepAliveTimeout() { return _keepAliveTimeout; }

	/*
	* �ص�����
	*/
	virtual int onRequestDataReceived(IRequest* request, size_t bytesTransfered);
	virtual int onResponderDataSent(IResponder *responder, size_t bytesTransfered);
	virtual void onRequest(IRequest* request, int status);
	virtual void onResponder(IResponder *responder, int status);

	/* 
	* ��������ֹͣ������
	*/
	int run(IHTTPConfig *conf, IHTTPServerStatusHandler *statusHandler);
	int stop();
	inline bool runing() { return _isRuning; }
};

#endif
