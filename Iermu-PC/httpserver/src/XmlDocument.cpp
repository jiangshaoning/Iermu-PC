// XmlProcess.cpp: implementation of the XMLDocument class.
//
//////////////////////////////////////////////////////////////////////
// XML �ı�������
// 1. Ŀ��:��ȫ������MFC,ʵ��һ��XML�ı�������
// 2. �ŵ�
// (1) ��ȫ������MFC,�������κ�C++����ʹ��
// (2) �������ļ�ϵͳ,�������ڴ��й���XML�ĵ��ṹ��ʹ��
// 2. ����
// (1) ������Ҫ�������ļ������뵽�ڴ���,���Բ��ʺϴ���ܴ��XML�ļ�.һ��10M���ڱȽϺ���
// (2) ����Ľ��ȫ���Ƿ�UNICODE��,ʹ��ʱҪע��.
// (3) ���ڱ��˶�XML��׼������Ϥ,ֻ�Ǹ���ʵ����;�����ʹ�ýӿ�,�����кܶ�XML����û��ʵ��

// ��Ȩ����: �����޸Ļ���������ҵ��;,���뱣��ԭ���ߵ������Ϣ.
// ����: ������ (querw) / Que's C++ Studio
// ����: 2006����

//
//
//

//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <vector>
#include <stack>
#include "XmlDocument.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif
//wchar_t wsz[20] = L"\x4E2D\x6587\x0031\x0032\x0033";

// �ѷ��������ϸ��������Դ���
#ifdef _DEBUG
//#define VERBOSE_TRACE
#endif

using namespace std;
typedef pair<wstring, wstring> wstr2wstr_t;
typedef list<wstr2wstr_t> list_wstr2wstr_t;		// �����б�
typedef list_wstr2wstr_t::iterator iter_wstr2wstr_t;

/*
* ANSI <-> UNICODE ת������
*/

/*
* ϵͳ��ص��ַ���ת������
* Windows ϵͳ WideCharToMultiByte / MultiByteToWideChar
* Linux ϵͳ iconv �⺯��
*
* dest == NULL ����㳤��
* ����������� null
*/
#ifdef _WIN32

unsigned int OS_MapCP(const char *codePage)
{
	if(codePage == NULL) return CP_ACP;
	unsigned int cp = 0;

	if(0 == stricmp(codePage, "iso-8859-1")) cp = 28591;
	if(0 == stricmp(codePage, "gb2312")) cp = 936;
	if(0 == stricmp(codePage, "big5")) cp = 950;
	if(0 == stricmp(codePage, "GB18030")) cp = 54936;
	if(0 == stricmp(codePage, "x-Chinese_CNS")) cp = 20000;
	if(0 == stricmp(codePage, "hz-gb-2312")) cp = 52936;
	if(0 == stricmp(codePage, "utf-8")) cp = CP_UTF8;

	CPINFO cpInf;
	memset(&cpInf, 0, sizeof(CPINFO));
	if(GetCPInfo(cp, &cpInf))
	{
		return cp;
	}
	else
	{
		/* δ��װָ���Ĵ���ҳ */
		return CP_ACP;
	}
}

/*
* �Ѹ��ַ�UNICODE�ַ���ת�����ַ��������෴
* Windows ϵͳ����ν"���ַ���" = Little Endian UTF-16
*/
int OS_AToW(const char *cp, const char *src, size_t srcLen, wchar_t *dest, size_t destLen)
{
	return MultiByteToWideChar(OS_MapCP(cp), 0, src, srcLen, dest, destLen);
}

int OS_WtoA(const char *cp, const wchar_t *src, size_t srcLen, char *dest, size_t destLen)
{
	return WideCharToMultiByte(OS_MapCP(cp), 0, src, srcLen, dest, destLen, NULL, NULL);
}

#else

// δʵ�� iconv
// ..
// ..

#endif

#define STR_ZERO(str, len) memset((str), 0, sizeof(char) * (len))
#define WSTR_ZERO(str, len) memset((str), 0, sizeof(wchar_t) * (len))

string G_W2A(cwchar_t *pwszText)
{
	if(pwszText == NULL) return "";

	// ������Ҫ������ֽ���
	int nNeedSize = OS_WtoA(NULL, pwszText, wcslen(pwszText), NULL, 0);

	// ʵ��ת��
	string strRet("");
	if(nNeedSize > 0)
	{
		char *pRet = new char[nNeedSize + 1];
		STR_ZERO(pRet, nNeedSize + 1);
		OS_WtoA(NULL, pwszText, wcslen(pwszText), pRet, nNeedSize);
		strRet = pRet;
		delete []pRet;
	}
	return strRet;
}

wstring G_A2W(const char* pszText)
{
	if(pszText == NULL) return L"";

	int nNeedSize = OS_AToW(NULL, pszText, strlen(pszText), NULL, 0);

	wstring strRet(L"");
	if(nNeedSize > 0)
	{
		wchar_t *pRet = new wchar_t[nNeedSize + 1];
		WSTR_ZERO(pRet, nNeedSize + 1);
		OS_AToW(NULL, pszText, strlen(pszText), pRet, nNeedSize);
		strRet = pRet;
		delete []pRet;
	}
	return strRet;
}

bool G_IsBlankChar(cwchar_t ch)
{
	return ch == L' ' || ch == L'\n' || ch == L'\r' || ch == L'\t';
}

bool G_IsBlankChar(char ch)
{
	return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
}

bool G_IsValidText(cwchar_t* pszText)
{
	// ���һ���ı�ֻ����' ' \n \r \t����Ϊ����Ч������
	cwchar_t *pTmp = pszText;
	int i = 0;
	while(pTmp[i] != 0)
	{
		if(!G_IsBlankChar(pTmp[i]))
		{
			return true;
		}
		++i;
	}
	return false;
}

bool G_IsValidText(const char *pszText)
{
	// ���һ���ı�ֻ����' ' \n \r \t����Ϊ����Ч������
	const char *pTmp = pszText;
	int i = 0;
	while(pTmp[i] != 0)
	{
		if(!G_IsBlankChar(pTmp[i]))
		{
			return true;
		}
		++i;
	}
	return false;
}

bool G_GetStr(cwchar_t *pBegin, cwchar_t *pEnd, wstring &str) // ��ȡ�ַ���
{
	int nLen = pEnd - pBegin + 1;
	if(nLen > 0)
	{
		wchar_t *pName = new wchar_t[nLen + 1];
		wcsncpy(pName, pBegin, nLen);
		pName[nLen] = 0;
		str = pName;
		delete []pName;
	}

	return nLen > 0;
}

bool G_IsMatch(cwchar_t *pStr1, cwchar_t *pStr2, size_t nStr2Len)
{
	for(size_t i = 0; i < nStr2Len; ++i)
	{
		if(pStr1[i] == 0 || pStr1[i] != pStr2[i])
		{
			return false;
		}
	}

	return true;
}
#define NSTR_EQUAL(str1, str2) G_IsMatch((str1), (str2), (sizeof(str2) / sizeof(wchar_t) - 1))

