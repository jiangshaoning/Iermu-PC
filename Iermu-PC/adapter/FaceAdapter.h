#pragma once
#include <helper/SAdapterBase.h>
#include "../MainDlg.h"

typedef struct
{
	string			thumbnail;		//»À¡≥Õº∆¨
	string			imageid;
	string			name;
	string			camera;
	string			time;
}FaceItem;

class CGroupDlg;

class CFaceAdapter : public SAdapterBase
{	
public:
	SArray<FaceItem>  *m_cameraList;

	CFaceAdapter(CMainDlg *dlg);
	~CFaceAdapter();
	CFaceAdapter(SArray<FaceItem> *cList);
	void SetTags(SArray<FaceItem> *cList);
	//void add(FaceItem &item);
	void refresh();
	void CameraAdapterClose();
protected:

	virtual int getCount()
	{
		return m_cameraList->GetCount();
	}

	virtual void getView(int position, SWindow *pItem, pugi::xml_node xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}
		SImageWnd *pImg = pItem->FindChildByName2<SImageWnd>(L"img_file_icon");

		if (pImg)
		{
			IBitmap *pBitmap =NULL;
			string path = FACE_PATH;
			path.append("\\").append(m_cameraList->GetAt(position).imageid).append(".jpg");
			SStringA imgPath = path.c_str();
			SStringT imgPahtT = S_CA2T(imgPath);

			if (IsFileExist(imgPahtT))
				pBitmap = SResLoadFromFile::LoadImage(imgPahtT);

			if (pBitmap)
			{
				pImg->SetImage(pBitmap, kHigh_FilterLevel);
				pBitmap->Release();
			}
		}

		SWindow* camera_description = pItem->FindChildByName(L"camera_description");
		if (camera_description)
		{
			//SLOGFMTE("#####################  %s  ##################\n", m_CInfo[position].description.c_str());
			SStringA description = S_CA2A(m_cameraList->GetAt(position).name.c_str());
			camera_description->SetWindowText(S_CA2T(description));
		}
	}

	BOOL IsFileExist(const SStringT& csFile);
private:
	int			m_Count = 0;
	CMainDlg    *m_dlg;
	IBitmap		*m_Bitmap_skin;
	IBitmap		*m_Bitmap_off;
};