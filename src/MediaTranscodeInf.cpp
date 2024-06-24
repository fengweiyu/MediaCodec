/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       MediaTranscodeInf.cpp
* Description           : 	    接口层，防止曝露内部文件
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "MediaTranscodeInf.h"
#include "MediaTranscode.h"

/*****************************************************************************
-Fuction        : MediaTranscodeInf
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
MediaTranscodeInf::MediaTranscodeInf()
{
    m_pHandle = NULL;
    m_pHandle = new MediaTranscode();
}
/*****************************************************************************
-Fuction        : ~MediaTranscodeInf
-Description    : ~MediaTranscodeInf
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
MediaTranscodeInf::~MediaTranscodeInf()
{
    if(NULL != m_pHandle)
    {
        MediaTranscode *pMediaTranscode = (MediaTranscode *)m_pHandle;
        delete pMediaTranscode;
    }  
}

/*****************************************************************************
-Fuction        : Transform
-Description    : Transform
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int MediaTranscodeInf::Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame)
{
    MediaTranscode *pMediaTranscode = (MediaTranscode *)m_pHandle;
    return pMediaTranscode->Transform(i_pSrcFrame,o_pDstFrame);
}

