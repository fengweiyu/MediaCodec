/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       YUV2RGB.h
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef YUV2RGB_H
#define YUV2RGB_H

extern "C" {
#include "libavcodec/avcodec.h"
}



/*****************************************************************************
-Class          : YUV2RGB
-Description    : YUV2RGB
* Modify Date     VideoRawHandleAuthor           Modification
* -----------------------------------------------
* 2022/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class YUV2RGB
{
public:
	YUV2RGB();
	virtual ~YUV2RGB();
    int YUV420P2RGBA(int i_iSrcWidth, int i_iSrcHeight,unsigned char * i_pbSrcBufYUV,int i_iSrcBufMaxLen,unsigned char * o_pbDstBufRGB,int i_iDstBufMaxLen);
    int YUV420P2RGBA(AVFrame* i_pbSrcAVFrame,unsigned char * o_pbDstBufRGB,int i_iDstBufMaxLen);
private:
        
    unsigned char * m_pbImageY;//存储一帧原始图像的Y分量，最大支持4k
    unsigned char * m_pbImageU;//存储一帧原始图像的U分量，最大支持4k
    unsigned char * m_pbImageV;//存储一帧原始图像的V分量，最大支持4k
};













#endif
