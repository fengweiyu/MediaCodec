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
    int proc(const char * i_strSrcFileName,const char * i_strDstFileName);
    
private:
    int MediaFrameToCodecFrame(T_MediaFrameInfo * i_ptMediaFrame,T_CodecFrame * i_ptCodecFrame);
    int CreateCodecFrame(const char * i_strDstFileName,unsigned char *pbCodecBuf,int iCodecBufMaxLen,T_CodecFrame * i_ptCodecFrame);
    
};

#endif
