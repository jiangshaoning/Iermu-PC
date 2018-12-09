// ����ͷ�ļ�
#include "vdev.h"

#ifdef WIN32
#include <tchar.h>
#endif

extern "C" {
#include "libavutil/log.h"
#include "libavutil/time.h"
}

// �ڲ���������
#define COMPLETED_COUNTER  30

// ����ʵ��
void* vdev_create(int type, void *surface, int bufnum, int w, int h, int frate, char *temp)
{
    VDEV_COMMON_CTXT *c = NULL;
#ifdef WIN32
    switch (type) {
    case VDEV_RENDER_TYPE_GDI: c = (VDEV_COMMON_CTXT*)vdev_gdi_create(surface, bufnum, w, h, frate); break;
    case VDEV_RENDER_TYPE_D3D: c = (VDEV_COMMON_CTXT*)vdev_d3d_create(surface, bufnum, w, h, frate); break;
    case VDEV_RENDER_TYPE_EAPIL:c =(VDEV_COMMON_CTXT*)vdev_eapil_create(surface, bufnum, w, h, frate, temp); break;
    }
    _tcscpy(c->font_name, DEF_FONT_NAME);
    c->font_size = DEF_FONT_SIZE;
    c->status   |= VDEV_CONFIG_FONT;
	c->rendertype = type;
#endif
#ifdef ANDROID
    c = (VDEV_COMMON_CTXT*)vdev_android_create(surface, bufnum, w, h, frate);
#endif
    return c;
}

void vdev_destroy(void *ctxt)
{
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
    if (c->destroy) c->destroy(c);
}

void vdev_lock(void *ctxt, uint8_t *buffer[8], int linesize[8])
{
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
    if (c->lock) c->lock(c, buffer, linesize);
}

void vdev_unlock(void *ctxt, int64_t pts)
{
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
    if (c->unlock) c->unlock(c, pts);
}

void vdev_setrect(void *ctxt, int x, int y, int w, int h)
{
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
    c->x  = x; c->y  = y;
    c->w  = w; c->h  = h;
    c->status |= VDEV_ERASE_BG0;
    if (c->setrect) c->setrect(c, x, y, w, h);
}

void vdev_pause(void *ctxt, int pause)
{
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
    if (pause) {
        c->status |=  VDEV_PAUSE;
    } else {
        c->status &= ~VDEV_PAUSE;
    }

    // set AV_NOPTS_VALUE to triger re-calculating of
    // start_pts & start_tick in video rendering thread
    c->start_pts = AV_NOPTS_VALUE;
}

void vdev_reset(void *ctxt)
{
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
#if 0 //++ no need to reset vdev buffer queue
    while (0 == sem_trywait(&c->semr)) {
        sem_post(&c->semw);
    }
    c->head   = c->tail =  0;
#endif//-- no need to reset vdev buffer queue
    c->apts   = c->vpts = AV_NOPTS_VALUE;
    c->status = 0;
}

void vdev_getavpts(void *ctxt, int64_t **ppapts, int64_t **ppvpts)
{
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
    if (ppapts) *ppapts = &c->apts;
    if (ppvpts) *ppvpts = &c->vpts;
}

void vdev_leftbtndown(void *ctxt, int x, int y)
{
	VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
	if (c->leftbtndown) c->leftbtndown(c, x, y);
}

void vdev_leftbtnup(void *ctxt)
{
	VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
	if (c->leftbtnup) c->leftbtnup(c);
}

void vdev_mousemove(void *ctxt, int x, int y)
{
	VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
	if (c->mousemove) c->mousemove(c, x, y);
}

void vdev_mousewheel(void *ctxt, short delta)
{
	VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
	if (c->mousewheel) c->mousewheel(c, delta);
}

void vdev_setrendertype(void *ctxt, int type)
{
	VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
	if (c->setrendertype) c->setrendertype(c, type);

}

void vdev_setparam(void *ctxt, int id, void *param)
{
    if (!ctxt || !param) return;
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;

    switch (id) {
    case PARAM_PLAY_SPEED_VALUE:
        c->speed = *(int*)param;
        break;
    case PARAM_AVSYNC_TIME_DIFF:
        c->tickavdiff = *(int*)param;
        break;
	case PARAM_PLAY_LOW_DELAY:
		c->low_delay = *(int*)param;
		break;
    }
    if (c->setparam) c->setparam(c, id, param);
}

void vdev_getparam(void *ctxt, int id, void *param)
{
    if (!ctxt || !param) return;
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;

    switch (id) {
    case PARAM_PLAY_SPEED_VALUE:
        *(int*)param = c->speed;
        break;
    case PARAM_AVSYNC_TIME_DIFF:
        *(int*)param = c->tickavdiff;
        break;
    }
    if (c->getparam) c->getparam(c, id, param);
}

