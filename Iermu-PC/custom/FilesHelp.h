#pragma once
#include "../stdafx.h"
#include <vector>
using namespace std;

class CFileHelp
{
public:
	CFileHelp(void);
	~CFileHelp(void);
public:
	
	
	//�򿪶Ի��� lpstrFilter�������ַ���   hwndOwner��������  fileNames�������ļ�·��
	static BOOL OpenFile(LPCWSTR lpstrFilter, HWND hwndOwner, vector<SStringT> &fileNames, bool IsMulti = true);
	//��һ������·����ֳɣ��ļ�������չ��
	static void SplitPathFileName(SStringT fileName, SStringT &szName, SStringT &szExt);
	// ����ļ��� path��·��  hwndOwner : ������  tile : ���ڱ���
	static BOOL BrowseDir(SStringT &path, HWND hwndOwner, SStringT title);
	// ����ļ���׺�� pstrPath���ļ�·�� pstrExtFilter�������б�
	static bool FindFileExt(LPCTSTR pstrPath, LPCTSTR pstrExtFilter);
	//�ݹ������ǰĿ¼���ļ��ļ�
	static void EnumerateFiles(vector<SStringT> &vctString, LPCTSTR p_strExtFilter);
	//�õ��ļ��Ĵ�С
	static DWORD GetFileSize(LPCTSTR fileName);

	//�ļ���Сת��Ϊ�ַ����� xx.xxM
	static LPCTSTR FileSizeToString(DWORD dwSize);

	//�ļ���ʱ��04:00
	static SStringT TimeToToleString(int time);
	//����ļ��Ƿ����
	static BOOL CheckFileExist(SStringT pathFileName);
};

class CTestDropTarget :public IDropTarget
{
public:
	CTestDropTarget()
	{
		nRef = 0;
	}

	virtual ~CTestDropTarget() {}

	//////////////////////////////////////////////////////////////////////////
	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		HRESULT hr = S_FALSE;
		if (riid == __uuidof(IUnknown))
			*ppvObject = (IUnknown*) this, hr = S_OK;
		else if (riid == __uuidof(IDropTarget))
			*ppvObject = (IDropTarget*)this, hr = S_OK;
		if (SUCCEEDED(hr)) AddRef();
		return hr;

	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return ++nRef; }

	virtual ULONG STDMETHODCALLTYPE Release(void) {
		ULONG uRet = --nRef;
		if (uRet == 0) delete this;
		return uRet;
	}

	//////////////////////////////////////////////////////////////////////////
	// IDropTarget

	virtual HRESULT STDMETHODCALLTYPE DragEnter(
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void)
	{
		return S_OK;
	}


protected:
	int nRef;
};


