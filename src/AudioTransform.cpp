/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       AudioTransform.cpp
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "AudioTransform.h"
#include "MediaTranscodeCom.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <functional>
#include <time.h>

//#include <sys/time.h>


/*****************************************************************************
-Fuction        : AudioTransform
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
AudioTransform::AudioTransform()
{
    m_pAudioDecode = NULL;
    m_pAudioRawHandle = NULL;
    m_pAudioEncode = NULL;
    m_ptAVFrame = av_frame_alloc();
}
/*****************************************************************************
-Fuction        : ~AudioTransform
-Description    : ~AudioTransform
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
AudioTransform::~AudioTransform()
{
    if(NULL!= m_pAudioDecode)
    {
        delete m_pAudioDecode;
        m_pAudioDecode = NULL;
    }
    if(NULL!= m_pAudioRawHandle)
    {
        delete m_pAudioRawHandle;
        m_pAudioRawHandle = NULL;
    }
    if(NULL!= m_pAudioEncode)
    {
        delete m_pAudioEncode;
        m_pAudioEncode = NULL;
    }
    if(NULL!=m_ptAVFrame)
    {
        av_frame_free(&m_ptAVFrame);
        m_ptAVFrame=NULL;
    }
}

/*****************************************************************************
-Fuction        : Transform
-Description    : //return ResLen,<0 err
-Input          : 
-Output         : 
-Return         : 大于0表示取到帧的大小，=0表示帧数据取完了或者填入的帧数据不够
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioTransform::Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame)
{
    int iRet = -1;
    AVCodecContext *ptSrcCodecContext=NULL;
    AVCodecContext *ptDstCodecContext=NULL;
    int64_t ddwPTS=0;

    if(NULL==m_ptAVFrame)
    {
        CODEC_LOGE("NULL==m_ptAVFrame err \r\n");
        return iRet;
    }

    if(NULL== m_pAudioDecode)
    {
        m_pAudioDecode = new AudioDecode();
        if(NULL==m_pAudioDecode)
        {
            CODEC_LOGE("NULL==m_pAudioDecode err \r\n");
            return iRet;
        }
        iRet=m_pAudioDecode->Init(i_pSrcFrame->eEncType,(int)i_pSrcFrame->dwSampleRate);
    }
    if(NULL== m_pAudioEncode)
    {
        m_pAudioEncode = new AudioEncode();
        if(NULL==m_pAudioEncode)
        {
            CODEC_LOGE("NULL==m_pAudioEncode err \r\n");
            return iRet;
        }
        unsigned int dwChannels=o_pDstFrame->dwChannels==0?i_pSrcFrame->dwChannels : o_pDstFrame->dwChannels;
        iRet=m_pAudioEncode->Init(o_pDstFrame->eEncType,(int)o_pDstFrame->dwSampleRate,(int)dwChannels);
    }
    //CODEC_LOGE("i_pSrcFrame->ddwPTS%d,i_pSrcFrame->ddwDTS%d \r\n",i_pSrcFrame->ddwPTS,i_pSrcFrame->ddwDTS);
    av_frame_unref(m_ptAVFrame);
    iRet=m_pAudioDecode->Decode(i_pSrcFrame->pbFrameBuf,i_pSrcFrame->iFrameBufLen,i_pSrcFrame->ddwPTS,i_pSrcFrame->ddwDTS,m_ptAVFrame);
    if(iRet<0)
    {
        CODEC_LOGE("m_pAudioDecode->Decode err \r\n");
        return iRet;
    }
    //当该标志被设置时，FFmpeg会尝试使用最接近的时间戳来表示每个解码帧的时间戳，即尽可能接近原始媒体中的时间戳
    //m_ptAVFrame->pts = m_ptAVFrame->best_effort_timestamp;//AudioRawHandle内处理
    if(NULL== m_pAudioRawHandle)
    {
        m_pAudioRawHandle = new AudioRawHandle();
        if(NULL==m_pAudioRawHandle)
        {
            CODEC_LOGE("NULL==m_pAudioRawHandle err \r\n");
            return iRet;
        }
        iRet=m_pAudioDecode->GetCodecContext(&ptSrcCodecContext);
        iRet=m_pAudioEncode->GetCodecContext(&ptDstCodecContext);
        iRet=m_pAudioRawHandle->Init(ptSrcCodecContext,ptDstCodecContext,o_pDstFrame->iAudioFrameSize);
    }
    iRet=m_pAudioEncode->GetCodecContext(&ptDstCodecContext);
    iRet=m_pAudioRawHandle->RawHandle(m_ptAVFrame,ptDstCodecContext);
    if(iRet<0)
    {
        CODEC_LOGE("m_pAudioRawHandle->RawHandle err \r\n");
        return iRet;
    }
    do
    {
        iRet=GetDstFrame(o_pDstFrame);
        if(iRet<0)
        {
            CODEC_LOGE("GetDstFrame err \r\n");
            break;
        }
        o_pDstFrame->iFrameBufLen=iRet;
        o_pDstFrame->eFrameType=CODEC_FRAME_TYPE_AUDIO_FRAME;
        break;//暂不支持循环这里取多帧，由外部调用GetDstFrame来取多帧
    }while(iRet>0);

    return iRet;//大于0表示取到帧的大小，=0表示帧数据取完了或者填入的帧数据不够
}


/*****************************************************************************
-Fuction        : GetDstFrame
-Description    : 
-Input          : 
-Output         : 
-Return         : 大于0表示取到帧的大小，=0表示帧数据取完了
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioTransform::GetDstFrame(T_CodecFrame *o_pDstFrame)
{
    int iRet = -1;
    AVCodecContext *ptDstCodecContext=NULL;
    int64_t ddwPTS=0;

    
    if(NULL==m_pAudioEncode ||NULL==m_ptAVFrame ||NULL==o_pDstFrame)
    {
        CODEC_LOGE("GetDstFrame NULL==m_pAudioEncode NULL==m_ptAVFrame||NULL==o_pDstFrame err \r\n");
        return iRet;
    }
    iRet=m_pAudioEncode->GetCodecContext(&ptDstCodecContext);
    iRet=m_pAudioRawHandle->GetFrame(m_ptAVFrame,ptDstCodecContext);
    if(iRet<0)
    {
        CODEC_LOGE("m_pAudioRawHandle->RawHandle err \r\n");
        return iRet;
    }
    if(iRet==0)
    {
        //CODEC_LOGD("m_pAudioRawHandle->GetFrame iRet==0 \r\n");
        return iRet;
    }
    
    iRet=m_pAudioEncode->Encode(m_ptAVFrame,o_pDstFrame->pbFrameBuf,(unsigned int)o_pDstFrame->iFrameBufMaxLen,&ddwPTS);
    if(iRet<=0)
    {
        CODEC_LOGE("m_pAudioEncode->Encode err \r\n");
        av_frame_unref(m_ptAVFrame);
        return -1;
    }
    o_pDstFrame->iFrameBufLen=iRet;
    o_pDstFrame->ddwPTS=ddwPTS;
    o_pDstFrame->ddwDTS=o_pDstFrame->ddwPTS;
    o_pDstFrame->eFrameType=CODEC_FRAME_TYPE_AUDIO_FRAME;
    av_frame_unref(m_ptAVFrame);//暂不支持一次性多帧获取，
    
    return iRet;
}


