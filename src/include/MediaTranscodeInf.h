/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       MediaTranscodeInf.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef MEDIA_TRANSCODE_INF_H
#define MEDIA_TRANSCODE_INF_H

#include "MediaTranscodeCom.h"


/*****************************************************************************
-Class          : MediaTranscodeInf
-Description    : MediaTranscodeInf
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class MediaTranscodeInf
{
public:
	MediaTranscodeInf();
	virtual ~MediaTranscodeInf();
    int SetWaterMarkParam(int i_iEnable,const char * i_strText,const char * i_strFontFile);
    int DecodeToRGB(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame);
    int Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame);
    int GetDstFrame(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame);
private:
    void * m_pHandle;
};










#endif
