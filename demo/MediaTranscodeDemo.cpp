/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       MediaTranscodeDemo.c
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "MediaTranscodeDemo.h"
#include "MediaTranscodeInf.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <utility>

using std::make_pair;

#define MEDIA_TRANS_FRAME_BUF_MAX_LEN	(2*1024*1024) 

/*****************************************************************************
-Fuction		: MediaTranscodeDemo
-Description	: MediaTranscodeDemo
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
MediaTranscodeDemo :: MediaTranscodeDemo()
{

}

/*****************************************************************************
-Fuction		: ~MediaTranscodeDemo
-Description	: ~MediaTranscodeDemo
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
MediaTranscodeDemo :: ~MediaTranscodeDemo()
{
}

/*****************************************************************************
-Fuction        : main
-Description    : main
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int MediaTranscodeDemo :: proc(const char * i_strSrcFileName,const char * i_strDstFileName)
{
	int iRet = -1,iWriteLen=0;
    MediaTranscodeInf oMediaTranscodeInf;
    T_MediaFrameInfo tFileFrameInfo;
    MediaHandle oMediaHandle;
    unsigned char * pbFileBuf=NULL;
	FILE  *pMediaFile=NULL;
    T_CodecFrame tSrcFrame,tDstFrame;


    oMediaHandle.Init((char *)i_strSrcFileName);//默认取文件流
    do
    {
        pbFileBuf = new unsigned char [MEDIA_TRANS_FRAME_BUF_MAX_LEN];
        if(NULL == pbFileBuf)
        {
            CODEC_LOGE("NULL == pbFileBuf err\r\n");
            break;
        } 
        pMediaFile = fopen(i_strDstFileName,"wb");//
        if(NULL == pMediaFile)
        {
            CODEC_LOGE("fopen %s err\r\n",i_strDstFileName);
            break;
        } 
        
        memset(&tFileFrameInfo,0,sizeof(T_MediaFrameInfo));
        tFileFrameInfo.pbFrameBuf = new unsigned char [MEDIA_TRANS_FRAME_BUF_MAX_LEN];
        if(NULL == tFileFrameInfo.pbFrameBuf)
        {
            CODEC_LOGE("NULL == m_tFileFrameInfo.pbFrameBuf err\r\n");
            break;
        } 
        tFileFrameInfo.iFrameBufMaxLen = MEDIA_TRANS_FRAME_BUF_MAX_LEN;

        while(1)
        {
            tFileFrameInfo.iFrameLen = 0;
            iRet =oMediaHandle.GetFrame(&tFileFrameInfo);
            if(tFileFrameInfo.iFrameLen <= 0)
            {
                CODEC_LOGE("m_tFileFrameInfo.iFrameLen <= 0 %s\r\n",i_strSrcFileName);
                break;
            } 

            memset(&tSrcFrame,0,sizeof(T_CodecFrame));
            MediaFrameToCodecFrame(&tFileFrameInfo, &tSrcFrame);
            memset(&tDstFrame,0,sizeof(T_CodecFrame));
            CreateCodecFrame(i_strDstFileName,pbFileBuf, MEDIA_TRANS_FRAME_BUF_MAX_LEN,&tDstFrame);
            iRet =oMediaTranscodeInf.Transform(&tSrcFrame, &tDstFrame);
            if(iRet < 0)
            {
                CODEC_LOGE("oMediaTranscodeInf.Transform err %s\r\n",i_strDstFileName);
                break;
            } 
            //iWriteLen = oMediaHandle.FrameToContainer(&tFileFrameInfo,STREAM_TYPE_FMP4_STREAM,pbFileBuf,MEDIA_FILE_BUF_MAX_LEN,&iHeaderLen);
            iRet = fwrite(tDstFrame.pbFrameBuf,1,tDstFrame.iFrameBufLen, pMediaFile);
            if(iRet != tDstFrame.iFrameBufLen)
            {
                CODEC_LOGE("fwrite err %d iWriteLen%d\r\n",iRet,iWriteLen);
                break;
            }
        }
    }while(0);
    
    
    if(NULL != tFileFrameInfo.pbFrameBuf)
    {
        delete [] tFileFrameInfo.pbFrameBuf;
    } 
    if(NULL != pbFileBuf)
    {
        delete [] pbFileBuf;
    } 
    if(NULL != pMediaFile)
    {
        fclose(pMediaFile);//fseek(m_pMediaFile,0,SEEK_SET); 
    } 
    CODEC_LOGW("~MEDIA_TRANS exit\r\n");
    return iRet;
}

/*****************************************************************************
-Fuction		: MediaFrameToCodecFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int MediaTranscodeDemo :: MediaFrameToCodecFrame(T_MediaFrameInfo * i_ptMediaFrame,T_CodecFrame * i_ptCodecFrame)
{
    int iRet = -1;

    if(NULL== i_ptMediaFrame || NULL== i_ptCodecFrame)
    {
        CODEC_LOGE("NULL== i_pMediaFrame || NULL== i_pCodecFrame err \r\n");
        return iRet;
    }
    i_ptCodecFrame->pbFrameBuf=i_ptMediaFrame->pbFrameStartPos;
    i_ptCodecFrame->iFrameBufMaxLen=i_ptMediaFrame->iFrameLen;//
    i_ptCodecFrame->iFrameBufLen=i_ptMediaFrame->iFrameLen;

    switch (i_ptMediaFrame->eFrameType)
    {
        case MEDIA_FRAME_TYPE_VIDEO_I_FRAME:
        {
            i_ptCodecFrame->eFrameType=CODEC_FRAME_TYPE_VIDEO_I_FRAME;
            break;
        }
        case MEDIA_FRAME_TYPE_VIDEO_P_FRAME:
        {
            i_ptCodecFrame->eFrameType=CODEC_FRAME_TYPE_VIDEO_P_FRAME;
            break;
        }
        case MEDIA_FRAME_TYPE_VIDEO_B_FRAME:
        {
            i_ptCodecFrame->eFrameType=CODEC_FRAME_TYPE_VIDEO_B_FRAME;
            break;
        }
        case MEDIA_FRAME_TYPE_AUDIO_FRAME:
        {
            i_ptCodecFrame->eFrameType=CODEC_FRAME_TYPE_AUDIO_FRAME;
            break;
        }
        default:
            break;
    }
    switch (i_ptMediaFrame->eEncType)
    {
        case MEDIA_ENCODE_TYPE_H264:
        {
            i_ptCodecFrame->eEncType=CODEC_TYPE_H264;
            break;
        }
        case MEDIA_ENCODE_TYPE_H265:
        {
            i_ptCodecFrame->eEncType=CODEC_TYPE_H265;
            break;
        }
        case MEDIA_ENCODE_TYPE_AAC:
        {
            i_ptCodecFrame->eEncType=CODEC_TYPE_AAC;
            break;
        }
        case MEDIA_ENCODE_TYPE_G711A:
        {
            i_ptCodecFrame->eEncType=CODEC_TYPE_G711A;
            break;
        }
        case MEDIA_ENCODE_TYPE_G711U:
        {
            i_ptCodecFrame->eEncType=CODEC_TYPE_G711U;
            break;
        }
        default:
        {
            CODEC_LOGE("ptFrameInfo->eEncType err%d\r\n",i_ptMediaFrame->eEncType);
            return iRet;
        }
    }
    i_ptCodecFrame->ddwDTS=i_ptMediaFrame->dwTimeStamp;
    i_ptCodecFrame->ddwPTS=i_ptMediaFrame->dwTimeStamp;
    i_ptCodecFrame->iFrameRate=25;
    i_ptCodecFrame->dwSampleRate=i_ptMediaFrame->dwSampleRate;
    i_ptCodecFrame->dwWidth=i_ptMediaFrame->dwWidth;
    i_ptCodecFrame->dwHeight=i_ptMediaFrame->dwHeight;
    
    return 0;
}

/*****************************************************************************
-Fuction		: MediaFrameToCodecFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int MediaTranscodeDemo :: CreateCodecFrame(const char * i_strDstFileName,unsigned char *pbCodecBuf,int iCodecBufMaxLen,T_CodecFrame * i_ptCodecFrame)
{
    int iRet = -1;

    if(NULL== pbCodecBuf || NULL== i_ptCodecFrame || NULL== i_strDstFileName)
    {
        CODEC_LOGE("NULL== pbCodecBuf || NULL== i_pCodecFrame err \r\n");
        return iRet;
    }

    if(NULL!= strstr(i_strDstFileName,".h265"))
    {
        i_ptCodecFrame->eEncType=CODEC_TYPE_H265;
    }
    else if(NULL!= strstr(i_strDstFileName,".h264"))
    {
        i_ptCodecFrame->eEncType=CODEC_TYPE_H264;
    }
    else
    {
        CODEC_LOGW("i_strDstFileName unknow,use default enc %d \r\n",i_strDstFileName);
        i_ptCodecFrame->eEncType=CODEC_TYPE_H264;
    }
    i_ptCodecFrame->pbFrameBuf=pbCodecBuf;
    i_ptCodecFrame->iFrameBufMaxLen=iCodecBufMaxLen;

    i_ptCodecFrame->iFrameRate=25;
    i_ptCodecFrame->dwWidth=640;//宽高最好要和原来保持一致(否则要用filter去缩放处理)//,scale=xx:h=xx //scale=w=iw/2:h=ih/2 //scale=320:-1
    i_ptCodecFrame->dwHeight=480;//如果不一致，特别是变小的情况下，直接去编码，会导致画面被裁减 乃至绿屏，甚至程序会奔溃
    
    return 0;
}


