/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       VideoRawHandle.h
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef VIDEO_RAW_HANDLE_H
#define VIDEO_RAW_HANDLE_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/opt.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}
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
-Class          : VideoRawHandle
-Description    : VideoRawHandle
* Modify Date     VideoRawHandleAuthor           Modification
* -----------------------------------------------
* 2022/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class VideoRawHandle
{
public:
	VideoRawHandle();
	virtual ~VideoRawHandle();
    int Init(AVCodecContext *i_ptDecodeCtx,const char *i_strFiltersDescr);
    int RawHandle(AVFrame *m_ptAVFrame);
    int CreateWaterMarkFilter(int i_iWidth, int i_iHeight,const char * i_strText,const char * i_strFontFile,string *o_pstrFilterDescr);
    int CreateScaleFilter(int i_iDstWidth, int i_iDstHeight,string *o_pstrFilterDescr);
    int Init(AVCodecContext *i_ptDecodeCtx,string ** i_ppFilterDescrs,int i_iFiltersNum);
private:
        
    AVFrame     *m_ptFiltFrame;//存储一帧解码后像素（采样）数据
    AVFilterContext *m_ptBufferSrcCtx;
    AVFilterContext *m_ptBufferSinkCtx;
    AVFilterGraph *m_ptFilterGraph;
};













#endif
