#ifndef __FANPLAYER_DXVA2HWA_H__
#define __FANPLAYER_DXVA2HWA_H__

#ifdef __cplusplus
extern "C" {
#endif

// ����ͷ�ļ�
#include "libavcodec/avcodec.h"

// ��������
int  dxva2hwa_init(AVCodecContext *ctxt, void *d3ddev);
void dxva2hwa_free(AVCodecContext *ctxt);

#ifdef __cplusplus
}
#endif

#endif


