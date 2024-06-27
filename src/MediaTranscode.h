/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       MediaTranscode.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef MEDIA_TRANSCODE_H
#define MEDIA_TRANSCODE_H

#include "VideoTransform.h"
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
-Class          : MediaTranscode
-Description    : MediaTranscode
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2022/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class MediaTranscode
{
public:
	MediaTranscode();
	virtual ~MediaTranscode();
    int Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame);
    int GetDstFrame(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame);
private:
        
    VideoTransform *m_pVideoTransform;
};













#endif
