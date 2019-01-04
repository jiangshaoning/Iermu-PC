
#include "stdafx.h"
#include <future>   
#include "FaceAdapter.h"
#include "json.h"
#include "CppSQLite3.h"

CFaceAdapter::CFaceAdapter(CMainDlg *dlg):m_dlg(dlg)
{

	//string path_skin = FACE_PATH;
	//path_skin.append("\\iermu_thumb.png");
	//string path_off = FACE_PATH;
	//path_off.append("\\listCameraOff.png");


	//SStringA imgPath = path_skin.c_str();
	//m_Bitmap_skin = SResLoadFromFile::LoadImage(S_CA2T(imgPath));

	//imgPath = path_off.c_str();
	//m_Bitmap_off = SResLoadFromFile::LoadImage(S_CA2T(imgPath));
}

CFaceAdapter::~CFaceAdapter()
{
}

void CFaceAdapter::CameraAdapterClose()
{
	if (m_Bitmap_skin)
	{
		m_Bitmap_skin->Release();
		m_Bitmap_skin = NULL;
	}
		
	if (m_Bitmap_off)
	{
		m_Bitmap_off->Release();
		m_Bitmap_off = NULL;
	}
		
}


CFaceAdapter::CFaceAdapter(SArray<FaceItem> *cList)
{
	SetTags(cList);
}

//void CFaceAdapter::add(FaceItem &item)
//{
//	m_cameraList.Add(item);
//	notifyDataSetChanged();
//}

void CFaceAdapter::refresh()
{
	notifyDataSetChanged();
}

BOOL CFaceAdapter::IsFileExist(const SStringT& csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

void CFaceAdapter::SetTags(SArray<FaceItem> *cList)
{
	m_cameraList = cList;
	//m_cameraList.RemoveAll();
	//for (int i = 0; i<tags.GetCount(); i++)
	//{
	//	FaceItem obj = tags[i];
	//	m_cameraList.Add(obj);
	//}
	notifyDataSetChanged();
}

