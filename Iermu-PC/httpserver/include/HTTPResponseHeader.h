/* Copyright (C) 2012 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/


#pragma once
#include <string>
#include <list>
#include "HTTPDef.h"
#include "memfile.h"

/*
* HTTPResponseHeader �������� HTTP ��Ӧͷ
*/

class HTTPResponseHeader : public INoCopy
{
private:
	typedef std::list<std::pair<std::string, std::string>> fields_t;
	fields_t _headers;	// ��Ӧͷ�Ĺ�������
	memfile _buf; // �������
	int _resCode; // ��Ӧ��

	std::string getFirstLine(); // �����ƶ��� Servercode ���ض�Ӧ�� HTTP��Ӧͷ�ĵ�һ��,�������з�.
	size_t write(const std::string &str);
	fields_t::iterator find(const std::string &name);

public:
	HTTPResponseHeader();
	~HTTPResponseHeader();

	int setResponseCode(int resCode); // ������Ӧ��,���ؾ�ֵ
	size_t addDefaultFields(); // ���Ĭ����
	bool add(const std::string &name, const std::string &val); // �����
	bool add(const std::string &fields); // �����,����Ϊ��λ,һ�ο���Ӷ����
	bool remove(const std::string &name); // ɾ����
	bool getField(const std::string &name, std::string &val);
	std::string getHeader();

	bool format(); // ��ʽ��������
	size_t read(byte *buf, size_t len); // ��ȡ������
	bool eof(); // ��ȡ�Ƿ����
	void reset(); // ����
};

