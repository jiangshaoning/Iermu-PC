//-----------------------------------------------------------------------
// Copyright (C) 2016-2020 EAPIL Co.,Ltd. All rights reserved.
//
// FILE       NAME : EpDeviceType.h
//
// CREATED    BY   : weixing (weixing@eapil.com) at 06/02/2017, 13:37
//
// FUNCTION        :所有设备类型，算法中很多参数与此相关
//-----------------------------------------------------------------------

#ifndef __EAPIL_DEVICE_TYPE_H__
#define __EAPIL_DEVICE_TYPE_H__

#include <math.h>

typedef enum {
	PANOCU_400W = 0,	
	PANOC2_200W,		
	PORTAM_400W,		
	PANO_500W,			
	PANO_200W,			
	PORT_500W,			
	PORT_200W,			
	OTHER_1920_960,		
	PANO_4K,			
	PANO_4KXY,
	QF_200W,			
	PANOC2_200W_EPBIN,	
	PANO_LX_400W,
	SINGLE_LENS_CHECK_200W,		
	OPTICS_MODULE_CHECK_200W,	
	PRISM_INSPECTION_200W,		
	XYFW_4K,
	XYFW_1K,
	LX_500W,					
	PANOC2_200W_EPBIN_PUB,		
	JD_200W,			
    NO_EAPIL_DEVICE
}EP_DEVICE_TYPE_ENUM;

#if 1
/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any diEpRect,
// indiEpRect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef __cplusplus
#  error types.hpp header must be compiled as C++
#endif

#include <climits>
#include <cfloat>
#include <vector>

//! @addtogroup core_basic
//! @{


//////////////////////////////// EpPoint_ ////////////////////////////////

/** @brief Template class for 2D EpPoints specified by its coordinates `x` and `y`.

An instance of the class is interchangeable with C structures, CvEpPoint and CvEpPoint2D32f . There is
also a cast operator to convert EpPoint coordinates to the specified type. The conversion from
floating-EpPoint coordinates to integer coordinates is done by rounding. Commonly, the conversion
uses this operation for each of the coordinates. Besides the class members listed in the
declaration above, the following operations on EpPoints are implemented:
@code
pt1 = pt2 + pt3;
pt1 = pt2 - pt3;
pt1 = pt2 * a;
pt1 = a * pt2;
pt1 = pt2 / a;
pt1 += pt2;
pt1 -= pt2;
pt1 *= a;
pt1 /= a;
double value = norm(pt); // L2 norm
pt1 == pt2;
pt1 != pt2;
@endcode
For your convenience, the following type aliases are defined:
@code
typedef EpPoint_<int> EpPoint2i;
typedef EpPoint2i EpPoint;
typedef EpPoint_<float> EpPoint2f;
typedef EpPoint_<double> EpPoint2d;
@endcode
Example:
@code
EpPoint2f a(0.3f, 0.f), b(0.f, 0.4f);
EpPoint pt = (a + b)*10.f;
cout << pt.x << ", " << pt.y << endl;
@endcode
*/
template<typename _Tp> class EpPoint_
{
public:
    typedef _Tp value_type;

    // various constructors
    EpPoint_();
    EpPoint_(_Tp _x, _Tp _y);
    EpPoint_(const EpPoint_& pt);

    EpPoint_& operator = (const EpPoint_& pt);
    //! conversion to another data type
    template<typename _Tp2> operator EpPoint_<_Tp2>() const;

    //! dot product
    _Tp dot(const EpPoint_& pt) const;
    //! dot product computed in double-precision arithmetics
    double ddot(const EpPoint_& pt) const;
    //! cross-product
    double cross(const EpPoint_& pt) const;

    _Tp x, y; //< the EpPoint coordinates
};

typedef EpPoint_<int> EpPoint2i;
typedef EpPoint_<float> EpPoint2f;
typedef EpPoint_<double> EpPoint2d;
typedef EpPoint2i EpPoint;


//////////////////////////////// EpSize_ ////////////////////////////////