size_t G_OutputStr(wchar_t *pDest, size_t off, size_t len, cwchar_t *pszSrc)
{
	size_t srcLen = wcslen(pszSrc);
	if(pDest)
	{
		// ��ิ�� len - off ���ȵ��ַ���
		if(srcLen > len - off) srcLen = len - off;
		if(srcLen > 0) wcsncpy(pDest + off, pszSrc, srcLen);
	}
	return srcLen;
}

size_t G_OutputStr(wchar_t *pDest, size_t off, size_t len, cwchar_t ch)
{
	int n = 1;
	if(pDest)
	{
		// ��ิ�� len - off ���ȵ��ַ���
		if(len > off) 
		{
			pDest[off] = ch;
			n = 1;
		}
		else
		{
			n = 0;
		}
	}
	return n;
}
#define OUTPUT_STR(d, off, len, s) G_OutputStr((d), (off), (len), (s))

size_t G_SplitStrings(const std::string &str, std::vector<std::string> &vec, char sp)
{
	std::string srcString(str);
	srcString.push_back(sp); // ����һ���ָ���,����ѭ������.
	std::string::size_type st = 0;
	std::string::size_type stNext = 0;
	while( (stNext = srcString.find(sp, st)) != std::string::npos )
	{
		if(stNext > st)
		{
			vec.push_back(srcString.substr(st, stNext - st));
		}

		// next
		st = stNext + 1;
	}
	return vec.size();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
enum xmlnode_state	// ����״̬
{
	st_begin = 0,		// ��ʼ

	st_tagstart,	// /*tag��ʼ - "<"��ĵ�һ���ַ�*/
	st_tagend,		// tag���� - />,>,?> ǰ�ĵ�һ���ַ�

	st_attrnamestart,	// ��������ʼ - tag��ĵ�һ���ǿո��ַ�
	st_attrnameend,		// ���������� - =, ,ǰ�ĵ�һ���ַ�

	st_attrseparator,	// ������������ֵ�ķָ��� '='

	st_attrvaluestart,	// ����ֵ��ʼ - ',",��ĵ�һ���ַ�
	st_attrvalueend,	// ����ֵ���� - ',",ǰ�ĵ�һ���ַ�

	st_contentstart,	// ���ݿ�ʼ - >��ĵ�һ���ַ�
	st_contentend,		// ���ݽ��� - <ǰ�ĵ�һ���ַ�

	st_endtagstart,		// ����TAG ��ʼ </,<?��ĵ�һ���ַ�
	st_endtagend,		// ����TAG ���� >ǰ�ĵ�һ���ַ�

	st_commentstart,	// ע�Ϳ�ʼ <!--��ĵ�һ���ַ�
	st_commentend,		// ע�ͽ���	-->ǰ�ĵ�һ���ַ�

	st_cdatastart,
	st_cdataend,

	st_end,		// ��������
};

enum xmlnode_output_state
{
	ost_begin, // ��ʼ
	ost_child, // ��������ӽڵ�
	ost_endtag, // �����β��ǩ
	ost_end // ����
};

class XMLNode  
{
private:
	xmlnode_type m_type;		// �ڵ�����
	wstring m_strName;			// ����tag
	wstring m_strText;			// ���� content / comment
	list_wstr2wstr_t m_AttrList;	// �����б�
	bool m_bHasEndTag; // �Ƿ��н������ <name></name> ���ֵ�����ο�,����ڵ㺬���ӽڵ�,���Ǵ��н������.

	XMLNode *m_pParent;			// ���ڵ�ָ��
	XMLNode *m_pPrevSibling;		// ��һ���ֵܽڵ�ָ��
	XMLNode *m_pNextSibling;		// ��һ���ֵܽڵ�ָ��
	XMLNode *m_pFirstChild;		// ��һ���ӽڵ�ָ��
		
	xmlnode_state m_st;			// ����״̬
	xmlnode_output_state m_ost; // ���״̬
	XMLNode *m_pOutputChild; // ��������е��ӽڵ�

private:
	XMLNode(xmlnode_type xNodeType);
	~XMLNode();
	XMLNode(const XMLNode&);
	XMLNode& operator = (const XMLNode&);

	bool LoadNode(cwchar_t* pszContent, cwchar_t* &pszEnd, bool bKeepPlaceHolder);// װ�ؽڵ�, pszContent ������ 0 ��β.
	int GetNode(wchar_t* pBuffer, int nLen); // ���Ϊ���ַ���/���߼��������ֽ���
	void DeleteTree(); // ɾ���Ե�ǰ�ڵ�Ϊ���ڵ������
	bool LinkChild(XMLNode *pNode);		// ����һ���ӽڵ�
	bool Unlink();						// ���Լ��Ӹ��ڵ��жϿ�
#ifdef VERBOSE_TRACE
	static void _trace(const char *psz, const wchar_t *pszText);
	static void _trace(const char *pre, wstring &str);
#else
#define _trace(str1, str2)
#endif
	friend class XMLDocument;
};

XMLNode::XMLNode(const xmlnode_type xNodeType)
	: m_type(xNodeType), m_st(st_begin), m_pOutputChild(NULL), m_ost(ost_begin),
	m_pParent(NULL), m_pPrevSibling(NULL), m_pNextSibling(NULL), 
	m_pFirstChild(NULL), m_bHasEndTag(false)
{
}

XMLNode::~XMLNode()
{
	// ��������ӽڵ�,���п������ڴ�й©
	// ɾ��һ���ڵ�֮ǰ,Ӧ�õ��� DeleteTree() ������ڵ���ӽڵ�ȫ��ɾ��.
	assert(m_pFirstChild == NULL);
}

#ifdef VERBOSE_TRACE
void XMLNode::_trace(const char *psz, const wchar_t *pszText)
{
	TRACE("%s:%s\n", psz, G_W2A(pszText).c_str());
}

void XMLNode::_trace(const char *pre, wstring &str)
{
	_trace(pre, str.c_str());
}
#endif

/*
* ɾ���Ե�ǰ�ڵ�Ϊ���ڵ������(��ǰ�ڵ㲢���ᱻɾ��,�Լ�����ɾ���Լ�)
*/
void XMLNode::DeleteTree()
{
	if(!m_pFirstChild)
	{
	}
	else
	{
		// �������ӽڵ�ѹջ,Ȼ��ȫ��ɾ��
		// �൱����һ��������ȵı���
		std::stack<XMLNode*> NodeStack; // ������������õ�ջ.

		// �Ȱѵ�ǰ�ڵ�������ӽڵ���ջ,��Ϊ��������
		XMLNode *pChildNode = m_pFirstChild;
		while(pChildNode)
		{
			NodeStack.push(pChildNode);
			pChildNode = pChildNode->m_pNextSibling;
		}

		// �������нڵ�ֱ��ջ��Ϊ��
		while(!NodeStack.empty())
		{
			// ����һ���ڵ�
			XMLNode *pCurNode = NodeStack.top();
			NodeStack.pop();

			// �Ѹýڵ�������ӽڵ���ջ.
			pChildNode = pCurNode->m_pFirstChild;
			while(pChildNode)
			{
				NodeStack.push(pChildNode);
				pChildNode = pChildNode->m_pNextSibling;
			}

			// �������,ɾ���ڵ�
			pCurNode->m_pFirstChild = NULL;
			delete pCurNode;
		}

		// ���õ�ǰ�ڵ��״̬(���ӽڵ�)
		m_pFirstChild = NULL;
	}
}

bool XMLNode::LinkChild(XMLNode *pNode)
{
	pNode->m_pParent = this;
	if(m_pFirstChild)
	{
		XMLNode *pChild = m_pFirstChild;
		while(pChild)
		{
			if(pChild->m_pNextSibling == NULL)
			{
				pChild->m_pNextSibling = pNode;
				pNode->m_pPrevSibling = pChild;
				break;
			}
			else
			{
				pChild = pChild->m_pNextSibling;
			}
		}
	}
	else
	{
		m_pFirstChild = pNode;
	}

	return true;
}


bool XMLNode::Unlink()
{
	if(m_pParent)
	{
		if(m_pParent->m_pFirstChild == this)
		{
			m_pParent->m_pFirstChild = m_pNextSibling;
		}
	}

	if(m_pPrevSibling) m_pPrevSibling->m_pNextSibling = m_pNextSibling;
	if(m_pNextSibling) m_pNextSibling->m_pPrevSibling = m_pPrevSibling;

	return true;
}

// pszContent ������ null ��β
// ��������ɹ�, pszEnd ָ����һ�����������ַ����� null.
// �������ʧ��,��pszEndָ����������ַ�.
// ��� bKeepPlaceHolder == true ��ڵ��Ŀհ��ַ�(������ʽ,���е�)��������Ϊ�ӽڵ�.

bool XMLNode::LoadNode(cwchar_t* pszContent, cwchar_t* &pszEnd, bool bKeepPlaceHolder)
{
	/*
	* Ϊ�˱���ݹ����,�������ӽڵ�ʱ,�Ѹ��ڵ�ѹ���ջ,Ȼ��ѭ������.
	*/
	std::stack<XMLNode*> NodeStack; /* �������ڵ�ջ */
	XMLNode *pCurNode = this; /* ��ǰ���ڷ����Ľڵ� */

	const wchar_t* pCur = pszContent; /* ����ָ����һ��Ҫ�������ַ� */
	const wchar_t* pBegin = NULL;
	const wchar_t* pEnd = NULL;

	wstr2wstr_t attrName_attrValue; // ��ʱ��¼ "������=����ֵ"
	wchar_t chValueFlag;	// ' ���� " Ӧ�óɶԳ���
	bool bContinue = true;

	/*
	* pszContent ���� NULL ��β,����ѭ���ڷ��� pCur[0] �� pCur[1] �ǰ�ȫ��
	*/
	while(pCur[0] != 0)
	{
		switch(pCurNode->m_st)
		{
		case st_begin:
			{
				/*
				* ���� '<' ֮ǰ�����пհ��ַ�.
				*/
				if(pCur[0] == L'<')
				{
					_trace("+++++++++++++++++++++", L"��ʼ�����ڵ�");

					// �жϽڵ�����
					if(NSTR_EQUAL(pCur, L"<?"))
					{
						// (1) "<?" ��ͷ����XML�ڵ�
						++pCur; // ���� '?'
						pCurNode->m_type = et_xml;
						pCurNode->m_st = st_tagstart;
					}
					else if(NSTR_EQUAL(pCur, L"<!--"))
					{
						// (2) "<!--" ��ͷ����ע�ͽڵ�
						pCur += 3;
						pCurNode->m_type = et_comment;
						pCurNode->m_st = st_commentstart;
					}
					else if(NSTR_EQUAL(pCur, L"<![CDATA["))
					{
						// (3) "<![CDATA[" ��ͷ "]]>"��β����CDATA����
						pCur += 8;
						pCurNode->m_type = et_cdata;
						pCurNode->m_st = st_cdatastart;
					}
					else
					{
						// (4) �����ڵ�
						pCurNode->m_type = et_normal;
						pCurNode->m_st = st_tagstart;
					}	
				}
				else
				{
					// ��������'<'֮ǰ�Ŀհ��ַ�
					if(G_IsBlankChar(*pCur))
					{
						// �հ��ַ���������
					}
					else
					{
						// �Ƿ��ַ�,ֹͣ����
						bContinue = false;
						break;
					}
				}
			}
			break;
		case st_tagstart:
			{
				// ��¼������Ŀ�ʼλ��
				pBegin = pCur;
				pEnd = NULL;
				pCurNode->m_st = st_tagend;
				--pCur; // ����һ���ַ�
			}
			break;
		case st_tagend:
			{
				// <tag ֱ������ ' ' ���� '>' ���� "/>" ���� "?>" ��ʾ�ڵ�������	
				// "/>" �� "?>" ͳһ����һ��״̬�л�
				if(NSTR_EQUAL(pCur, L"/>") && pCurNode->m_type == et_normal || NSTR_EQUAL(pCur, L"?>") && pCurNode->m_type == et_xml
					|| G_IsBlankChar(pCur[0]) || pCur[0] == L'>')
				{
					pEnd = pCur - 1;
					pCurNode->m_st = st_attrnamestart;
					--pCur;
				}
				else
				{
					// �Ƿ�tag���ַ��ڴ��ж�
					if(pCur[0] == L'<' || pCur[0] == L'/' || pCur[0] == L'?') 
					{
						bContinue = false;
						break;
					}
				}

				// �õ��ڵ����� <tag_name>
				if(pEnd != NULL)
				{
					if(G_GetStr(pBegin, pEnd, pCurNode->m_strName))
					{
						_trace("�ڵ���", pCurNode->m_strName);
						pBegin = NULL;
						pEnd = NULL;
					}
					else
					{
						pCur = pBegin;
						bContinue = false;
						break;
					}
				}
			}
			break;
		case st_attrnamestart:
			{
				// ���Ҳ���¼������������ʼ��ַ
				if(G_IsBlankChar(pCur[0]))
				{
					// ����������ǰ�Ŀհ��ַ�
				}
				else if(L'>' == pCur[0])
				{
					pCurNode->m_st = st_contentstart;
				}
				else if(NSTR_EQUAL(pCur, L"/>") && pCurNode->m_type == et_normal || NSTR_EQUAL(pCur, L"?>") && pCurNode->m_type == et_xml)
				{
					pCurNode->m_st = st_end;
					++pCur;
				}
				else
				{
					// �����ַ���ʶ����������ʼ��ַ
					pBegin = pCur;
					pEnd = NULL;
					pCurNode->m_st = st_attrnameend;
					--pCur;
				}
			}
			break;
		case st_attrnameend:
			{
				// ���Ҳ���ʶ�������Ľ�����ַ:���� '=' ���߿հ��ַ�.
				if(L'=' == pCur[0] || G_IsBlankChar(pCur[0]))
				{
					pCurNode->m_st = st_attrseparator;
					pEnd = pCur - 1;
					--pCur; // ���� '=' ���� st_attrseparator ����
				}
				else
				{
					// ������������Ƿ�����˷Ƿ����ַ�
				}

				if(pEnd)
				{
					attrName_attrValue.first = L"";
					attrName_attrValue.second = L"";
					if(G_GetStr(pBegin, pEnd, attrName_attrValue.first))
					{
						_trace("������", attrName_attrValue.first);
					}
					else
					{
						// �Ƿ���������
						bContinue = false;
						break;
					}
				}
			}
			break;
		case st_attrseparator:
			{
				if(G_IsBlankChar(pCur[0]))
				{
					// ���˵� '=' ǰ�Ķ���Ŀհ��ַ�
				}
				else if(L'=' == pCur[0])
				{
					// ���ҵ��˷ָ���,��ʼ��������ֵ����ʼ��ַ
					pCurNode->m_st = st_attrvaluestart;
				}
				else
				{
					// �������ͷָ����м���˿ո����б���ַ�
					bContinue = false;
					break;
				}
			}
			break;
		case st_attrvaluestart:
			{
				if(G_IsBlankChar(pCur[0]))
				{
					// ���˵� '=' ��, ''' '"' ǰ�Ķ���Ŀհ��ַ�
				}
				else if(L'\'' == pCur[0] || L'\"' == pCur[0])
				{
					// ��¼����ֵ����ʼλ��
					chValueFlag = pCur[0];	// ��¼'/"Ҫ�ɶԳ���
					pBegin = pCur + 1; // ���� ''' ���� '"'
					pEnd = NULL;
					pCurNode->m_st = st_attrvalueend;
				}
				else
				{
					// ������ '=' �ַ����зǷ����ַ�(ֻ�����пո�)
					bContinue = false;
					break;
				}
			}
			break;
		case st_attrvalueend:
			{
				// ��λ����ֵ�Ľ�����ַ:�ɶԳ��ֵ� ''' ���� '"' �ĵڶ���
				if(pCur[0] == chValueFlag)
				{
					assert(chValueFlag == L'\'' || chValueFlag == L'"');
					pEnd = pCur - 1;
					G_GetStr(pBegin, pEnd, attrName_attrValue.second); // ����ֵ�����ǿ�ֵ,���Բ���� G_GetStr() �ķ���ֵ
					pCurNode->m_AttrList.push_back(attrName_attrValue);
					_trace("����ֵ", attrName_attrValue.second);

					// ������һ������, �����е����Ż���˫����,����pCurҪ����һ���ַ�
					if( L' ' == pCur[1] || L'>' == pCur[1] ||
						NSTR_EQUAL(&pCur[1], L"/>") && pCurNode->m_type == et_normal ||
						NSTR_EQUAL(&pCur[1], L"?>") && pCurNode->m_type == et_xml)
					{
						pCurNode->m_st = st_attrnamestart;
					}
					else
					{
						// ����ֵ\"/\'֮���ַǷ��ַ�
						bContinue = false;
						break;
					}
					
				}
				else
				{
					// �Ƿ�������ֵ�ַ��ڴ��ж�
				}
			}
			break;
		case st_contentstart:
			{
				// <name attrname='attrvalue'>content</name>
				// �����ǿո�
				pBegin = pCur;
				pEnd = NULL;
				pCurNode->m_st = st_contentend;
				--pCur;
			}
			break;
		case st_contentend:
			{
				if(L'<' == pCur[0])
				{
					// ��ͨ�ı�Ҳ��Ϊһ���ӽڵ�
					wstring strText;
					pEnd = pCur - 1;
					if(G_GetStr(pBegin, pEnd, strText))
					{
						if(G_IsValidText(strText.c_str()))
						{
							XMLNode *pNode = new XMLNode(et_text);
							pNode->m_strText = strText;
							pCurNode->LinkChild(pNode);
							_trace("��ͨ�ı�", strText);
						}
						else
						{
							if(bKeepPlaceHolder)
							{
								XMLNode *pNode = new XMLNode(et_positionholder);
								pNode->m_strText = strText;
								pCurNode->LinkChild(pNode);
								_trace("�հ�ռλ��", NULL);
							}
						}
					}

					pBegin = NULL;
					pEnd = NULL;

					// ���ݽ�����,�ж���һ������
					if(L'/' == pCur[1] && pCurNode->m_type == et_normal || L'?' ==pCur[1] && pCurNode->m_type == et_xml)
					{						
						pCurNode->m_st = st_endtagstart;
						++pCur; // ���� / ���� ?
					}
					else
					{
						// ��ʼ�����ӽڵ�ǰ�ѵ�ǰ�ڵ�ѹջ.
						XMLNode *pNode = new XMLNode(et_normal);
						pCurNode->LinkChild(pNode);
						NodeStack.push(pCurNode);

						pCurNode = pNode;
						pCurNode->m_st = st_begin;
						--pCur; // �ӽڵ��"<"��ʼ,���Ի���1��

						_trace("--------------------", L"��ʼ�����ӽڵ�");
					}
				}
				else
				{
					// �Ƿ��������ַ��ڴ��ж�
				}
			}
			break;
		case st_cdatastart:
			{
				pBegin = pCur;
				pEnd = NULL;
				pCurNode->m_st = st_cdataend;
				--pCur;
			}
			break;
		case st_cdataend:
			{
				if(NSTR_EQUAL(pCur, L"]]>"))
				{
					pEnd = pCur - 1;
					G_GetStr(pBegin, pEnd, pCurNode->m_strText); // CDATA�ı�Ҳ��Ϊһ���ӽڵ�
					_trace("CDATA����", pCurNode->m_strText);

					// cdata������,�ж���һ������
					pCur += 2;
					pCurNode->m_st = st_end;
				}
				else
				{
					// �Ƿ���CDATA�ַ��ڴ��ж�
				}
			}
			break;
		case st_commentstart:
			{
				pBegin = pCur;
				pCurNode->m_st = st_commentend;
				pEnd = NULL;
				--pCur;
			}
			break;
		case st_commentend:
			{
				if(NSTR_EQUAL(pCur, L"-->"))
				{
					pEnd = pCur - 1;
					G_GetStr(pBegin, pEnd, pCurNode->m_strText);
					_trace("ע������", pCurNode->m_strText);

					// ע�ͽڵ����
					pCur += 2;
					pCurNode->m_st = st_end;
				}
				else
				{
					// �Ƿ���ע���ַ��ڴ��ж�
					// �������� "--" Ϊ�Ƿ�
				}
			}
			break;
		case st_endtagstart:
			{
				// ������ǩ </tagname>
				pBegin = pCur;
				pEnd = NULL;
				pCurNode->m_st = st_endtagend;
				--pCur;
			}
			break;
		case st_endtagend:
			{
				if(L'>' == pCur[0])
				{
					pEnd = pCur - 1;
					wstring strTag;
					G_GetStr(pBegin, pEnd, strTag);
					_trace("������ǩ", strTag);
					if(strTag == pCurNode->m_strName) 
					{
						pCurNode->m_bHasEndTag = true;
						pCurNode->m_st = st_end;
					}
					else 
					{
						bContinue = false;
						break;
					}
				}
			}
			break;
		case st_end:
			{
				_trace("************************", L"һ���ڵ�������");
				// ��ǰ�ڵ�������,��ջ��ȡ�����ڵ��������
				if(!NodeStack.empty())
				{
					pCurNode = NodeStack.top();
					NodeStack.pop();

					pCurNode->m_st = st_contentstart;

					--pCur;
				}
				else
				{
					bContinue = false;
					break;
				}
			}
			break;
		default:
			{
			}
			break;
		}

		/*
		* ����������һ���ַ�
		*/
		if(bContinue)
		{
			++pCur;
		}
		else
		{
			break;
		}
	}

	/*
	* ����
	*/
	pszEnd = pCur;
	if(this->m_st == st_end)
	{
		return true;
	}
	else
	{
		// ��ʹ��ȡʧ��,Ҳ�����Ѿ���һ�����ӽڵ��Ѿ�����,�����ڷ���ǰɾ��֮.
		DeleteTree();
		return false;
	}
}

//�ѽڵ�����ɿ��ַ���
// ����ֵ��ʵ��д����ַ�����(0��ʾ���ʧ��)(������0)
// pBuffer == NULL ���ڼ��㳤�Ȳ����� null
int XMLNode::GetNode(wchar_t* pBuffer, int nLen)
{
	XMLNode *pCurNode = this;
	std::stack<XMLNode*> NodeStack; /* ������ڵ�ջ */
	int nPos = 0;

	// ��ʼ�����״̬
	pCurNode->m_ost = ost_begin;
	pCurNode->m_pOutputChild = NULL;
	
	while(pCurNode)
	{
		switch(pCurNode->m_ost)
		{
		case ost_begin:
			{
				if(pCurNode->m_type == et_text || pCurNode->m_type == et_positionholder)
				{
					// �ı��ڵ�,ֱ�ӷ���
					nPos += OUTPUT_STR(pBuffer, nPos, nLen - nPos, pCurNode->m_strText.c_str());
					pCurNode->m_ost = ost_end;
				}
				else if(pCurNode->m_type == et_comment)
				{	
					// ע�ͽڵ���� <!--ע������-->
					nPos += OUTPUT_STR(pBuffer, nPos, nLen, L"<!--");
					nPos += OUTPUT_STR(pBuffer, nPos, nLen, pCurNode->m_strText.c_str());
					nPos += OUTPUT_STR(pBuffer, nPos, nLen, L"-->");
					pCurNode->m_ost = ost_end;
				}
				else if(pCurNode->m_type == et_cdata)
				{
					// CDATA �ڵ���� <![CDATA[����]]>
					nPos += OUTPUT_STR(pBuffer, nPos, nLen, L"<![CDATA[");
					nPos += OUTPUT_STR(pBuffer, nPos, nLen, pCurNode->m_strText.c_str());
					nPos += OUTPUT_STR(pBuffer, nPos, nLen, L"]]>");
					pCurNode->m_ost = ost_end;
				}
				else
				{
					/*
					* �����ڵ� <name></name>
					*/

					// ��� <
					nPos += OUTPUT_STR(pBuffer, nPos, nLen, L'<');

					// ����XMLЭ��ڵ�,��� ?
					if(pCurNode->m_type == et_xml)
					{
						nPos += OUTPUT_STR(pBuffer, nPos, nLen, L'?');
					}

					// ���tag����
					nPos += OUTPUT_STR(pBuffer, nPos, nLen, pCurNode->m_strName.c_str());

					// �����������ֵ(ͳһʹ��˫����)
					for(iter_wstr2wstr_t iter = pCurNode->m_AttrList.begin(); iter != pCurNode->m_AttrList.end(); ++iter)
					{
						nPos += OUTPUT_STR(pBuffer, nPos, nLen, L' ');
						nPos += OUTPUT_STR(pBuffer, nPos, nLen, iter->first.c_str());
						nPos += OUTPUT_STR(pBuffer, nPos, nLen, L"=\"");
						nPos += OUTPUT_STR(pBuffer, nPos, nLen, iter->second.c_str());
						nPos += OUTPUT_STR(pBuffer, nPos, nLen, L'\"');
					}

					/*
					* ����ӽڵ�
					*/
					if(pCurNode->m_pFirstChild)
					{
						// ��� >
						nPos += OUTPUT_STR(pBuffer, nPos, nLen, L'>');

						// �ѵ�ǰ�ڵ�ѹջ,Ȼ��ʼ�����һ���ӽڵ�
						pCurNode->m_pOutputChild = pCurNode->m_pFirstChild;
						pCurNode->m_ost = ost_child;

						NodeStack.push(pCurNode);
						pCurNode = pCurNode->m_pOutputChild;
						pCurNode->m_ost = ost_begin;
					}
					else
					{
						// �ڵ���н�����ǩ,�������һ�� '>' Ȼ��ת�������ǩ���״̬.
						if(pCurNode->m_bHasEndTag)
						{
							nPos += OUTPUT_STR(pBuffer, nPos, nLen, L'>');
							pCurNode->m_ost = ost_endtag;
						}
						else
						{
							// û���ӽڵ�Ҳ�����������ǩ,ֱ����� /> ���� ?> �����
							wchar_t ch = pCurNode->m_type == et_xml ? L'?' : L'/';
							nPos += OUTPUT_STR(pBuffer, nPos, nLen, ch);
							nPos += OUTPUT_STR(pBuffer, nPos, nLen, L'>');
							pCurNode->m_ost = ost_end;
						}
					}
				}
			}
			break;
		case ost_child:
			{
				// һ���ӽڵ��Ѿ�������,׼�������һ���ӽڵ�.
				pCurNode->m_pOutputChild = pCurNode->m_pOutputChild->m_pNextSibling;

				if(pCurNode->m_pOutputChild)
				{
					// ��ǰ�ڵ�ѹջ
					pCurNode->m_ost = ost_child;
					NodeStack.push(pCurNode);

					// �����һ���ӽڵ�
					pCurNode = pCurNode->m_pOutputChild;
					pCurNode->m_ost = ost_begin;
				}
				else
				{
					// û���ӽڵ���
					pCurNode->m_ost = ost_endtag;
				}
			}
			break;
		case ost_endtag:
			{
				// ���������ǩ </name> ���� <?name>
				wchar_t ch = pCurNode->m_type == et_xml ? L'?' : L'/';
				
				nPos += OUTPUT_STR(pBuffer, nPos, nLen, L'<');
				nPos += OUTPUT_STR(pBuffer, nPos, nLen, ch);
				nPos += OUTPUT_STR(pBuffer, nPos, nLen, pCurNode->m_strName.c_str());
				nPos += OUTPUT_STR(pBuffer, nPos, nLen, L'>');
				
				pCurNode->m_ost = ost_end;
			}
			break;
		case ost_end:
			{
				// ����״̬Ԥ����һ�����
				pCurNode->m_ost = ost_begin;
				pCurNode->m_pOutputChild = NULL;

				// �ָ�����ϵ�
				if(NodeStack.empty())
				{
					pCurNode = NULL;
				}
				else
				{
					pCurNode = NodeStack.top();
					NodeStack.pop();
				}
			}
			break;
		default: break;
		}
	}

	return nPos;
}

////////////////////////////////////////////////////////////////////
/*
* ��֧�� XPath
*/
class XPath
{
private:
	string m_strPath;
	XMLHANDLE m_hCurNode;
	XMLDocument *m_XmlDoc;
	std::list<XMLHANDLE> m_ResultsList;

	void Find();

public:
	XPath(XMLDocument *xmlDoc, XMLHANDLE hCur, const string &strPath);
	~XPath();

	size_t Count();
	XMLHANDLE First();
	XMLHANDLE Next();
	XMLHANDLE Last();
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

XMLDocument::XMLDocument()
	: m_hXmlRoot(NULL), m_hRoot(NULL)
{
}

XMLDocument::~XMLDocument()
{
	Destroy();
}

bool XMLDocument::Destroy()
{
	if(m_hRoot)
	{
		m_hRoot->DeleteTree();
		delete m_hRoot;
		m_hRoot = NULL;
	}

	if(m_hXmlRoot)
	{
		m_hXmlRoot->DeleteTree();
		delete m_hXmlRoot;
		m_hXmlRoot = NULL;
	}
	return true;
}

XMLHANDLE XMLDocument::Build(const char *pRoot, const char* pVersion, const char* pEncode)
{
	if(m_hXmlRoot) return NULL;

	m_hXmlRoot = new XMLNode(et_xml);
	SetName(m_hXmlRoot, "xml");
	SetAttrValue(m_hXmlRoot, "version", pVersion);
	SetAttrValue(m_hXmlRoot, "encoding", pEncode);

	m_hRoot = new XMLNode(et_normal);
	SetName(m_hRoot, pRoot);

	return m_hRoot;
}

bool XMLDocument::Load(const char* pszBuffer, int nLen, bool bKeepBlank)
{
	// �Ѿ�����,��Ҫ�ȵ��� Destroy()
	bool bRet = false;
	if(m_hXmlRoot) return bRet;

	// ����ǿ��ַ���, ����Unicode Load���.
	if(*((wchar_t*)pszBuffer) == (wchar_t)0xFEFF)
	{
		return Load((cwchar_t*)pszBuffer, nLen, bKeepBlank);
	}

	// ����΢���BOM��ʾ(UTF-8��ʶ),ת��ΪUNICODE����� Unicode Load���
	if(nLen > 3 && pszBuffer[0] == (char)0xEF && pszBuffer[1] == (char)0xBB && pszBuffer[2] == (char)0xBF) 
	{
		pszBuffer += 3; 
		nLen -= 3;
		int nWLen = OS_AToW("utf-8", pszBuffer, nLen, NULL, 0);
		if(nWLen <= 0)
		{
			return false;
		}
		wchar_t *pszWBuffer = new wchar_t[nWLen + 1];
		WSTR_ZERO(pszWBuffer, nWLen + 1);
		OS_AToW("utf-8", pszBuffer, nLen, pszWBuffer, nWLen);

		bRet = Load(pszWBuffer, nWLen, bKeepBlank);
		delete []pszWBuffer;
		return bRet;
	}

	// ��λ��һ���ڵ�Ľ���λ��,���ڴ�ʱû���ַ��������Ϣ
	// Ĭ�ϱ����� utf-8
	int nFirstNodeEndPos = 0;
	while(nFirstNodeEndPos < nLen)
	{
		if(pszBuffer[nFirstNodeEndPos++] == '>') break;
	}

	if(nFirstNodeEndPos <= nLen)
	{
		cwchar_t *pwszEnd = NULL;
		
		// �ѵ�һ�� '>' ֮ǰ(����'>')ת��Ϊ���ַ�,�����÷�������
		int nFirstNodeLen = OS_AToW("utf-8", pszBuffer, nFirstNodeEndPos, NULL, 0);
		wchar_t *pFirstNodeBuf = new wchar_t[nFirstNodeLen + 1];
		WSTR_ZERO(pFirstNodeBuf, nFirstNodeLen + 1);
		OS_AToW("utf-8", pszBuffer, nFirstNodeLen, pFirstNodeBuf, nFirstNodeLen);

		XMLNode *pTmpNode = new XMLNode(et_none);
		if(!pTmpNode->LoadNode(pFirstNodeBuf, pwszEnd, bKeepBlank))
		{
			delete pTmpNode;
		}
		else
		{
			// �����µ���ʼλ��
			pszBuffer += nFirstNodeEndPos;
			nLen -= nFirstNodeEndPos;

			// ��һ���ڵ�����ɹ�,�ж��ǲ��� xml Э��ڵ�
			if( GetType(pTmpNode) == et_xml && GetName(pTmpNode) == "xml" )
			{
				// ��һ���ڵ��� xml Э��ڵ�,����������ڵ�
				m_hXmlRoot = pTmpNode;
				
				// ���� xml Э��ڵ��еı�������,����������ڵ�
				if(nLen <= 0)
				{
					// ����ʧ��,����û�� xml Э��ڵ�,���ǲ���û�и��ڵ�
				}
				else
				{
					string strEncode = GetAttrValue(m_hXmlRoot, "encoding");
					int nRootNodeLen = 0;
					wchar_t *pRootNodeBuf = NULL;
					
					nRootNodeLen = OS_AToW(strEncode.c_str(), pszBuffer, nLen, NULL, 0);
					pRootNodeBuf = new wchar_t[nRootNodeLen + 1];
					WSTR_ZERO(pRootNodeBuf, nRootNodeLen + 1);
					OS_AToW(strEncode.c_str(), pszBuffer, nLen, pRootNodeBuf, nRootNodeLen);
					
					pTmpNode = new XMLNode(et_none);
					if(!pTmpNode->LoadNode(pRootNodeBuf, pwszEnd, bKeepBlank))
					{
						delete pTmpNode;
					}
					else
					{
						m_hRoot = pTmpNode;
						if(!G_IsValidText(pwszEnd))
						{
							bRet = true;
						}
						else
						{
							// ����ʧ��,ֻ����һ�����ڵ�,���Ҹ��ڵ�����пհ�֮��������ַ�.
						}
					}

					delete []pRootNodeBuf;
				}
			}
			else
			{
				// ��һ���ڵ㲻�� xml Э��ڵ�,��Ϊ�����Ǹ��ڵ�,û��ָ��Э��ڵ�,�򴴽�һ��Ĭ�ϵ�.
				m_hRoot = pTmpNode;

				if(!G_IsValidText(pwszEnd) && !G_IsValidText(pszBuffer))
				{
					m_hXmlRoot = new XMLNode(et_xml);
					SetName(m_hXmlRoot, "xml");
					SetAttrValue(m_hXmlRoot, "version", "1.0");
					SetAttrValue(m_hXmlRoot, "encoding", "ISO-8859-1"); // ���� gb2312 

					bRet = true;
				}
				else
				{
					// ����ʧ��,ֻ����һ�����ڵ�,���Ҹ��ڵ�����пհ�֮��������ַ�.
				}
			}
		}

		delete []pFirstNodeBuf;
	}
	
	// �������ʧ��,���� xml ��
	if(!bRet)
	{
		Destroy();
	}
	return bRet;
}

/*
* UNICODE �������
* ��������Դ�й��ڱ��������
*/
bool XMLDocument::Load(cwchar_t *pwszBuffer, int nLen, bool bKeepBlank)
{
	bool bRet = false;
	if(m_hXmlRoot) return bRet;

	// ���� UNICODE ���(Windowsϵͳ�µ��ļ�������������)
	if(pwszBuffer[0] == (cwchar_t)0xFEFF)
	{
		++pwszBuffer;
		--nLen;
	}

	// �����һ���ڵ�
	cwchar_t *pwszEnd = NULL;
	XMLNode *pTmpNode = new XMLNode(et_none);
	if(pTmpNode->LoadNode(pwszBuffer, pwszEnd, bKeepBlank))
	{
		// ��һ���ڵ�����ɹ�
		if(GetType(pTmpNode) == et_xml && GetName(pTmpNode) == "xml")
		{
			// ��һ���ڵ��� xml Э��ڵ�,����������ڵ�
			m_hXmlRoot = pTmpNode;

			// �����µ���ʼλ��
			pwszBuffer = pwszEnd;
			pTmpNode = new XMLNode(et_none);
			if(pTmpNode->LoadNode(pwszBuffer, pwszEnd, bKeepBlank))
			{
				m_hRoot = pTmpNode;
				if(!G_IsValidText(pwszEnd))
				{
					bRet = true;
				}
				else
				{
					// ����ʧ��,ֻ����һ�����ڵ�,���Ҹ��ڵ�����пհ�֮��������ַ�.
				}
			}
			else
			{
				delete pTmpNode;
			}
		}
		else
		{
			// ��һ���ڵ㲻�� xml Э��ڵ�,����Ϊ�Ǹ��ڵ�
			m_hRoot = pTmpNode;
			if(!G_IsValidText(pwszEnd))
			{
				// ����һ��Ĭ�ϵ� xml Э��ڵ�
				m_hXmlRoot = new XMLNode(et_xml);
				SetName(m_hXmlRoot, "xml");
				SetAttrValue(m_hXmlRoot, "version", "1.0");
				SetAttrValue(m_hXmlRoot, "encoding", "ISO-8859-1"); // ���� gb2312 
				bRet = true;
			}
			else
			{
				// ����ʧ��,���ڵ���治�����зǿհ��ַ�
			}
		}
	}
	else
	{
		delete pTmpNode;
	}
	
	if(!bRet)
	{
		Destroy();
	}
	return bRet;
}

bool XMLDocument::Load(const char* pszFileName, bool bKeepBlank)
{
	bool bRet = false;
	FILE *pFile = NULL;
	pFile = fopen(pszFileName, "rb");
	if(pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long lLen = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		byte *pBuffer = new byte[lLen + 3];	// ȷ����ʹ��UNICODE�ļ�,�������ַ�����0��β.
		if(pBuffer)
		{
			memset(pBuffer, 0, lLen + 3);
			fread(pBuffer, 1, lLen, pFile);

			bRet = Load((char*)pBuffer, lLen, bKeepBlank);
			delete []pBuffer;
		}

		fclose(pFile);
	}
	return bRet;
}

XMLHANDLE XMLDocument::AppendNode(XMLHANDLE hParent, const char *pBuffer, int nLen, bool bKeepBlank)
{
	// ������Դת��Ϊ UNICODE ���ַ���
	int wLen = OS_AToW(NULL, pBuffer, nLen, NULL, 0);
	wchar_t *pwBuffer = new wchar_t[wLen + 1];
	WSTR_ZERO(pwBuffer, wLen + 1);
	OS_AToW(NULL, pBuffer, nLen, pwBuffer, wLen);

	cwchar_t *pwszEnd = NULL;
	XMLHANDLE hNewNode = new XMLNode(et_none);
	if(hNewNode->LoadNode(pwBuffer, pwszEnd, bKeepBlank))
	{
		// ���ؼ��ڵ�����Ƿ����ַ�
		if(hParent)
		{
			hParent->LinkChild(hNewNode);
		}
		else
		{
			m_hRoot->LinkChild(hNewNode);
		}
	}
	else
	{
		delete hNewNode;
		hNewNode = NULL;
	}
	delete []pwBuffer;
	return hNewNode;
}

// ���XML�ڵ������(������0)
// ����ʵ��д��/��Ҫ���ֽڸ���. 0��ʾʧ��.
int XMLDocument::GetString(XMLHANDLE hXml, char *pBuffer, int nLen)
{
	int nPos = 0;
	if(hXml)
	{
		// ��ȡ�����ڵ������ʱ,���������ANSI��
		int nNeedSize = hXml->GetNode(NULL, 0);

		wchar_t *pwszOut = new wchar_t[nNeedSize + 1];
		WSTR_ZERO(pwszOut, nNeedSize + 1);
		hXml->GetNode(pwszOut, nNeedSize);

		nPos = OS_WtoA(NULL, pwszOut, nNeedSize, pBuffer, nLen);
		delete []pwszOut;
	}
	else if(m_hRoot == NULL)
	{
		// return 0
	}
	else
	{
		assert(m_hRoot && m_hXmlRoot);

		/*
		* XMLNode ֻ��������ַ���, ������ʵ�ʵ��� OS_WtoA ֮ǰ�޷�֪�� n ���ȵĿ��ַ���ת��ΪA�ַ�����ĳ���
		* ֻ�ܽ���ʵ�ʲ���.
		*/

		// ����õ�ʵ����Ҫ�Ŀ��ַ�������
		int wLen = m_hXmlRoot->GetNode(NULL, 0);
		wLen += 2; // ���з�,������ xml Э��ڵ�ĩβ���һ�����з�
		wLen += m_hRoot->GetNode(NULL, 0);

		// ���仺��ȥ,����ʵ�����
		int wPos = 0;
		wchar_t *pwBuf = new wchar_t[wLen + 1];
		WSTR_ZERO(pwBuf, wLen + 1);
		wPos = m_hXmlRoot->GetNode(pwBuf, wLen);
		pwBuf[wPos++] = L'\r';
		pwBuf[wPos++] = L'\n';
		wPos += m_hRoot->GetNode(pwBuf + wPos, wLen - wPos);

		// ת��Ϊָ���ı���
		string strEncode = GetAttrValue(m_hXmlRoot, "encoding");
		nPos = OS_WtoA(strEncode.c_str(), pwBuf, wPos, pBuffer, nLen);


		// ɾ��������
		delete []pwBuf;
	}
	return nPos;
}


bool XMLDocument::Save(const char* pszFileName)
{
	bool bRet = false;
	int nOut = GetString(NULL, NULL, 0);

	char *pOut = new char[nOut];
	if(pOut)
	{
		GetString(NULL, pOut, nOut);

		FILE *pFile = fopen(pszFileName, "wb");
		if(pFile)
		{
			fwrite(pOut, 1, nOut, pFile);
			fclose(pFile);
			bRet = true;
		}

		delete []pOut;
	}
	return bRet;
}

XMLHANDLE XMLDocument::AppendNode(XMLHANDLE hParent, const char* pName, xmlnode_type type /* = et_normal */)
{
	if(!m_hXmlRoot) return NULL;

	XMLNode *pNode = new XMLNode(type);
	if(pName) pNode->m_strName = G_A2W(pName);
	if(hParent == NULL)
	{
		m_hRoot->LinkChild(pNode);
	}
	else
	{
		hParent->LinkChild(pNode);
	}
	return pNode;
}

bool XMLDocument::SetAttrValue(XMLHANDLE hXml, const char *pszAttr, const char *pszValue)
{
	if(hXml == NULL || pszAttr == NULL) return false;
	wstring wstrAttr = G_A2W(pszAttr);
	wstring wstrValue = G_A2W(pszValue);

	for(iter_wstr2wstr_t iter = hXml->m_AttrList.begin(); iter != hXml->m_AttrList.end(); ++iter)
	{
		if(wstrAttr == iter->first)
		{
			iter->second = wstrValue;
			return true;
		}
	}
	
	wstr2wstr_t attr;
	attr.first = wstrAttr;
	attr.second = wstrValue;
	hXml->m_AttrList.push_back(attr);
	return true;
}

string XMLDocument::GetAttrValue(XMLHANDLE hXml, const char *pszAttr)
{
	if(hXml)
	{
		iter_wstr2wstr_t iter;
		wstring wstrAttr = G_A2W(pszAttr);
		for(iter = hXml->m_AttrList.begin(); iter != hXml->m_AttrList.end(); ++iter)
		{
			if(iter->first == wstrAttr) return G_W2A(iter->second.c_str());
		}
	}

	return "";
}

XMLHANDLE XMLDocument::GetRootNode()
{
	return m_hRoot;
}

xmlnode_type XMLDocument::GetType(XMLHANDLE hXml)
{
	return hXml->m_type;
}

bool XMLDocument::GetAttrList(XMLHANDLE hXml, list_str2str* pList)
{
	if(hXml && pList)
	{
		for(iter_wstr2wstr_t iter = hXml->m_AttrList.begin(); iter != hXml->m_AttrList.end(); ++iter)
		{
			pList->push_back(str2str(G_W2A(iter->first.c_str()), G_W2A(iter->second.c_str())));
		}
		return true;
	}
	return false;
}

bool XMLDocument::SetName(XMLHANDLE hXml, const char *pszName)
{
	hXml->m_strName = G_A2W(pszName);
	return true;
}

string XMLDocument::GetName(XMLHANDLE hXml)
{
	if(hXml) return G_W2A(hXml->m_strName.c_str());
	else return "";
}

bool XMLDocument::DeleteNode(XMLHANDLE hXml)
{
	if(!hXml || hXml == m_hRoot || hXml == m_hXmlRoot)
	{
		// ���ڵ㲻�ܱ�ɾ��
		return false;
	}
	else
	{
		hXml->Unlink();
		hXml->DeleteTree();
		delete hXml;
		return true;
	}
}

bool XMLDocument::DeleteAllChildren(XMLHANDLE hXml)
{
	if(!hXml)
	{
		return false;
	}
	else
	{
		hXml->DeleteTree();
		return true;
	}
}

XMLHANDLE XMLDocument::Parent(XMLHANDLE hXml)
{
	if(hXml) return hXml->m_pParent;
	else return NULL;
}

XMLHANDLE XMLDocument::FirstChild(XMLHANDLE hXml)
{
	if(hXml) return hXml->m_pFirstChild;
	else return NULL;
}

XMLHANDLE XMLDocument::NextSibling(XMLHANDLE hXml)
{
	if(hXml) return hXml->m_pNextSibling;
	else return NULL;
}

XMLHANDLE XMLDocument::PrevSibling(XMLHANDLE hXml)
{
	if(hXml) return hXml->m_pPrevSibling;
	else return NULL;
}

XMLHANDLE XMLDocument::GetChildByName(XMLHANDLE hXml, const char *pszName)
{
	XMLHANDLE hChild = FirstChild(hXml);
	while(hChild)
	{
		if(GetName(hChild) == pszName) break;
		hChild = NextSibling(hChild);
	}

	return hChild;
}

XMLHANDLE XMLDocument::GetChildByAttr(XMLHANDLE hXml, const char *pszName, const char *pszAttr, const char *pszAttrValue)
{
	XMLHANDLE hChild = FirstChild(hXml);
	while(hChild)
	{
		if(GetName(hChild) == pszName && GetAttrValue(hChild, pszAttr) == pszAttrValue)
		{
			break;
		}
		hChild = NextSibling(hChild);
	}

	return hChild;
}

string XMLDocument::GetText(XMLHANDLE hXml)
{
	if(hXml) return G_W2A(hXml->m_strText.c_str());
	else return "";
}

bool XMLDocument::SetText(XMLHANDLE hXml, const char *pszText)
{
	if(hXml) hXml->m_strText = G_A2W(pszText);
	return hXml != NULL;
}

XMLHANDLE XMLDocument::GetXmlRoot()
{
	return m_hXmlRoot;
}

//XMLHANDLE XMLDocument::AddContent(XMLHANDLE hXml, const char* pszContent, bool bCdata)
//{
//	XMLHANDLE hRet = NULL;
//	if(hXml)
//	{
//		hRet = AppendNode(hXml, NULL, bCdata ? et_cdata : et_text);
//		SetText(hRet, pszContent);
//	}
//
//	return hRet;
//}

XMLHANDLE XMLDocument::SetContent(XMLHANDLE hXml, const char *pszContent)
{
	if(hXml == NULL) return NULL;

	XMLHANDLE hChild = FirstChild(hXml);
	while(hChild)
	{
		if(et_text == GetType(hChild))
		{
			break;
		}
		hChild = NextSibling(hChild);
	}

	if(!hChild) hChild = AppendNode(hXml, NULL, et_text);
	SetText(hChild, pszContent);
	return hChild;
}

std::string XMLDocument::GetContent(XMLHANDLE hXml, XMLHANDLE* phContent)
{
	if(hXml == NULL) return "";
	
	XMLHANDLE hChild = FirstChild(hXml);
	while(hChild)
	{
		if(et_text == GetType(hChild))
		{
			if(phContent != NULL)
			{
				*phContent = hChild;
			}
			return GetText(hChild);
		}

		hChild = NextSibling(hChild);
	}

	return "";
}

/*
* ��򵥵����ֶ�λ
*/
XMLHANDLE XMLDocument::GetNode(const char *pszPath, bool bCreateNew)
{
	XMLHANDLE hNode = NULL;

	// ����'/'�ָ�����λ�ڵ�
	std::vector<std::string> vecPaths;
	if(0 == G_SplitStrings(pszPath, vecPaths, '/'))
	{
		return NULL;
	}

	// ��λ���ڵ�(·���ĵ�һ���ڵ�ض��Ǹ��ڵ�)
	std::vector<std::string>::iterator iter = vecPaths.begin();
	if(GetName(m_hRoot) != (*iter))
	{
		return NULL;
	}
	
	hNode = m_hRoot;
	for(++iter; iter != vecPaths.end(); ++iter)
	{
		XMLHANDLE hChild = GetChildByName(hNode, (*iter).c_str());
		if(hChild == NULL)
		{
			if(bCreateNew)
			{
				hNode = AppendNode(hNode, (*iter).c_str(), et_normal);
			}
			else
			{
				hNode = NULL;
				break;
			}
		}
		else
		{
			hNode = hChild;
		}
	}

	return hNode;
}
