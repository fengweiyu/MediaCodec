/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       AudioTransform.h
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef AUDIO_TRANSFORM_H
#define AUDIO_TRANSFORM_H

#include "AudioDecode.h"
#include "AudioRawHandle.h"
#include "AudioEncode.h"
#include <thread>
#include <mutex>
#include <string>
#include <list>
#include <map>

using std::map;
using std::string;
using std::list;
using std::mutex;
using std::thread;



/*****************************************************************************
-Class          : AudioTransform
-Description    : AudioTransform
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2022/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class AudioTransform
{
public:
	AudioTransform();
	virtual ~AudioTransform();
    int Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame);
    int GetDstFrame(T_CodecFrame *o_pDstFrame);
    
private:
    AVFrame * m_ptAVFrame;

	AudioDecode *m_pAudioDecode;
	AudioEncode *m_pAudioEncode;
	AudioRawHandle *m_pAudioRawHandle;
};













#endif