/** @brief Template class for specifying the EpSize of an image or EpRectangle.

The class includes two members called width and height. The structure can be converted to and from
the old OpenCV structures CvEpSize and CvEpSize2D32f . The same set of arithmetic and comparison
operations as for EpPoint_ is available.

OpenCV defines the following EpSize_\<\> aliases:
@code
typedef EpSize_<int> EpSize2i;
typedef EpSize2i EpSize;
typedef EpSize_<float> EpSize2f;
@endcode
*/
template<typename _Tp> class EpSize_
{
public:
    typedef _Tp value_type;

    //! various constructors
    EpSize_();
    EpSize_(_Tp _width, _Tp _height);
    EpSize_(const EpSize_& sz);
    EpSize_(const EpPoint_<_Tp>& pt);

    EpSize_& operator = (const EpSize_& sz);
    //! the area (width*height)
    _Tp area() const;

    //! conversion of another data type.
    template<typename _Tp2> operator EpSize_<_Tp2>() const;

    _Tp width, height; // the width and the height
};

typedef EpSize_<int> EpSize2i;
typedef EpSize_<float> EpSize2f;
typedef EpSize_<double> EpSize2d;
typedef EpSize2i EpSize;


//////////////////////////////// EpRect_ ////////////////////////////////

/** @brief Template class for 2D EpRectangles

described by the following parameters:
-   Coordinates of the top-left corner. This is a default interpretation of EpRect_::x and EpRect_::y
in OpenCV. Though, in your algorithms you may count x and y from the bottom-left corner.
-   EpRectangle width and height.

OpenCV typically assumes that the top and left boundary of the EpRectangle are inclusive, while the
right and bottom boundaries are not. For example, the method EpRect_::contains returns true if

\f[x  \leq pt.x < x+width,
y  \leq pt.y < y+height\f]

Virtually every loop over an image ROI in OpenCV (where ROI is specified by EpRect_\<int\> ) is
implemented as:
@code
for(int y = roi.y; y < roi.y + roi.height; y++)
for(int x = roi.x; x < roi.x + roi.width; x++)
{
// ...
}
@endcode
In addition to the class members, the following operations on EpRectangles are implemented:
-   \f$\texttt{EpRect} = \texttt{EpRect} \pm \texttt{EpPoint}\f$ (shifting a EpRectangle by a certain offset)
-   \f$\texttt{EpRect} = \texttt{EpRect} \pm \texttt{EpSize}\f$ (expanding or shrinking a EpRectangle by a
certain amount)
-   EpRect += EpPoint, EpRect -= EpPoint, EpRect += EpSize, EpRect -= EpSize (augmenting operations)
-   EpRect = EpRect1 & EpRect2 (EpRectangle intersection)
-   EpRect = EpRect1 | EpRect2 (minimum area EpRectangle containing EpRect1 and EpRect2 )
-   EpRect &= EpRect1, EpRect |= EpRect1 (and the corresponding augmenting operations)
-   EpRect == EpRect1, EpRect != EpRect1 (EpRectangle comparison)

This is an example how the partial ordering on EpRectangles can be established (EpRect1 \f$\subseteq\f$
EpRect2):
@code
template<typename _Tp> inline bool
operator <= (const EpRect_<_Tp>& r1, const EpRect_<_Tp>& r2)
{
return (r1 & r2) == r1;
}
@endcode
For your convenience, the EpRect_\<\> alias is available: cv::EpRect
*/
template<typename _Tp> class EpRect_
{
public:
    typedef _Tp value_type;

    //! various constructors
    EpRect_();
    EpRect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height);
    EpRect_(const EpRect_& r);
    EpRect_(const EpPoint_<_Tp>& org, const EpSize_<_Tp>& sz);
    EpRect_(const EpPoint_<_Tp>& pt1, const EpPoint_<_Tp>& pt2);

    EpRect_& operator = (const EpRect_& r);
    //! the top-left corner
    EpPoint_<_Tp> tl() const;
    //! the bottom-right corner
    EpPoint_<_Tp> br() const;

    //! EpSize (width, height) of the EpRectangle
    EpSize_<_Tp> EpSize() const;
    //! area (width*height) of the EpRectangle
    _Tp area() const;

    //! conversion to another data type
    template<typename _Tp2> operator EpRect_<_Tp2>() const;

    //! checks whether the EpRectangle contains the EpPoint
    bool contains(const EpPoint_<_Tp>& pt) const;

    _Tp x, y, width, height; //< the top-left corner, as well as width and height of the EpRectangle
};

