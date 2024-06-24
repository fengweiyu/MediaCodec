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
    
}

/*****************************************************************************
-Fuction        : Transform
-Description    : //return ResLen,<0 err
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioTransform::Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame)
{
    int iRet = -1;
    AVFrame * ptAVFrame=NULL;
    char strTemp[512] = { 0 };

    if(NULL== m_pAudioDecode)
    {
        m_pAudioDecode = new AudioDecode();
        if(NULL==m_pAudioDecode)
        {
            CODEC_LOGE("NULL==m_pAudioDecode err \r\n");
            return iRet;
        }
        iRet=m_pAudioDecode->Init(i_pSrcFrame->eEncType);
    }
    ptAVFrame = av_frame_alloc();
    if(NULL==ptAVFrame)
    {
        CODEC_LOGE("NULL==ptAVFrame err \r\n");
        return iRet;
    }
    
    iRet=m_pAudioDecode->Decode(i_pSrcFrame->pbFrameBuf,i_pSrcFrame->iFrameBufLen,,i_pSrcFrame->ddwPTS,,i_pSrcFrame->ddwDTS,ptAVFrame);
    if(iRet<0)
    {
        CODEC_LOGE("m_pAudioDecode->Decode err \r\n");
        return iRet;
    }


    if(NULL== m_pAudioRawHandle)
    {
        m_pAudioRawHandle = new AudioRawHandle();
        if(NULL==m_pAudioRawHandle)
        {
            CODEC_LOGE("NULL==m_pAudioRawHandle err \r\n");
            return iRet;
        }
        AVCodecContext *ptCodecContext=NULL;
        iRet=m_pAudioDecode->GetCodecContext(&ptCodecContext);
        time_t rawtime = time(NULL);
        struct tm *timeinfo = localtime(&rawtime); // ?????????????????
        snprintf(strTemp,sizeof(strTemp),"drawtext=fontfile=msyhbd.ttc:fontcolor=red:fontsize=25:x=50:y=20:text='%s'",asctime(timeinfo));
        iRet=m_pAudioRawHandle->Init(ptCodecContext,strTemp);
    }
    iRet=m_pAudioRawHandle->RawHandle(ptAVFrame);
    if(iRet<0)
    {
        CODEC_LOGE("m_pAudioRawHandle->RawHandle err \r\n");
        return iRet;
    }


    if(NULL== m_pAudioEncode)
    {
        m_pAudioEncode = new AudioEncode();
        if(NULL==m_pAudioEncode)
        {
            CODEC_LOGE("NULL==m_pAudioEncode err \r\n");
            return iRet;
        }
        iRet=m_pAudioEncode->Init(o_pDstFrame->eEncType,o_pDstFrame->iFrameRate,(int)o_pDstFrame->dwWidth,(int)o_pDstFrame->dwHeight);
    }
    iRet=m_pAudioEncode->Encode(ptAVFrame,o_pDstFrame->pbFrameBuf,(unsigned int)o_pDstFrame->iFrameBufMaxLen,&o_pDstFrame->iFrameRate,&o_pDstFrame->eFrameType);
    if(iRet<=0)
    {
        CODEC_LOGE("m_pAudioEncode->Encode err \r\n");
        return -1;
    }
    o_pDstFrame->iFrameBufLen=iRet;
    return iRet;
}

