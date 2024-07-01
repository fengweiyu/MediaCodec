/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       MediaTranscodeDemo.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef MEDIA_TRANSCODE_DEMO_H
#define MEDIA_TRANSCODE_DEMO_H

#include "MediaHandle.h"
#include "MediaTranscodeCom.h"
#include "cJSON.h"


/*****************************************************************************
-Class			: MediaTranscodeDemo
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class MediaTranscodeDemo
{
public:
	MediaTranscodeDemo();
	virtual ~MediaTranscodeDemo();
    int proc(const char * i_strJsonFileName);
    int Transcode(const char * i_strSrcFileName,const char * i_strDstFileName,T_CodecFrame *i_ptVideoSrcFrame=NULL,T_CodecFrame *i_ptAudioSrcFrame=NULL,T_CodecFrame *i_ptVideoDstFrame=NULL,T_CodecFrame *i_ptAudioDstFrame=NULL);
private:
    int SaveDstFrame(T_CodecFrame * i_ptCodecFrame,MediaHandle *i_pMediaHandle,unsigned char *o_pbBuf,int i_iMaxLen);
    int CodecFrameToMediaFrame(T_CodecFrame * i_ptCodecFrame,T_MediaFrameInfo * m_ptMediaFrame);
    int MediaFrameToCodecFrame(T_MediaFrameInfo * i_ptMediaFrame,T_CodecFrame * i_ptCodecFrame);
    int CreateCodecFrameDefault(const char * i_strDstFileName,unsigned char *pbCodecBuf,int iCodecBufMaxLen,T_CodecFrame * i_ptCodecFrame);
    int CreateCodecFrameFromJSON(const char * i_strJSON,char * o_strSrcFilePath,int i_iSrcBufMax,T_CodecFrame * o_ptVideoSrcCodecFrame,T_CodecFrame * o_ptAudioSrcCodecFrame,char * o_strDstFilePath,int i_iDstBufMax,T_CodecFrame * o_ptVideoDstCodecFrame,T_CodecFrame * o_ptAudioDstCodecFrame);
    int GetFrameInfo(cJSON * i_ptFrameJson,T_CodecFrame * o_ptVideoCodecFrame,T_CodecFrame * o_ptAudioCodecFrame,char * o_strFilePath,int i_iBufMax);
    
};

#endif