typedef EpRect_<int> EpRect2i;
typedef EpRect_<float> EpRect2f;
typedef EpRect_<double> EpRect2d;
typedef EpRect2i EpRect;


//! @} core_basic


/////////////////////////////////////////////////////////////////////////
///////////////////////////// Implementation ////////////////////////////
/////////////////////////////////////////////////////////////////////////


//////////////////////////////// 2D EpPoint ///////////////////////////////

template<typename _Tp> inline
EpPoint_<_Tp>::EpPoint_()
    : x(0), y(0) {}

template<typename _Tp> inline
EpPoint_<_Tp>::EpPoint_(_Tp _x, _Tp _y)
    : x(_x), y(_y) {}

template<typename _Tp> inline
EpPoint_<_Tp>::EpPoint_(const EpPoint_& pt)
    : x(pt.x), y(pt.y) {}

template<typename _Tp> inline
EpPoint_<_Tp>& EpPoint_<_Tp>::operator = (const EpPoint_& pt)
{
    x = pt.x; y = pt.y;
    return *this;
}

template<typename _Tp> template<typename _Tp2> inline
EpPoint_<_Tp>::operator EpPoint_<_Tp2>() const
{
    return EpPoint_<_Tp2>((_Tp2)(x), (_Tp2)(y));
}

template<typename _Tp> inline
_Tp EpPoint_<_Tp>::dot(const EpPoint_& pt) const
{
    return (_Tp)(x*pt.x + y*pt.y);
}

template<typename _Tp> inline
double EpPoint_<_Tp>::ddot(const EpPoint_& pt) const
{
    return (double)x*pt.x + (double)y*pt.y;
}

template<typename _Tp> inline
double EpPoint_<_Tp>::cross(const EpPoint_& pt) const
{
    return (double)x*pt.y - (double)y*pt.x;
}

