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
    int Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame);
private:

	VideoDecode *m_pVideoDecode;
	VideoEncode *m_pVideoEncode;
	VideoRawHandle *m_pVideoRawHandle;
};













#endif
