/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       VideoDecode.h
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef VIDEO_DECODE_H
#define VIDEO_DECODE_H

#include "MediaTranscodeCom.h"
extern "C" {
#include "libavcodec/avcodec.h"
}

/*****************************************************************************
-Class          : VideoDecode
-Description    : VideoDecode
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class VideoDecode
{
public:
	VideoDecode();
	virtual ~VideoDecode();
    int Decode(unsigned char * i_pbFrameData,unsigned int  i_dwFrameLen,int64_t i_ddwPTS,int64_t i_ddwDTS,AVFrame *o_ptAVFrame);
    int Init(E_CodecType i_eCodecType);
    int GetCodecContext(AVCodecContext **o_ptCodecContext);
private:
    int CodecTypeToAvCodecId(E_CodecType eCodecType);
    
    AVCodecParserContext *m_ptParser;//用于将码流数据解析到AVPacket结构体中
    AVPacket * m_ptPacket;//存储一帧压缩编码数据
    AVCodecContext  *m_ptCodecContext;//编码器相关信息上下文，内部包含编码器相关的信息(用于控制编解码)，指向AVFormatContext中的streams成员中的codec成员
    AVFrame         *m_ptFrame;//存储一帧解码后像素（采样）数据
};













#endif
