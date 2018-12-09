#pragma once

namespace SOUI
{
    class SImageMaskWnd : public SImageWnd
    {
    SOUI_CLASS_NAME(SImageMaskWnd,L"imageMask")
    public:
        SImageMaskWnd(void);
        ~SImageMaskWnd(void);
		void SetGray(bool bGray);
		void SetSkinFormFile(const SStringW &);
    protected:
        void OnPaint(IRenderTarget *pRT);
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
        SOUI_MSG_MAP_END()
        
        HRESULT OnAttrMask(const SStringW & strValue,BOOL bLoading);
        HRESULT OnAttrImage(const SStringW & strValue,BOOL bLoading);
        
        void MakeCacheAlpha(ISkinObj *pSkin);
		void MakeCacheAlpha(IBitmap *pSkin);
		

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"mask", OnAttrMask)//image.a
            ATTR_CUSTOM(L"skin", OnAttrImage)
			ATTR_SKIN(L"state_skin", m_pSkin, TRUE)
			ATTR_BOOL(L"is_show_state", m_bShowState, TRUE)
        SOUI_ATTRS_END()
        
		
		ISkinObj				*m_pSkin; 
        SStringW                m_strSkin;
        CAutoRefPtr<IBitmap>    m_bmpCache;
		CAutoRefPtr<IBitmap>    m_bmpCacheGray;
        CAutoRefPtr<IBitmap>    m_bmpMask;
        int                     m_iMaskChannel;
		bool					m_bGray;
		bool					m_bShowState;
    };
}
