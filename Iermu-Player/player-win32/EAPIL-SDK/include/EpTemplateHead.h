#ifndef	EPTEMPLATEHEAD_H
#define EPTEMPLATEHEAD_H

#include <string>
#include <limits.h>
#include "EpDeviceType.h"

typedef enum
{
    FIVE_MILLION = 0,       // 5百万分辨率摄像机
    FOUR_MILLION,           // 4百万分辨率摄像机
    TWO_MILLION,            // 2百万分辨率摄像机
	EIGHT_MILLION,			// 8百万分辨率摄像机
	EIGHT_MILLION_XYFW,		// 8百万分辨率摄像机
	FHT_XYFW,				// 50万分辨率
	FIVE_MILLION_LX,		// 500万分辨率
    UN_SUPPORT              // 未知分辨率摄像机
}VIDEO_RES;

typedef struct 
{
    float				v_rot[2];
    float				v_distance;
    float				v_mt[9];
    float				v_perspect0[9];
    float				v_perspect1;
    float				v_scale[2];
    float				v_rad[6];
    float				v_vertical;
    float				v_horizontal;
    float				v_srcWidth;
    float				v_dstWidth;
    float				v_dstHeight;

    int					centerX;
    int					centerY;

    float				resizeW;
    float				resizeH;
}VIParameter;

typedef struct 
{
	EP_DEVICE_TYPE_ENUM ep_device_type;
    VIParameter			viParameter[2];
    float				AreaCull[4];
    float				AreaBlend1[4];
    float				AreaBlend2[4];
    float				LeftImageArea[4];
    float				RightImageArea[4];
    float				LeftSrcImageArea[4];
    float				RightSrcImageArea[4];
    int					VideoWidth;
    int					VideoHeight;
    float				VideoScaleX;
    float				VideoScaleY;
	float				paramExt[64];

	bool				isHorProject;

    unsigned char *		IlluMask;

	int					parAdjustSign;
	int					parAdjustWidth;
	int					parAdjustHeight;
	int					parAdjustStep;
	float	*			parAdjustOffsetData;
}TempleteParameter;

typedef enum
{
	CAMERAEN = 0,               //从相机读取加密模板
	LOCAL,			//从本地读取加密或者非加密文件
	STRINGEN,                   //读取加密字符串
	STRING,			//读取非加密字符串
	REMOTE,			//从远程获取带加密模板（url）
	STRINGENEXT1,	//新格式模板114 QF//丢弃
	STRINGENEXT2,	//新格式模板161
	NONETYPE
}TemplateType;

typedef struct tagEpTemplateCtx
{
    std::string SerialNumber;
	EP_DEVICE_TYPE_ENUM ep_device_type;

    // p
    double dFov;
	int horizentalStitchFlag;
    int dstWidth;
    int dstHeight;

    //o v
    double sFov;
    double a;
    double b;
    double c;
    double d;
    double e;

    //o left
    int positionLeft[4];
    double r0;
    double p0;
    double y0;
    double e0;
    double d0;
    int leftCircleCenterX;
    int leftCircleCenterY;

    //o right
    int positionRight[4];
    double r1;
    double p1;
    double y1;
    double e1;
    double d1;
    int rightCircleCenterX;
    int rightCircleCenterY;

    //s
    int seamPositionLeft;
    int seamPositionRight;
    int seamWidth;
    int srcImgWidth;
    int srcImgHeight;

    tagEpTemplateCtx()
    {
        SerialNumber = "";
		horizentalStitchFlag = 0;
        dFov = -1.0f;
        dstWidth = -1;
        dstHeight = -1;

        //o v
        sFov = 1.0f;
        a = -1.0f;
		b = INT_MIN;
        c = -1.0f;
        d = -1.0f;
        e = -1.0f;

        //o left
		positionLeft[0] = INT_MIN;
        positionLeft[1] = INT_MIN;
        positionLeft[2] = INT_MIN;
        positionLeft[3] = INT_MIN;
        r0 = INT_MIN;
        p0 = INT_MIN;
        y0 = INT_MIN;
        e0 = INT_MIN;
        d0 = INT_MIN;
        leftCircleCenterX = -1;
        leftCircleCenterY = -1;

        //o right
        positionRight[0] = INT_MIN;
        positionRight[1] = INT_MIN;
        positionRight[2] = INT_MIN;
        positionRight[3] = INT_MIN;
        r1 = INT_MIN;
        p1 = INT_MIN;
        y1 = INT_MIN;
        e1 = INT_MIN;
        d1 = INT_MIN;
        rightCircleCenterX = -1;
        rightCircleCenterY = -1;

        //s
        seamPositionLeft = -1;
        seamPositionRight = -1;
        seamWidth = -1;
        srcImgWidth = -1;
        srcImgHeight = -1;
    }
}EpTemplateCtx;

#endif  // EPTEMPLATEHEAD_H
