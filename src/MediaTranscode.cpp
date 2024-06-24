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
    if(NULL!= m_pVideoTransform)
    {
        delete m_pVideoTransform;
        m_pVideoTransform = NULL;
    }
}

/*****************************************************************************
-Fuction        : Transform
-Description    : o_pDstFrame���������֡���������������
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
        CODEC_LOGE("i_pSrcFrame->eFrameType == CODEC_FRAME_TYPE_AUDIO_FRAME \r\n");
        return iRet;
    }
    
    if(NULL== m_pVideoTransform)
    {
        m_pVideoTransform = new VideoTransform();
        if(NULL==m_pVideoTransform)
        {
            CODEC_LOGE("NULL==m_pVideoDecode err \r\n");
            return iRet;
        }
    }
    iRet=m_pVideoTransform->Transform(i_pSrcFrame,o_pDstFrame);
    return iRet;
}