template<typename _Tp> static inline
EpPoint_<_Tp>& operator += (EpPoint_<_Tp>& a, const EpPoint_<_Tp>& b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

template<typename _Tp> static inline
EpPoint_<_Tp>& operator -= (EpPoint_<_Tp>& a, const EpPoint_<_Tp>& b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

template<typename _Tp> static inline
EpPoint_<_Tp>& operator *= (EpPoint_<_Tp>& a, int b)
{
    a.x = (_Tp)(a.x * b);
    a.y = (_Tp)(a.y * b);
    return a;
}

template<typename _Tp> static inline
EpPoint_<_Tp>& operator *= (EpPoint_<_Tp>& a, float b)
{
    a.x = (_Tp)(a.x * b);
    a.y = (_Tp)(a.y * b);
    return a;
}

template<typename _Tp> static inline
EpPoint_<_Tp>& operator *= (EpPoint_<_Tp>& a, double b)
{
    a.x = (_Tp)(a.x * b);
    a.y = (_Tp)(a.y * b);
    return a;
}

template<typename _Tp> static inline
EpPoint_<_Tp>& operator /= (EpPoint_<_Tp>& a, int b)
{
    a.x = (_Tp)(a.x / b);
    a.y = (_Tp)(a.y / b);
    return a;
}

template<typename _Tp> static inline
EpPoint_<_Tp>& operator /= (EpPoint_<_Tp>& a, float b)
{
    a.x = (_Tp)(a.x / b);
    a.y = (_Tp)(a.y / b);
    return a;
}

template<typename _Tp> static inline
EpPoint_<_Tp>& operator /= (EpPoint_<_Tp>& a, double b)
{
    a.x = (_Tp)(a.x / b);
    a.y = (_Tp)(a.y / b);
    return a;
}

template<typename _Tp> static inline
double norm(const EpPoint_<_Tp>& pt)
{
    return sqrt((double)pt.x*pt.x + (double)pt.y*pt.y);
}

template<typename _Tp> static inline
bool operator == (const EpPoint_<_Tp>& a, const EpPoint_<_Tp>& b)
{
    return a.x == b.x && a.y == b.y;
}

template<typename _Tp> static inline
bool operator != (const EpPoint_<_Tp>& a, const EpPoint_<_Tp>& b)
{
    return a.x != b.x || a.y != b.y;
}

template<typename _Tp> static inline
EpPoint_<_Tp> operator + (const EpPoint_<_Tp>& a, const EpPoint_<_Tp>& b)
{
    return EpPoint_<_Tp>((_Tp)(a.x + b.x), (_Tp)(a.y + b.y));
}

template<typename _Tp> static inline
EpPoint_<_Tp> operator - (const EpPoint_<_Tp>& a, const EpPoint_<_Tp>& b)
{
    return EpPoint_<_Tp>((_Tp)(a.x - b.x), (_Tp)(a.y - b.y));
}

template<typename _Tp> static inline
EpPoint_<_Tp> operator - (const EpPoint_<_Tp>& a)
{
    return EpPoint_<_Tp>((_Tp)(-a.x), (_Tp)(-a.y));
}

template<typename _Tp> static inline
EpPoint_<_Tp> operator * (const EpPoint_<_Tp>& a, int b)
{
    return EpPoint_<_Tp>((_Tp)(a.x*b), (_Tp)(a.y*b));
}

template<typename _Tp> static inline
EpPoint_<_Tp> operator * (int a, const EpPoint_<_Tp>& b)
{
    return EpPoint_<_Tp>((_Tp)(b.x*a), (_Tp)(b.y*a));
}

template<typename _Tp> static inline
EpPoint_<_Tp> operator * (const EpPoint_<_Tp>& a, float b)
{
    return EpPoint_<_Tp>((_Tp)(a.x*b), (_Tp)(a.y*b));
}

template<typename _Tp> static inline
EpPoint_<_Tp> operator * (float a, const EpPoint_<_Tp>& b)
{
    return EpPoint_<_Tp>((_Tp)(b.x*a), (_Tp)(b.y*a));
}

template<typename _Tp> static inline
EpPoint_<_Tp> operator * (const EpPoint_<_Tp>& a, double b)
{
    return EpPoint_<_Tp>((_Tp)(a.x*b), (_Tp)(a.y*b));
}

template<typename _Tp> static inline
EpPoint_<_Tp> operator * (double a, const EpPoint_<_Tp>& b)
{
    return EpPoint_<_Tp>((_Tp)(b.x*a), (_Tp)(b.y*a));
}


////////////////////////////////// EpSize /////////////////////////////////

template<typename _Tp> inline
EpSize_<_Tp>::EpSize_()
    : width(0), height(0) {}

template<typename _Tp> inline
EpSize_<_Tp>::EpSize_(_Tp _width, _Tp _height)
    : width(_width), height(_height) {}

template<typename _Tp> inline
EpSize_<_Tp>::EpSize_(const EpSize_& sz)
    : width(sz.width), height(sz.height) {}

template<typename _Tp> inline
EpSize_<_Tp>::EpSize_(const EpPoint_<_Tp>& pt)
    : width(pt.x), height(pt.y) {}

template<typename _Tp> template<typename _Tp2> inline
EpSize_<_Tp>::operator EpSize_<_Tp2>() const
{
    return EpSize_<_Tp2>((_Tp2)(width), (_Tp2)(height));
}

template<typename _Tp> inline
EpSize_<_Tp>& EpSize_<_Tp>::operator = (const EpSize_<_Tp>& sz)
{
    width = sz.width; height = sz.height;
    return *this;
}

template<typename _Tp> inline
_Tp EpSize_<_Tp>::area() const
{
    return width * height;
}

template<typename _Tp> static inline
EpSize_<_Tp>& operator *= (EpSize_<_Tp>& a, _Tp b)
{
    a.width *= b;
    a.height *= b;
    return a;
}

template<typename _Tp> static inline
EpSize_<_Tp> operator * (const EpSize_<_Tp>& a, _Tp b)
{
    EpSize_<_Tp> tmp(a);
    tmp *= b;
    return tmp;
}

template<typename _Tp> static inline
EpSize_<_Tp>& operator /= (EpSize_<_Tp>& a, _Tp b)
{
    a.width /= b;
    a.height /= b;
    return a;
}

template<typename _Tp> static inline
EpSize_<_Tp> operator / (const EpSize_<_Tp>& a, _Tp b)
{
    EpSize_<_Tp> tmp(a);
    tmp /= b;
    return tmp;
}

template<typename _Tp> static inline
EpSize_<_Tp>& operator += (EpSize_<_Tp>& a, const EpSize_<_Tp>& b)
{
    a.width += b.width;
    a.height += b.height;
    return a;
}

template<typename _Tp> static inline
EpSize_<_Tp> operator + (const EpSize_<_Tp>& a, const EpSize_<_Tp>& b)
{
    EpSize_<_Tp> tmp(a);
    tmp += b;
    return tmp;
}

template<typename _Tp> static inline
EpSize_<_Tp>& operator -= (EpSize_<_Tp>& a, const EpSize_<_Tp>& b)
{
    a.width -= b.width;
    a.height -= b.height;
    return a;
}

template<typename _Tp> static inline
EpSize_<_Tp> operator - (const EpSize_<_Tp>& a, const EpSize_<_Tp>& b)
{
    EpSize_<_Tp> tmp(a);
    tmp -= b;
    return tmp;
}

template<typename _Tp> static inline
bool operator == (const EpSize_<_Tp>& a, const EpSize_<_Tp>& b)
{
    return a.width == b.width && a.height == b.height;
}

template<typename _Tp> static inline
bool operator != (const EpSize_<_Tp>& a, const EpSize_<_Tp>& b)
{
    return !(a == b);
}



////////////////////////////////// EpRect /////////////////////////////////

template<typename _Tp> inline
EpRect_<_Tp>::EpRect_()
    : x(0), y(0), width(0), height(0) {}

template<typename _Tp> inline
EpRect_<_Tp>::EpRect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height)
    : x(_x), y(_y), width(_width), height(_height) {}

template<typename _Tp> inline
EpRect_<_Tp>::EpRect_(const EpRect_<_Tp>& r)
    : x(r.x), y(r.y), width(r.width), height(r.height) {}

template<typename _Tp> inline
EpRect_<_Tp>::EpRect_(const EpPoint_<_Tp>& org, const EpSize_<_Tp>& sz)
    : x(org.x), y(org.y), width(sz.width), height(sz.height) {}

template<typename _Tp> inline
EpRect_<_Tp>::EpRect_(const EpPoint_<_Tp>& pt1, const EpPoint_<_Tp>& pt2)
{
    x = std::min(pt1.x, pt2.x);
    y = std::min(pt1.y, pt2.y);
    width = std::max(pt1.x, pt2.x) - x;
    height = std::max(pt1.y, pt2.y) - y;
}

template<typename _Tp> inline
EpRect_<_Tp>& EpRect_<_Tp>::operator = (const EpRect_<_Tp>& r)
{
    x = r.x;
    y = r.y;
    width = r.width;
    height = r.height;
    return *this;
}

template<typename _Tp> inline
EpPoint_<_Tp> EpRect_<_Tp>::tl() const
{
    return EpPoint_<_Tp>(x, y);
}

template<typename _Tp> inline
EpPoint_<_Tp> EpRect_<_Tp>::br() const
{
    return EpPoint_<_Tp>(x + width, y + height);
}

template<typename _Tp> inline
EpSize_<_Tp> EpRect_<_Tp>::EpSize() const
{
    return EpSize_<_Tp>(width, height);
}

template<typename _Tp> inline
_Tp EpRect_<_Tp>::area() const
{
    return width * height;
}

template<typename _Tp> template<typename _Tp2> inline
EpRect_<_Tp>::operator EpRect_<_Tp2>() const
{
    return EpRect_<_Tp2>((_Tp2)(x), (_Tp2)(y), (_Tp2)(width), (_Tp2)(height));
}

template<typename _Tp> inline
bool EpRect_<_Tp>::contains(const EpPoint_<_Tp>& pt) const
{
    return x <= pt.x && pt.x < x + width && y <= pt.y && pt.y < y + height;
}


template<typename _Tp> static inline
EpRect_<_Tp>& operator += (EpRect_<_Tp>& a, const EpPoint_<_Tp>& b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

template<typename _Tp> static inline
EpRect_<_Tp>& operator -= (EpRect_<_Tp>& a, const EpPoint_<_Tp>& b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

template<typename _Tp> static inline
EpRect_<_Tp>& operator += (EpRect_<_Tp>& a, const EpSize_<_Tp>& b)
{
    a.width += b.width;
    a.height += b.height;
    return a;
}

template<typename _Tp> static inline
EpRect_<_Tp>& operator -= (EpRect_<_Tp>& a, const EpSize_<_Tp>& b)
{
    a.width -= b.width;
    a.height -= b.height;
    return a;
}

template<typename _Tp> static inline
EpRect_<_Tp>& operator &= (EpRect_<_Tp>& a, const EpRect_<_Tp>& b)
{
    _Tp x1 = std::max(a.x, b.x);
    _Tp y1 = std::max(a.y, b.y);
    a.width = std::min(a.x + a.width, b.x + b.width) - x1;
    a.height = std::min(a.y + a.height, b.y + b.height) - y1;
    a.x = x1;
    a.y = y1;
    if (a.width <= 0 || a.height <= 0)
        a = EpRect();
    return a;
}

template<typename _Tp> static inline
EpRect_<_Tp>& operator |= (EpRect_<_Tp>& a, const EpRect_<_Tp>& b)
{
    _Tp x1 = std::min(a.x, b.x);
    _Tp y1 = std::min(a.y, b.y);
    a.width = std::max(a.x + a.width, b.x + b.width) - x1;
    a.height = std::max(a.y + a.height, b.y + b.height) - y1;
    a.x = x1;
    a.y = y1;
    return a;
}

template<typename _Tp> static inline
bool operator == (const EpRect_<_Tp>& a, const EpRect_<_Tp>& b)
{
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

template<typename _Tp> static inline
bool operator != (const EpRect_<_Tp>& a, const EpRect_<_Tp>& b)
{
    return a.x != b.x || a.y != b.y || a.width != b.width || a.height != b.height;
}

template<typename _Tp> static inline
EpRect_<_Tp> operator + (const EpRect_<_Tp>& a, const EpPoint_<_Tp>& b)
{
    return EpRect_<_Tp>(a.x + b.x, a.y + b.y, a.width, a.height);
}

template<typename _Tp> static inline
EpRect_<_Tp> operator - (const EpRect_<_Tp>& a, const EpPoint_<_Tp>& b)
{
    return EpRect_<_Tp>(a.x - b.x, a.y - b.y, a.width, a.height);
}

template<typename _Tp> static inline
EpRect_<_Tp> operator + (const EpRect_<_Tp>& a, const EpSize_<_Tp>& b)
{
    return EpRect_<_Tp>(a.x, a.y, a.width + b.width, a.height + b.height);
}

template<typename _Tp> static inline
EpRect_<_Tp> operator & (const EpRect_<_Tp>& a, const EpRect_<_Tp>& b)
{
    EpRect_<_Tp> c = a;
    return c &= b;
}

template<typename _Tp> static inline
EpRect_<_Tp> operator | (const EpRect_<_Tp>& a, const EpRect_<_Tp>& b)
{
    EpRect_<_Tp> c = a;
    return c |= b;
}
#endif
#endif  // __EAPIL_DEVICE_TYPE_H__
