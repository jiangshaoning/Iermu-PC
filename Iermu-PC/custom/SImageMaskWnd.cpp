#include "stdafx.h"
#include "SImageMaskWnd.h"
#include "helper\SDIBHelper.h"

namespace SOUI
{
    SImageMaskWnd::SImageMaskWnd(void):m_bGray(false)
    {
		m_pSkin = NULL;
		m_bShowState = true;
    }

    SImageMaskWnd::~SImageMaskWnd(void)
    {
    }

	void SImageMaskWnd::SetGray(bool bGray)
	{
		m_bGray = bGray;
		if (m_bmpCacheGray==NULL)
		{
			if (m_bmpCache)
			{
				m_bmpCache->Clone(&m_bmpCacheGray);
				SDIBHelper::GrayImage(m_bmpCacheGray);
			}
		}
	}
    
    void SImageMaskWnd::OnPaint(IRenderTarget *pRT)
	{
		CRect rcClient = GetClientRect();
		if (m_bGray&&m_bmpCacheGray)
		{
			CRect rcCache(CPoint(0, 0), m_bmpCacheGray->Size());
			pRT->DrawBitmapEx(&rcClient, m_bmpCacheGray, &rcCache, EM_STRETCH);
			return;
		}
        if(m_bmpCache)
        {
            CRect rcCache(CPoint(0,0),m_bmpCache->Size());
            pRT->DrawBitmapEx(&rcClient,m_bmpCache,&rcCache,EM_STRETCH);
        }

		CRect rcIcon(rcClient.right - 10, rcClient.bottom - 10, rcClient.right, rcClient.bottom);
		if (m_pSkin && m_bShowState)
		{
			m_pSkin->Draw(pRT, rcIcon, 0);
		}
    }

    
    HRESULT SImageMaskWnd::OnAttrMask(const SStringW & strValue,BOOL bLoading)
    {
        SStringW strChannel = strValue.Right(2);
        m_iMaskChannel = -1;
        if(strChannel == L".a")
            m_iMaskChannel = 3;
        else if(strChannel == L".b")
            m_iMaskChannel =0;
        else if(strChannel == L".g")
            m_iMaskChannel = 1;
        else if(strChannel == L".r")
            m_iMaskChannel = 2;

        IBitmap *pImg = NULL;
        if(m_iMaskChannel==-1)
        {//use alpha channel as default
            m_iMaskChannel = 0;
            pImg = LOADIMAGE2(strValue);
        }else
        {
            pImg = LOADIMAGE2(strValue.Left(strValue.GetLength()-2));
        }
        if(!pImg)
        {
            return E_FAIL;
        }
        m_bmpMask = pImg;
        pImg->Release();
        
        m_bmpCache = NULL;
        GETRENDERFACTORY->CreateBitmap(&m_bmpCache);
        m_bmpCache->Init(m_bmpMask->Width(),m_bmpMask->Height());
        
        if(!m_strSkin.IsEmpty())
        {
            ISkinObj * pSkin = GETSKIN(m_strSkin, GetScale());
            if(pSkin) MakeCacheAlpha(pSkin);
        }
        return S_OK;
    }

