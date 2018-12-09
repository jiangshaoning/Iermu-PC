// 包含头文件
#include <Windows.h>
#include "EpRenderer.h"

#include "GL/glew.h"

#include "vdev.h"

extern "C" {
#include "libavformat/avformat.h"
}

// 预编译开关
#define ENABLE_WAIT_D3D_VSYNC  FALSE

// 内部常量定义
#define DEF_VDEV_BUF_NUM  3

// 内部类型定义
typedef struct
{
    // common members
	VDEV_COMMON_MEMBERS

	HDC			_hDC;
	HGLRC		_hRC;
	EpRenderer  * _renderControl;
	bool		 _renderThreadRun;
	pthread_t	_renderthread;
	unsigned char * *yuvData;
	int			yuvWidth;
	int			yuvHeight;
} VDEVEAPILCTXT;

static void* EapilRenderThreadProc(void *param)
{
    VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)param;
	// create eapil
	unsigned int	PixelFormat;
	static	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		16,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	if (!(c->_hDC = GetDC((HWND)c->surface)))
	{
		MessageBox(NULL, L"不能创建一个窗口设备描述表", L"错误", MB_OK | MB_ICONEXCLAMATION);
		return NULL;
	}

	if (!(PixelFormat = ChoosePixelFormat(c->_hDC, &pfd)))
	{
		MessageBox(NULL, L"不能创建一种相匹配的像素格式", L"错误", MB_OK | MB_ICONEXCLAMATION);
		return NULL;
	}

	if (!SetPixelFormat(c->_hDC, PixelFormat, &pfd))
	{
		MessageBox(NULL, L"不能设置像素格式", L"错误", MB_OK | MB_ICONEXCLAMATION);
		return NULL;
	}

	if (!(c->_hRC = wglCreateContext(c->_hDC)))
	{
		MessageBox(NULL, L"不能创建OpenGL渲染描述表", L"错误", MB_OK | MB_ICONEXCLAMATION);
		return NULL;
	}

	if (!wglMakeCurrent(c->_hDC, c->_hRC))
	{
		MessageBox(NULL, L"不能激活当前的OpenGL渲然描述表", L"错误", MB_OK | MB_ICONEXCLAMATION);
		return NULL;
	}
	//c->_renderControl->SetExpireKey("E8414C234E5C32A99D992398089A53BA93CA02EFAE937EB1B40290E85058381E4F417FCF77196C73F0D14178888604246F4526B6B29D32BCC366A5781B1D6713");
	//EpRenderer::RegisterSDK("e8/owi8BnQ7Pm0XgRDP6H4L8L1q/z2MGsP4AehLiKC+UjqwccLLNVE6xcEO0curgdAA4H2atQ4QQ6JJbudLdjQ==");
	c->_renderControl->InitOpenGL();
	while (c->_renderThreadRun)
	{
		if (c->status & VDEV_CLOSE) break;

		c->_renderControl->Render();
		SwapBuffers(c->_hDC);
		Sleep(5);
	}
	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(c->_hDC);
	c->_renderControl->UnInitOpenGL();

	if (c->_hRC)
	{
		if (!wglMakeCurrent(NULL, NULL))
		{
			MessageBox(NULL, L"释放DC或RC失败。", L"关闭错误", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(c->_hRC))
		{
			MessageBox(NULL, L"释放RC失败。", L"关闭错误", MB_OK | MB_ICONINFORMATION);
		}
		c->_hRC = NULL;
	}
	if (c->_hDC && !ReleaseDC((HWND)c->surface, c->_hDC))
	{
		MessageBox(NULL, L"释放DC失败。", L"关闭错误", MB_OK | MB_ICONINFORMATION);
		c->_hDC = NULL;
	}

	return NULL;
}

static void* VideoRenderThreadProc(void *param)
{
	VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)param;

	while (1) {
		sem_wait(&c->semr);
		if (c->status & VDEV_CLOSE) break;

		if (c->refresh_flag) {
			c->refresh_flag = 0;

			vdev_refresh_background(c);
		}

		int64_t vpts = c->vpts = c->ppts[c->head];
		if (vpts != -1)
		{
			c->_renderControl->TranslateVideoData(c->yuvData[c->tail], c->yuvWidth, c->yuvHeight);
		}

		av_log(NULL, AV_LOG_DEBUG, "vpts: %lld\n", vpts);
		if (++c->head == c->bufnum) c->head = 0;
		sem_post(&c->semw);

		// handle av-sync & frame rate & complete
		vdev_avsync_and_complete(c);
	}
	c->_renderThreadRun = false;
	pthread_join(c->_renderthread, NULL);
	return NULL;
}

