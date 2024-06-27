/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       AudioDecode.h
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef AUDIO_DECODE_H
#define AUDIO_DECODE_H

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
}

/*****************************************************************************
-Class          : AudioDecode
-Description    : AudioDecode
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class AudioDecode
{
public:
	AudioDecode();
	virtual ~AudioDecode();
    int Decode(unsigned char * i_pbFrameData,unsigned int  i_dwFrameLen,int64_t i_ddwPTS,int64_t i_ddwDTS,AVFrame *o_ptAVFrame);
    int Init(E_CodecType i_eCodecType,int i_iSampleRate,int i_iChannels=0);//i_iChannels�°汾û��
    int GetCodecContext(AVCodecContext **o_ptCodecContext);
private:
    int CodecTypeToAvCodecId(E_CodecType eCodecType);
    
    AVCodecParserContext *m_ptParser;//���ڽ��������ݽ�����AVPacket�ṹ����
    AVPacket * m_ptPacket;//�洢һ֡ѹ����������
    AVCodecContext  *m_ptCodecContext;//�����������Ϣ�����ģ��ڲ�������������ص���Ϣ(���ڿ��Ʊ����)��ָ��AVFormatContext�е�streams��Ա�е�codec��Ա
    AVFrame         *m_ptFrame;//�洢һ֡��������أ�����������
};













#endif
