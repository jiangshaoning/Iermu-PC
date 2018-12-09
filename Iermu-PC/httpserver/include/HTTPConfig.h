/* Copyright (C) 2012 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once
#include "HTTPDef.h"
#include "XmlDocument.h"

/*
* ʵ�� IHTTPConfig ��Ϊ HTTPServer �����ýӿ�
* �� XML �ļ�ʵ��.
*/

class HTTPConfig : public INoCopy, public IHTTPConfig
{
private:
	XMLDocument _xmlDoc;
	XMLHANDLE _curFcgiServerXmlHandle;

public:
	HTTPConfig();
	~HTTPConfig();

	bool load(const std::string &fileName);
	bool save(const std::string &fileName);

	// ����
	bool setDocRoot(const std::string &str);
	bool setTmpRoot(const std::string &str);
	bool setDefaultFileNames(const std::string &str);
	bool setIp(const std::string &str);
	bool setPort(u_short p);
	bool setDirVisible(BOOL visible);
	bool setMaxConnections(size_t n);
	bool setMaxConnectionsPerIp(size_t n);
	bool setMaxConnectionSpeed(size_t n);
	bool setSessionTimeout(size_t n);
	bool setRecvTimeout(size_t n);
	bool setSendTimeout(size_t n);
	bool setKeepAliveTimeout(size_t n);
	bool addFcgiServer(const fcgi_server_t *serverInf);
	bool removeFcgiServer(const std::string &name);
	bool updateFcgiServer(const std::string &name, const fcgi_server_t *serverInf);

	bool setAutoRun(BOOL yes);
	bool autoRun();

	bool screenLog();
	bool enableScreenLog(bool enabled);

	std::string logFileName();
	bool setLogFileName(const std::string &fileName);

	//slogger::LogLevel logLevel();
	//bool setLogLevel(slogger::LogLevel ll);

	//  ͨ�÷���
	bool set(const std::string &path, const std::string &v);
	bool get(const std::string &path, std::string &v);

	// IHTTPConfig ʵ��
	std::string docRoot();
	std::string tmpRoot();
	std::string defaultFileNames();
	std::string ip();
	u_short port();
	bool dirVisible();
	size_t maxConnections();
	size_t maxConnectionsPerIp();
	size_t maxConnectionSpeed();
	size_t sessionTimeout();
	size_t recvTimeout();
	size_t sendTimeout();
	size_t keepAliveTimeout();

	bool getFirstFcgiServer(fcgi_server_t *serverInf);
	bool getNextFcgiServer(fcgi_server_t *serverInf);
};