static void vdev_eapil_lock(void *ctxt, uint8_t *buffer[8], int linesize[8])
{
    VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)ctxt;   
	sem_wait(&c->semw);
	int w = c->w;
	int h = c->h;

	if (c->yuvWidth != c->w || c->yuvHeight != c->h) 
	{
		c->sw = c->w; c->sh = c->h;
		c->yuvWidth = c->w;
		c->yuvHeight = c->h;

		for (int i = 0; i < c->bufnum; i++)
		{
			if (c->yuvData[i])
			{
				free(c->yuvData[i]);
			}
			c->yuvData[i] = new unsigned char[c->yuvWidth*c->yuvHeight*3/2];
		}
	}

	if (buffer) buffer[0] = c->yuvData[c->tail];
	if (linesize) linesize[0] = 3;
}

static void vdev_eapil_unlock(void *ctxt, int64_t pts)
{
    VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)ctxt;
    
    c->ppts [c->tail] = pts;
    if (++c->tail == c->bufnum) c->tail = 0;

    sem_post(&c->semr);
}

static void vdev_eapil_setrect(void *ctxt, int x, int y, int w, int h)
{
	VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)ctxt;
	c->x = x; c->y = y;
	c->w = w; c->h = h;
	c->_renderControl->SetWindow(w, h);
	c->refresh_flag = 1;
}

void vdev_eapil_destroy(void *ctxt)
{
	int i;
	VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)ctxt;

	// make visual effect & rendering thread safely exit
	c->status = VDEV_CLOSE;
	sem_post(&c->semr);
	pthread_join(c->thread, NULL);

	//++ for video
	
	for (i = 0; i<c->bufnum; i++) {
		if (c->yuvData[i]) {
			free(c->yuvData[i]);
		}
	}
	// close semaphore
	sem_destroy(&c->semr);
	sem_destroy(&c->semw);
	delete c->_renderControl;
	c->_renderControl = NULL;
	if (c->ppts) free(c->ppts);
	c->ppts = NULL;
	if (c) free(c);
	c = NULL;
}



static void vdev_eapil_LButtonDown(void *ctxt, int x, int y)
{
	VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)ctxt;
	c->_renderControl->OnLButtonDown(x, y);
}

static void vdev_eapil_LButtonUp(void *ctxt)
{
	VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)ctxt;
	c->_renderControl->OnLButtonUp();
}

static void vdev_eapil_MouseMove(void *ctxt, int x, int y)
{
	VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)ctxt;
	c->_renderControl->OnMouseMove(x, y);
}

static void vdev_eapil_MouseWheel(void *ctxt, short delta)
{
	VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)ctxt;
	c->_renderControl->OnMouseWheel(delta);
}

static void vdev_eapil_setRenderType(void *ctxt, int type)
{
	VDEVEAPILCTXT *c = (VDEVEAPILCTXT*)ctxt;
	PlayerType ptype = RENDERBALL;
	switch (type)
	{
		case 0:
			ptype = RENDERBALL;
			break;
		case 1:
			ptype = RENDERVR;
			break;
		case 2:
			ptype = RENDERWIDESCREEN;
			break;
		case 3:
			ptype = RENDERBALLFOURSCREEN;
			break;
		default:
			break;
	}
	c->_renderControl->SetPlayerType(ptype);
}

