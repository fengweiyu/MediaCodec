/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       VideoTransform.cpp
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "VideoTransform.h"
#include "MediaTranscodeCom.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <functional>
#include <time.h>

//#include <sys/time.h>


/*****************************************************************************
-Fuction        : VideoTransform
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
VideoTransform::VideoTransform()
{
    m_pVideoDecode = NULL;
    m_pVideoRawHandle = NULL;
    m_pVideoEncode = NULL;

}
/*****************************************************************************
-Fuction        : ~VideoTransform
-Description    : ~VideoTransform
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
VideoTransform::~VideoTransform()
{
    if(NULL!= m_pVideoDecode)
    {
        delete m_pVideoDecode;
        m_pVideoDecode = NULL;
    }
    if(NULL!= m_pVideoRawHandle)
    {
        delete m_pVideoRawHandle;
        m_pVideoRawHandle = NULL;
    }
    if(NULL!= m_pVideoEncode)
    {
        delete m_pVideoEncode;
        m_pVideoEncode = NULL;
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
int VideoTransform::Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame)
{
    int iRet = -1;
    AVFrame * ptAVFrame=NULL;
    char strTemp[512] = { 0 };

    if(NULL== m_pVideoDecode)
    {
        m_pVideoDecode = new VideoDecode();
        if(NULL==m_pVideoDecode)
        {
            CODEC_LOGE("NULL==m_pVideoDecode err \r\n");
            return iRet;
        }
        iRet=m_pVideoDecode->Init(i_pSrcFrame->eEncType);
    }
    ptAVFrame = av_frame_alloc();
    if(NULL==ptAVFrame)
    {
        CODEC_LOGE("NULL==ptAVFrame err \r\n");
        return iRet;
    }
    
    iRet=m_pVideoDecode->Decode(i_pSrcFrame->pbFrameBuf,i_pSrcFrame->iFrameBufLen,,i_pSrcFrame->ddwPTS,,i_pSrcFrame->ddwDTS,ptAVFrame);
    if(iRet<0)
    {
        CODEC_LOGE("m_pVideoDecode->Decode err \r\n");
        return iRet;
    }


    if(NULL== m_pVideoRawHandle)
    {
        m_pVideoRawHandle = new VideoRawHandle();
        if(NULL==m_pVideoRawHandle)
        {
            CODEC_LOGE("NULL==m_pVideoRawHandle err \r\n");
            return iRet;
        }
        AVCodecContext *ptCodecContext=NULL;
        iRet=m_pVideoDecode->GetCodecContext(&ptCodecContext);
        time_t rawtime = time(NULL);
        struct tm *timeinfo = localtime(&rawtime); // 将时间戳转换为本地时间
        snprintf(strTemp,sizeof(strTemp),"drawtext=fontfile=msyhbd.ttc:fontcolor=red:fontsize=25:x=50:y=20:text='%s'",asctime(timeinfo));
        iRet=m_pVideoRawHandle->Init(ptCodecContext,strTemp);
    }
    iRet=m_pVideoRawHandle->RawHandle(ptAVFrame);
    if(iRet<0)
    {
        CODEC_LOGE("m_pVideoRawHandle->RawHandle err \r\n");
        return iRet;
    }


    if(NULL== m_pVideoEncode)
    {
        m_pVideoEncode = new VideoEncode();
        if(NULL==m_pVideoEncode)
        {
            CODEC_LOGE("NULL==m_pVideoEncode err \r\n");
            return iRet;
        }
        iRet=m_pVideoEncode->Init(o_pDstFrame->eEncType,o_pDstFrame->iFrameRate,(int)o_pDstFrame->dwWidth,(int)o_pDstFrame->dwHeight);
    }
    iRet=m_pVideoEncode->Encode(ptAVFrame,o_pDstFrame->pbFrameBuf,(unsigned int)o_pDstFrame->iFrameBufMaxLen,&o_pDstFrame->iFrameRate,&o_pDstFrame->eFrameType);
    if(iRet<=0)
    {
        CODEC_LOGE("m_pVideoEncode->Encode err \r\n");
        return -1;
    }
    o_pDstFrame->iFrameBufLen=iRet;
    return iRet;
}

