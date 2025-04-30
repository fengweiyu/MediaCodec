/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       MediaTranscode.cpp
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "MediaTranscode.h"
#include "MediaTranscodeCom.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <functional>
#include <time.h>

//#include <sys/time.h>


/*****************************************************************************
-Fuction        : MediaTranscode
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
MediaTranscode::MediaTranscode()
{
    m_pVideoTransform = NULL;
    m_pAudioTransform = new AudioTransform();
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
MediaTranscode::~MediaTranscode()
{
    if(NULL!= m_pAudioTransform)
    {
        delete m_pAudioTransform;
        m_pAudioTransform = NULL;
    }
    if(NULL!= m_pVideoTransform)
    {
        delete m_pVideoTransform;
        m_pVideoTransform = NULL;
    }
}

/*****************************************************************************
-Fuction        : Transform
-Description    : o_pDstFrame可能输出多帧，由外层解析库解析
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int MediaTranscode::SetWaterMarkParam(int i_iEnable,const char * i_strText,const char * i_strFontFile)
{
    int iRet = -1;

    if(NULL== i_strText || NULL== i_strFontFile)
    {
        CODEC_LOGE("NULL== i_strText || NULL== i_strFontFile err \r\n");
        return iRet;
    }
    
    if(NULL== m_pVideoTransform)
    {
        m_pVideoTransform = new VideoTransform();
        if(NULL==m_pVideoTransform)
        {
            CODEC_LOGE("NULL==m_pVideoTransform err \r\n");
            return iRet;
        }
    }
    iRet=m_pVideoTransform->SetWaterMarkParam(i_iEnable,i_strText,i_strFontFile);
    return iRet;
}

/*****************************************************************************
-Fuction        : DecodeToRGB
-Description    : 解码输出RGBA数据
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int MediaTranscode::DecodeToRGB(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame)
{
    int iRet = -1;

    if(NULL== i_pSrcFrame || NULL== o_pDstFrame)
    {
        CODEC_LOGE("NULL== i_pSrcFrame || NULL== o_pDstFrame err \r\n");
        return iRet;
    }
    
    if(i_pSrcFrame->eFrameType == CODEC_FRAME_TYPE_AUDIO_FRAME)
    {
        CODEC_LOGE("i_pSrcFrame->eFrameType AUDIO err %d\r\n", i_pSrcFrame->eFrameType);
        return iRet;
    }
    
    if(NULL== m_pVideoTransform)
    {
        m_pVideoTransform = new VideoTransform();
        if(NULL==m_pVideoTransform)
        {
            CODEC_LOGE("NULL==m_pVideoTransform err \r\n");
            return iRet;
        }
    }
    iRet=m_pVideoTransform->DecodeToRGB(i_pSrcFrame,o_pDstFrame);
    return iRet;
}

/*****************************************************************************
-Fuction        : Transform
-Description    : o_pDstFrame可能输出多帧，由外层解析库解析
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int MediaTranscode::Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame)
{
    int iRet = -1;

    if(NULL== i_pSrcFrame || NULL== o_pDstFrame)
    {
        CODEC_LOGE("NULL== i_pSrcFrame || NULL== o_pDstFrame err \r\n");
        return iRet;
    }
    
    if(i_pSrcFrame->eFrameType == CODEC_FRAME_TYPE_AUDIO_FRAME)
    {
        if(NULL== m_pAudioTransform)
        {
            CODEC_LOGE("NULL== m_pAudioTransform \r\n");
            return iRet;
        }
        iRet=m_pAudioTransform->Transform(i_pSrcFrame,o_pDstFrame);
        return iRet;
    }
    
    if(NULL== m_pVideoTransform)
    {
        m_pVideoTransform = new VideoTransform();
        if(NULL==m_pVideoTransform)
        {
            CODEC_LOGE("NULL==m_pVideoTransform err \r\n");
            return iRet;
        }
    }
    iRet=m_pVideoTransform->Transform(i_pSrcFrame,o_pDstFrame);
    return iRet;
}

/*****************************************************************************
-Fuction        : GetDstFrame
-Description    : 
-Input          : 
-Output         : 
-Return         : >0表示取到帧的大小，=0表示帧数据取完了
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int MediaTranscode::GetDstFrame(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame)
{
    int iRet = -1;

    if(NULL== i_pSrcFrame || NULL== o_pDstFrame)
    {
        CODEC_LOGE("NULL== i_pSrcFrame || NULL== o_pDstFrame err \r\n");
        return iRet;
    }
    
    if(i_pSrcFrame->eFrameType == CODEC_FRAME_TYPE_AUDIO_FRAME)
    {
        iRet=m_pAudioTransform->GetDstFrame(o_pDstFrame);
        return iRet;
    }

    return 0;
}