// 接口函数实现
void* vdev_eapil_create(void *surface, int bufnum, int w, int h, int frate, char *temp)
{
    VDEVEAPILCTXT *ctxt = (VDEVEAPILCTXT*)calloc(1, sizeof(VDEVEAPILCTXT));
    if (!ctxt) {
        av_log(NULL, AV_LOG_ERROR, "failed to allocate eapil vdev context !\n");
        exit(0);
    }

    // init vdev context
    bufnum          = bufnum ? bufnum : DEF_VDEV_BUF_NUM;
    ctxt->surface   = surface;
    ctxt->bufnum    = bufnum;
    ctxt->pixfmt 	= AV_PIX_FMT_YUV420P;
	ctxt->w			= w > 1 ? w : 1;
	ctxt->h			= h > 1 ? h : 1;
	ctxt->sw		= w > 1 ? w : 1;
	ctxt->sh		= h > 1 ? h : 1;
    ctxt->tickframe = 1000 / frate;
    ctxt->ticksleep = ctxt->tickframe;
    ctxt->apts      = -1;
    ctxt->vpts      = -1;
    ctxt->lock      = vdev_eapil_lock;
    ctxt->unlock    = vdev_eapil_unlock;
    ctxt->setrect   = vdev_eapil_setrect;
    ctxt->destroy   = vdev_eapil_destroy;
	ctxt->leftbtndown = vdev_eapil_LButtonDown;
	ctxt->leftbtnup = vdev_eapil_LButtonUp;
	ctxt->mousemove = vdev_eapil_MouseMove;
	ctxt->mousewheel = vdev_eapil_MouseWheel;
	ctxt->setrendertype = vdev_eapil_setRenderType;


    // alloc buffer & semaphore
    ctxt->ppts   = (int64_t*)calloc(bufnum, sizeof(int64_t));
	ctxt->yuvData = (BYTE**)calloc(bufnum, sizeof(BYTE*));
   
    // create semaphore
    //ctxt->semr = CreateSemaphore(NULL, 0     , bufnum, NULL);
    //ctxt->semw = CreateSemaphore(NULL, bufnum, bufnum, NULL);
	sem_init(&ctxt->semr, 0, 0);
	sem_init(&ctxt->semw, 0, bufnum);
	if (!ctxt->ppts || !ctxt->yuvData || !ctxt->semr || !ctxt->semw ) {
		av_log(NULL, AV_LOG_ERROR, "failed to allocate resources for vdev-eapil !\n");
		exit(0);
	}
	ctxt->_renderControl = new EpRenderer();
	RECT rect;
	GetWindowRect((HWND)ctxt->surface, &rect);
	int ww = rect.right - rect.left;
	int hh = rect.bottom - rect.top;
	ctxt->_renderControl->SetWindow(ww, hh);
	//test
	//ctxt->_renderControl->LoadTemplate("6c0ca87ff17b22a3c4a1967b41f6c000e79e6deda8e54c2d71b0000eb28e5d0580cc1b86767ee74e441409a2984f8ac9aeb5998e1dc1a5aff3e6e78f9b3179720e2e15d03509dbfad47407982bc49d6a4a7aee9e217d456fe7738f161bd0f580f52df72e69833ee9da8523bfd721fb89cb61922828fe57847fbfb45f3fe40b879ba45260de382ca8c3c0e256b2e73c9b60fdd7201ec06c2a1b3a5b151c98dfeabe263f0d71db7d2848ca845fdb03f3ffa0650e16f34d0089b783aa5184d0633a902d1b8d9f073d0fc3591ce0006c152e110db8ce4a521a5feb39675ceff453495a9ea9168d7e61e6b173ed75594aa0661c18cf40661cd826cc5d9f3635542e0c5e61edb75793eab1ae11b504774e58560304baa43640b0690e045a91c4e53c576c8fe8af3172796534bee503044597b4f804afbf4ed31edd15308bb001dec9e7f3dcb9aec11eb8ac7b18d36900ae2802c9b49efb4a39653e6181c8c020635b162a1b2167c40dfc0d1e94cadf86810a9637df173f7f96d6cb1f6966d8106c2deffd880197f1b18ced564e518438bb9d0a147f1ac3154955b4101b87ad89895a3350c9c7e399f8fed3c3b241c84e29e6eb5e56ba2fe71b7049a74cd594d6445b3d94b7b79e4137e222812a5fa62becc204861a8bd63f719f025d5470a55ca1a1d1db669e8643480c634d7010dad1d0e93c", STRINGEN);
	ctxt->_renderControl->LoadTemplate(temp, STRINGEN);
	ctxt->_renderControl->SetLogoVisable(false);
	EpRenderer::RegisterSDK("e8/owi8BnQ7Pm0XgRDP6H4L8L1q/z2MGsP4AehLiKC+UjqwccLLNVE6xcEO0curgdAA4H2atQ4QQ6JJbudLdjQ==");
    // create video rendering thread
	pthread_create(&ctxt->thread, NULL, VideoRenderThreadProc, ctxt);

	ctxt->_renderThreadRun = true;
   // create video rendering thread
	pthread_create(&ctxt->_renderthread, NULL, EapilRenderThreadProc, ctxt);
	
    return ctxt;
}

