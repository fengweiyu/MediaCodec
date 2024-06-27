/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       AudioRawHandle.h
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef AUDIO_RAW_HANDLE_H
#define AUDIO_RAW_HANDLE_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/channel_layout.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
}



/*****************************************************************************
-Class          : AudioRawHandle
-Description    : AudioRawHandle
* Modify Date     AudioRawHandleAuthor           Modification
* -----------------------------------------------
* 2022/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class AudioRawHandle
{
public:
	AudioRawHandle();
	virtual ~AudioRawHandle();
    int Init(AVCodecContext *i_ptSrcDecodeCtx,AVCodecContext *i_ptDstEncodeCtx,int i_iDstFrameSize);
    int RawHandle(AVFrame *m_ptAVFrame,AVCodecContext *i_ptDstEncodeCtx);
    int GetFrame(AVFrame *m_ptAVFrame,AVCodecContext *i_ptDstEncodeCtx,int i_iUseAllSampleFlag=0);
private:
    int InitConvertedSamples(AVCodecContext *i_ptDstEncodeCtx,int i_iSampleNum);
    int GetFrameFromFifo(AVFrame *m_ptAVFrame,AVCodecContext *i_ptDstEncodeCtx,int i_iFrameSize);
    int InitAudioFrame(AVCodecContext *i_ptDstEncodeCtx,int i_iFrameSize); 

    AVAudioFifo *m_ptAudioFifo;//
    SwrContext * m_ptResmapleCtx;    ///< 重采样器
    unsigned char** m_ppbConvertedSamples; ///< 重采样缓存
    int m_iCurConvertedSamplesSize;
    int m_iDstFrameSize;//转换后的帧大小,由外部设置
    AVFrame * m_ptAudioFrame;
    int m_iAudioFrameBufSamples;//m_ptAudioFrame中分配的缓存总大小
    int64_t m_ddwAudioFramePTS;
    int64_t m_ddwAudioFrameBasePTS;
};













#endif
