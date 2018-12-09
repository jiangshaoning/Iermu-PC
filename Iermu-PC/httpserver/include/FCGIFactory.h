/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once
#include "IOCPNetwork.h"
#include "FCGIRecord.h"
#include "HTTPDef.h"

/*
* ����Fast CGI ������ģʽ
* 1. ����ģʽ,��Windows��ʹ��NT�� �����ܵ�.
* 2. Զ��ģʽ,ʹ���׽���.

* FastCGI on NT will set the listener pipe HANDLE in the stdin of
* the new process.  The fact that there is a stdin and NULL handles
* for stdout and stderr tells the FastCGI process that this is a
* FastCGI process and not a CGI process.
*/

/*
* Fast CGI ���ӹ���,�������� FCGI ����,�������͹�����ģʽ�µ� FCGI ����������.
* 
* ˵��: ΪʲôҪʹ�ö����Ĺܵ���.
* ʹ�ö����Ĺܵ����ĺô���ÿ�����Ӷ�Ӧһ�� FCGI ����,��˾Ϳ��԰�ȫ�Ĺر� FCGI ���̶����õ���Ӱ�쵽��������.
*/

/*
* FCGI ����
*/
typedef struct
{
	unsigned short requestId;
	iocp_key_t comm; /* ͨѶ��� */
	bool cacheAll;
	__int64 idleTime; /* ����Ծʱ��,�ڲ�ʹ�� */
	void *instPtr; /* FCGIFactoryʵ��ָ��,�ص�����ʱ��,�ڲ�ʹ�� */
}fcgi_conn_t;

/*
* ���п�������ʱ�Ļص�����
* conn == NULL ʱ,��ʾ��ȡʧ��.
*/
typedef void (*fcgi_conn_ready_func_t)(fcgi_conn_t *conn, void *param);

/*
* ��������ֵ����
*/
const int FCGIF_SUCESS = 0;
const int FCGIF_ERROR = 1;

class FCGIFactory : public INoCopy
{
private:

	/*
	* ����FCGI���̶���
	*/
	typedef struct
	{
		PROCESS_INFORMATION *processInfo; /* ���̾�� */
		char pipeName[MAX_PATH]; /* �ý��̶�Ӧ�Ĺܵ��� */
		fcgi_conn_t *conn; /* �ý��̶�Ӧ������(һ������ֻ����һ������) */
		uintptr_t thread; /* �������̲����ӹܵ����߳� */
		FCGIFactory *instPtr;
	}fcgi_process_context_t;
	typedef std::list<fcgi_process_context_t*> fcgi_process_list_t;

	/*
	* FCGI ������̶���
	*/
	typedef struct 
	{
		char type;	/* 0: ����; 1: Զ�� */
		char name[MAX_PATH]; /* ip��ַ/����exec path */
		u_int port; /* �˿� */
		fcgi_process_list_t *processList; /* �����б� */
	}fcgi_server_context_t;

	/*
	* �ȴ�����
	*/
	typedef std::pair<fcgi_conn_ready_func_t, void *> fcgi_get_conn_context_t;
	typedef std::list<fcgi_get_conn_context_t> fcgi_get_conn_list_t;
	typedef std::list<fcgi_conn_t*> fcgi_conn_list_t;
	

	/*
	* �ڲ����ݳ�Ա
	*/
	bool _inited;
	fcgi_server_context_t *_fcgiServer; /* FCGI������Ϣ */
	fcgi_conn_list_t _workingFcgiConnList; /* ���ڹ�����FCGI���� */
	fcgi_conn_list_t _idleFcgiConnList; /* ���е�FCGI���� */
	fcgi_get_conn_list_t _waitingList; /* �ȴ��������Ӷ��� */
	unsigned short _fcgiRequestIdSeed; /* FCGI Request ID */
	size_t _maxConn; /* ��������� */
	size_t _maxWait; /* �ȴ������������������ >> _maxConn */
	std::string _fileExts; /* ��չ�� */
	bool _cacheAll;
	IOCPNetwork *_network;
	IHTTPServer *_httpServer;
	Lock _lock;
	HighResolutionTimer _hrt;

	fcgi_process_context_t* allocProcessContext();
	bool freeProcessContext(fcgi_process_context_t *context);
	fcgi_conn_t* allocConnectionContext();
	bool freeConnectionContext(fcgi_conn_t *conn);
	fcgi_process_context_t* getProcessContext(fcgi_conn_t *conn);

	bool initConnection(fcgi_conn_t *conn);
	void maintain();

	/*
	* ���ӻص�
	*/
	static void IOCPCallback(iocp_key_t s, int flags, bool result, int transfered, byte* buf, size_t len, void* param);
	static unsigned __stdcall spawnChild(void *param);
	void onConnect(fcgi_conn_t *conn, bool sucess);

public:
	FCGIFactory(IHTTPServer *httpServer, IOCPNetwork *network);
	~FCGIFactory();

	int init(const std::string &name, unsigned int port, const std::string &fileExts, size_t maxConn, size_t maxWait, bool cacheAll);
	int destroy();

	/*
	* �Ƿ����url��Ӧ������
	*/
	bool catchRequest(const std::string &fileName);

	/*
	* ��ȡFCGI����,�������ֵΪNULL,���ʾ�޷��������һ����������,�Ѿ�����ȴ�����.
	* һ�������ӽ������״̬,����ͨ�� callbackFunc �����ص�.
	*/
	bool getConnection(fcgi_conn_t *&conn, fcgi_conn_ready_func_t callbackFunc, void *param);

	/*
	* �ͷ�FCGI����,��ָ�������Ƿ���Ȼ����.
	*/
	void releaseConnection(fcgi_conn_t* conn, bool good);
};