#ifdef WIN32
void vdev_textout(void *ctxt, int x, int y, int color, TCHAR *text)
{
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
    c->textx = x;
    c->texty = y;
    c->textc = color;
    c->textt = text;
}

void vdev_textcfg(void *ctxt, TCHAR *fontname, int fontsize)
{
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
    c->font_size = fontsize;
    _tcscpy_s(c->font_name, _countof(c->font_name), fontname);
    c->status |= VDEV_CONFIG_FONT;
}
#endif

int vdev_refresh_background(void *ctxt)
{
    int ret = 1;
#ifdef WIN32
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
    RECT rtwin, rect1, rect2, rect3, rect4;
    int  x = c->x, y = c->y, w = c->w, h = c->h;
    HWND hwnd = (HWND)c->surface;
    if (c->status & VDEV_ERASE_BG0) {
        c->status &= ~VDEV_ERASE_BG0;
        GetClientRect(hwnd, &rtwin);
        rect1.left = 0;   rect1.top = 0;   rect1.right = rtwin.right; rect1.bottom = y;
        rect2.left = 0;   rect2.top = y;   rect2.right = x;           rect2.bottom = y+h;
        rect3.left = x+w; rect3.top = y;   rect3.right = rtwin.right; rect3.bottom = y+h;
        rect4.left = 0;   rect4.top = y+h; rect4.right = rtwin.right; rect4.bottom = rtwin.bottom;
        InvalidateRect(hwnd, &rect1, TRUE);
        InvalidateRect(hwnd, &rect2, TRUE);
        InvalidateRect(hwnd, &rect3, TRUE);
        InvalidateRect(hwnd, &rect4, TRUE);
    }
    if (c->status & VDEV_ERASE_BG1) {
        rect1.left = x; rect1.top = y; rect1.right = x + w; rect1.bottom = y + h;
        InvalidateRect(hwnd, &rect1, TRUE);
        ret = 0;
    }
#endif
    return ret;
}

void vdev_avsync_and_complete(void *ctxt)
{
    VDEV_COMMON_CTXT *c = (VDEV_COMMON_CTXT*)ctxt;
    int     tickframe, tickdiff, scdiff, avdiff = -1;
    int64_t tickcur, sysclock;

    if (!(c->status & VDEV_PAUSE)) {
        //++ play completed ++//
        if (c->completed_apts != c->apts || c->completed_vpts != c->vpts) {
            c->completed_apts = c->apts;
            c->completed_vpts = c->vpts;
            c->completed_counter = 0;
            c->status &=~VDEV_COMPLETED;
        } else if (++c->completed_counter == COMPLETED_COUNTER) {
            c->status |= VDEV_COMPLETED;
            player_send_message(c->surface, MSG_PLAY_COMPLETED, 0);
        }
        //-- play completed --//

        //++ frame rate & av sync control ++//
        tickframe   = 100 * c->tickframe / c->speed;
        tickcur     = av_gettime_relative() / 1000;
        tickdiff    = (int)(tickcur - c->ticklast);
        c->ticklast = tickcur;

        // re-calculate start_pts & start_tick if needed
        if (c->start_pts < 0) {
            c->start_pts = c->vpts;
            c->start_tick= tickcur;
        }

        sysclock= c->start_pts + (tickcur - c->start_tick) * c->speed / 100;
        scdiff  = (int)(sysclock - c->vpts - c->tickavdiff); // diff between system clock and video pts
        avdiff  = (int)(c->apts  - c->vpts - c->tickavdiff); // diff between audio and video pts
        avdiff  = c->apts <= 0 ? scdiff : avdiff; // if apts is invalid, sync video to system clock
		if (c->low_delay)
			avdiff = 501; //д�� ���ӳ�ֵ
        if (tickdiff - tickframe >  5) c->ticksleep--;
        if (tickdiff - tickframe < -5) c->ticksleep++;
        if (c->vpts >= 0) {
            if      (avdiff >  500) c->ticksleep -= 3;
            else if (avdiff >  50 ) c->ticksleep -= 2;
            else if (avdiff >  30 ) c->ticksleep -= 1;
            else if (avdiff < -500) c->ticksleep += 3;
            else if (avdiff < -50 ) c->ticksleep += 2;
            else if (avdiff < -30 ) c->ticksleep += 1;
        }
        if (c->ticksleep < 0            ) c->ticksleep = 0;
        if (c->ticksleep > tickframe * 3) c->ticksleep = tickframe * 3;
        //-- frame rate & av sync control --//
    } else {
        c->ticksleep = c->tickframe;
    }

    if (c->ticksleep > 0) av_usleep(c->ticksleep * 1000);
    av_log(NULL, AV_LOG_INFO, "d: %3d, s: %3d\n", avdiff, c->ticksleep);
}
