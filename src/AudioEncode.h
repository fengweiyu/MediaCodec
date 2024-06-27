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


#include "MediaTranscodeCom.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
}

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
    int Init(E_CodecType i_eCodecType,int i_iSampleRate,int i_iChannels);
    int Encode(AVFrame *i_ptAVFrame,unsigned char * o_pbFrameData,unsigned int i_dwFrameMaxLen,int64_t *o_ddwPTS);
    int GetCodecContext(AVCodecContext **o_ptCodecContext);
private:
    int CodecTypeToAvCodecId(E_CodecType eCodecType);
    int SelectChannelLayout(const AVCodec *codec, AVChannelLayout *dst);
    int GenerateAdtsHeader(unsigned char * o_pbData,int i_iMaxDataLen,int i_iSampleRate,int i_iChannels,int i_iPayloadLen) ;
    
    
    AVPacket * m_ptPacket;//�洢һ֡ѹ����������
    AVCodecContext  *m_ptCodecContext;//�����������Ϣ�����ģ��ڲ�������������ص���Ϣ(���ڿ��Ʊ����)��ָ��AVFormatContext�е�streams��Ա�е�codec��Ա
};













#endif
