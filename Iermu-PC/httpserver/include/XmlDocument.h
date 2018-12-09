// XmlProcess.h: interface for the XMLDocument class.
//
// ��Ȩ����: �����޸Ļ���������ҵ��;,���뱣��ԭ���ߵ������Ϣ.
// ����: ������ (querw) / Que's C++ Studio
// ����: 2006����
//
// Update list
//
// �����������,��Ҫ�޸�����:
//
// 1. ��� XPath ��򵥵�֧��,����ֱ�Ӷ�λ. GetNode(), GetChildByPath(), AppendNode(path)
//
// 2. �����ط��ĵݹ���»���� 1. XmlNode::~XmlNode() �ݹ�ɾ�������ӽڵ�; 2. XmlNode::LoadNode() �ݹ鴴�����е��ӽڵ�; 3. GetNode()
//
// 3. ��� et_positionholder ���͵Ľڵ�,���ڱ���ԭʼxmlԴ�еĿհ��ַ�,�������,ͬ��Ҳ�����һЩ����,��Ϊռλ�ڵ������ӽڵ�����ڸ��ڵ�Ķ�����
//    ����,����ӽڵ��ʱ��Ҫע����˴���ڵ�. ����, xml Э��ڵ�֮��,���ڵ�֮ǰ�Ŀհ��ַ��͸��ڵ�֮��Ŀհ��ַ����Ǳ���ȥ,��Ϊû���κ�����.
//
// 4. ����һ�� xml �ĵ�ֻ������1�����ڵ�
//
// 5. Ӧ��˵�ڴ���(��xml�ļ��������ڴ汣���һ�Ŷ����)ʹ��utf-8���������ŵ�,���ǿ��ǵ�Windowsƽ̨��ʹ�ý϶�,��Windowsƽֱ̨����UTF-16
//	  ������ַ���,����û��ȥ�޸�. ���� XMLNode �ķ������� LoadNode �����ױ�����Ϊ���Դ��� char �͵��ַ���.
//
// - ����: �������⻹��ʹ����̫����ڴ�,����������ʱ, �����ļ���Ҫһ��������, ת��Ϊ���ַ���Ҫһ��������, ����Ϊ�ڴ�������Ҫ�ڴ�.
//
//
// 2012-4-11 by ������
//
//////////////////////////////////////////////////////////////////////

#if !defined(_XMLDOCUMENT_H_)
#define _XMLDOCUMENT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma warning(disable : 4786)
#include <string>
#include <list>

enum xmlnode_type
{
	et_none,		// ����ʱ��ȷ��,��������ݾ���
	et_xml,			// <?xml ...?>
	et_comment,		// <!-- ... -->
	et_normal,		// <tag />
	et_text,		// content text
	et_cdata,		// <![CDATA[ ... ]]>
	et_positionholder, // �հ�ռλ��,������ʽ�� TAB ��.
};

typedef const wchar_t cwchar_t;
typedef std::pair<std::string, std::string> str2str;
typedef std::list<str2str> list_str2str;
typedef list_str2str::iterator iter_str2str;

class XMLNode;
typedef XMLNode* XMLHANDLE;
typedef std::list<XMLNode*> list_nodeptr;
typedef list_nodeptr::iterator iter_nodeptr;

class XMLDocument
{
protected:
	/*
	* ����ָ������ͬʱ��ЧͬʱʧЧ
	*/
	XMLHANDLE m_hXmlRoot; // xml Э��ڵ�,���͵� <?xml version="1.0" encoding="utf-8"?>
	XMLHANDLE m_hRoot; // ���ڵ�,һ�� xml �ĵ�ֻ����һ�����ڵ�.

private:
	/*
	* ��ֹ����
	*/
	XMLDocument(const XMLDocument&);
	XMLDocument& operator = (const XMLDocument&);

public:
	XMLDocument();
	virtual ~XMLDocument();

	XMLHANDLE Build(const char *pRoot, const char* pVersion, const char* pEncode); // ���� xml ��,���ظ��ڵ�.
	bool Load(const char* pszBuffer, int nLen, bool bKeepBlank = false);	// ��������XML�ı����ж���(���뷽ʽ���ַ�����ָ��)
	bool Load(cwchar_t *pwszBuffer, int nLen, bool bKeepBlank = false);		// ��һ�����ַ����ж���������
	bool Load(const char* pszFileName, bool bKeepBlank = false);		// ��һ���ļ��ж���������
	int GetString(XMLHANDLE hXml, char *pBuffer, int nLen); // hXml = NULL ��ʾ����������(���뷽ʽ�ڸ��ڵ���ָ��) ��hXml!=NULLʱ,����ANSI��
	bool Save(const char*pszFileName);	// ���浽�ļ�
	bool Destroy();

	XMLHANDLE GetXmlRoot();	// ����XMLЭ��ڵ�
	XMLHANDLE GetRootNode();	// ���ظ��ڵ�
	XMLHANDLE GetNode(const char *pszPath, bool bCreateNew = false); // ���� XPath ��ʾ�Ľڵ�
	XMLHANDLE AppendNode(XMLHANDLE hParent, const char* pName, xmlnode_type type = et_normal);
	XMLHANDLE AppendNode(XMLHANDLE hParent, const char *pBuffer, int nLen, bool bKeepBlank = false); // �½ڵ��һ��ANSI������
	bool DeleteNode(XMLHANDLE hXml);

	std::string GetName(XMLHANDLE hXml);
	bool SetName(XMLHANDLE hXml, const char *pszName);
	std::string GetAttrValue(XMLHANDLE hXml, const char *pszAttr);
	bool SetAttrValue(XMLHANDLE hXml, const char *pszAttr, const char *pszValue);
	bool GetAttrList(XMLHANDLE hXml, list_str2str* pList);
	xmlnode_type GetType(XMLHANDLE hXml);
	std::string GetText(XMLHANDLE hXml);
	bool SetText(XMLHANDLE hXml, const char *pszText); // ��ȡ��ǰ�ڵ�� m_strText ֵ, ����Ϊ ct_comment , ct_cdata, ct_text 
	std::string GetContent(XMLHANDLE hXml, XMLHANDLE *phXml = NULL); // ���ǻ�ȡ��һ������Ϊ ct_text ���ӽڵ������ <tag>content</tag>
	XMLHANDLE SetContent(XMLHANDLE hXml, const char *pszContent);
	
	XMLHANDLE FirstChild(XMLHANDLE hXml);
	XMLHANDLE NextSibling(XMLHANDLE hXml);
	XMLHANDLE PrevSibling(XMLHANDLE hXml);
	XMLHANDLE Parent(XMLHANDLE hXml);

	XMLHANDLE GetChildByName(XMLHANDLE hXml, const char *pszName);
	XMLHANDLE GetChildByAttr(XMLHANDLE hXml, const char *pszName, const char *pszAttr, const char *pszAttrValue);
	XMLHANDLE GetChildByPath(XMLHANDLE hXml, const char *pszPath); // �������·������ӽڵ�
	bool DeleteAllChildren(XMLHANDLE hXml);
};

#endif // !defined(_XMLDOCUMENT_H_)
