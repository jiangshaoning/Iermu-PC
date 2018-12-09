/* ��׼ͷ�ļ� */
#ifndef __STDEFINE_H__
#define __STDEFINE_H__


#ifdef WIN32
// headers
#include <windows.h>
#include <inttypes.h>

// disable warnings
#pragma warning(disable:4996)

// configs
#define CONFIG_ENABLE_VEFFECT    1
#define CONFIG_ENABLE_SNAPSHOT   1
#define CONFIG_ENABLE_SOUNDTOUCH 1

#define PATH_MAX   MAX_PATH
#define strcasecmp stricmp
#endif


#ifdef ANDROID
// headers
#include <limits.h>
#include <inttypes.h>
#include <unistd.h>
#include <android/log.h>

typedef struct {
    long left;
    long top;
    long right;
    long bottom;
} RECT;

// configs
#define CONFIG_ENABLE_VEFFECT    0
#define CONFIG_ENABLE_SNAPSHOT   1
#define CONFIG_ENABLE_SOUNDTOUCH 1
#define TCHAR  char
#endif


#define DO_USE_VAR(a) do { a = a; } while (0)


#endif


