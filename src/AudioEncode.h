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
    
    AVPacket * m_ptPacket;//�洢һ֡ѹ����������
    AVCodecContext  *m_ptCodecContext;//�����������Ϣ�����ģ��ڲ�������������ص���Ϣ(���ڿ��Ʊ����)��ָ��AVFormatContext�е�streams��Ա�е�codec��Ա
};













#endif
