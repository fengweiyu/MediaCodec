/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       VideoTransform.h
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef VIDEO_TRANSFORM_H
#define VIDEO_TRANSFORM_H

#include "VideoDecode.h"
#include "VideoRawHandle.h"
#include "VideoEncode.h"
#include "YUV2RGB.h"
#include <thread>
#include <mutex>
#include <string>
#include <list>
#include <map>

using std::map;
using std::string;
using std::list;
using std::mutex;
using std::thread;



/*****************************************************************************
-Class          : VideoTransform
-Description    : VideoTransform
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2022/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class VideoTransform
{
public:
	VideoTransform();
	virtual ~VideoTransform();
    int SetWaterMarkParam(int i_iEnable,const char * i_strText,const char * i_strFontFile);
    int DecodeToRGBA(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame);
    int DecodeToRGB(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame);
    int Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame);
private:

    AVFrame * m_ptAVFrame;
    AVFrame * m_ptAVFrameRGBA;

	VideoDecode *m_pVideoDecode;
	VideoEncode *m_pVideoEncode;
	VideoRawHandle *m_pVideoRawHandle;
    YUV2RGB * m_pYUV2RGB;
    
    int m_iSetWaterMarkFlag;//0 ·ñ £¬1ÊÇ
	string m_strWaterMarkText;
	string m_strWaterMarkFontFile;
};













#endif