    void SImageMaskWnd::MakeCacheAlpha(ISkinObj *pSkin)
    {
        SASSERT(m_bmpMask && m_bmpCache);
        CAutoRefPtr<IRenderTarget> pRTDst;
        GETRENDERFACTORY->CreateRenderTarget(&pRTDst,0,0);
        CAutoRefPtr<IRenderObj> pOldBmp;
        pRTDst->SelectObject(m_bmpCache,&pOldBmp);
        CRect rc(CPoint(0,0),m_bmpCache->Size());
        pSkin->Draw(pRTDst,&rc,0);
		
        pRTDst->SelectObject(pOldBmp);
        
        //从mask的指定channel中获得alpha通道
        LPBYTE pBitCache = (LPBYTE)m_bmpCache->LockPixelBits();
        LPBYTE pBitMask = (LPBYTE)m_bmpMask->LockPixelBits();
        LPBYTE pDst = pBitCache;
        LPBYTE pSrc = pBitMask + m_iMaskChannel;
        int nPixels = m_bmpCache->Width()*m_bmpCache->Height();
        for(int i=0;i<nPixels;i++)
        {
            BYTE byAlpha = *pSrc;
            pSrc += 4;
            //源半透明，mask不透明时使用源的半透明属性
            if(pDst[3] == 0xff || (pDst[3]!=0xFF &&byAlpha == 0))
            {//源不透明,或者mask全透明
                *pDst++ = ((*pDst) * byAlpha)>>8;//做premultiply
                *pDst++ = ((*pDst) * byAlpha)>>8;//做premultiply
                *pDst++ = ((*pDst) * byAlpha)>>8;//做premultiply
                *pDst++ = byAlpha;
            }
        }
        m_bmpCache->UnlockPixelBits(pBitCache);
        m_bmpMask->UnlockPixelBits(pBitMask);
    }
	void SImageMaskWnd::MakeCacheAlpha(IBitmap *pSkin)
	{
		SASSERT(m_bmpMask && m_bmpCache);
		CAutoRefPtr<IRenderTarget> pRTDst;
		GETRENDERFACTORY->CreateRenderTarget(&pRTDst, 0, 0);		
		pRTDst->SelectObject(m_bmpCache);		
		CRect rc(CPoint(0, 0), m_bmpCache->Size());
		pRTDst->ClearRect(rc, RGBA(0,0,0,0));
		CRect rcSrc(0, 0, pSkin->Width(), pSkin->Height());
		//pSkin->Draw(pRTDst, &rc, 0);
		//pRTDst->DrawBitmap(rc, pSkin, 0, 0);
		pRTDst->DrawBitmapEx(rc, pSkin, rcSrc, 1);


		//从mask的指定channel中获得alpha通道
		LPBYTE pBitCache = (LPBYTE)m_bmpCache->LockPixelBits();
		LPBYTE pBitMask = (LPBYTE)m_bmpMask->LockPixelBits();
		LPBYTE pDst = pBitCache;
		LPBYTE pSrc = pBitMask + m_iMaskChannel;
		int nPixels = m_bmpCache->Width()*m_bmpCache->Height();
		for (int i = 0;i < nPixels; i++)
		{
			BYTE byAlpha = *pSrc;
			pSrc += 4;
			//源半透明，mask不透明时使用源的半透明属性
			if(pDst[3] == 0xff || (pDst[3]!=0xFF &&byAlpha == 0))
			{//源不透明,或者mask全透明
				*pDst++ = ((*pDst) * byAlpha)>>8;//做premultiply
				*pDst++ = ((*pDst) * byAlpha)>>8;//做premultiply
				*pDst++ = ((*pDst) * byAlpha)>>8;//做premultiply
				*pDst++ = byAlpha;
			}
			//end
		}
		m_bmpMask->UnlockPixelBits(pBitMask);
		m_bmpCache->UnlockPixelBits(pBitCache);
		m_bmpCacheGray = NULL;
		m_bmpCache->Clone(&m_bmpCacheGray);
		SDIBHelper::GrayImage(m_bmpCacheGray);
	}

	//从文件加载图片
	void SImageMaskWnd::SetSkinFormFile(const SStringW & strValue)
	{
		IBitmap * pSkin = LOADIMAGE2(L"file:" +strValue);
		if (pSkin) MakeCacheAlpha(pSkin);

		Invalidate();
	}

    HRESULT SImageMaskWnd::OnAttrImage(const SStringW & strValue,BOOL bLoading)
    {
        if(m_bmpCache)
        {
            ISkinObj * pSkin = GETSKIN(strValue, GetScale());
            if(pSkin) MakeCacheAlpha(pSkin);
        }else
        {
            m_strSkin = strValue;
        }
        return bLoading?S_OK:S_FALSE;
    }

}