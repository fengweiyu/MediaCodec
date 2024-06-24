/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       AudioEncode.h
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef AUDIO_ENCODE_H
#define AUDIO_ENCODE_H

#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>


/*****************************************************************************
-Class          : AudioEncode
-Description    : AudioEncode
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2023/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class AudioEncode
{
public:
	AudioEncode();
	virtual ~AudioEncode();
    int Encode(AVFrame *i_ptAVFrame,unsigned char * o_pbFrameData,unsigned int i_dwFrameMaxLen,int *o_iFrameRate,E_CodecFrameType *o_iFrameType);
    int Init(E_CodecType i_eCodecType,int i_iFrameRate,int i_iWidth,int i_iHeight);
private:
    int CodecTypeToAvCodecId(E_CodecType eCodecType);
    
    AVPacket * m_ptPacket;//存储一帧压缩编码数据
    AVCodecContext  *m_ptCodecContext;//编码器相关信息上下文，内部包含编码器相关的信息(用于控制编解码)，指向AVFormatContext中的streams成员中的codec成员
};













#endif
