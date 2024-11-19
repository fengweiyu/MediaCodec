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
-Fuction		: ~MediaTranscodeDemo
-Description	: ~MediaTranscodeDemo
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int MediaTranscodeDemo :: proc(const char * i_strJsonFileName)
{
	int iRet = -1,iReadLen=0;
	FILE  *pJsonFile=NULL;
    T_CodecFrame tVideoSrcFrame,tVideoDstFrame;
    T_CodecFrame tAudioSrcFrame,tAudioDstFrame;
    char * strFileBuf=NULL;
    char strSrcFilePath[128];
    char strDstFilePath[128];


    if(NULL== i_strJsonFileName)
    {
        CODEC_LOGE("NULL== i_strJsonFileName err \r\n");
        return iRet;
    }
    pJsonFile = fopen(i_strJsonFileName,"rb");//
    if(NULL == pJsonFile)
    {
        CODEC_LOGE("fopen %s err\r\n",i_strJsonFileName);
        return iRet;
    } 
    do
    {
        strFileBuf = new char [10*1024];
        if(NULL == strFileBuf)
        {
            CODEC_LOGE("NULL == pbFileBuf err\r\n");
            break;
        } 
        iReadLen = fread(strFileBuf, 1,10*1024, pJsonFile);
        if(iReadLen <= 0)
        {
            CODEC_LOGE("fread err %s iReadLen%d\r\n",i_strJsonFileName,iReadLen);
            break;
        }
        memset(&tVideoSrcFrame,0,sizeof(T_CodecFrame));
        memset(&tVideoDstFrame,0,sizeof(T_CodecFrame));
        memset(&tAudioSrcFrame,0,sizeof(T_CodecFrame));
        memset(&tAudioDstFrame,0,sizeof(T_CodecFrame));
        memset(&strSrcFilePath,0,sizeof(strSrcFilePath));
        memset(&strDstFilePath,0,sizeof(strDstFilePath));
        iRet =CreateCodecFrameFromJSON((const char *)strFileBuf,strSrcFilePath,sizeof(strSrcFilePath),&tVideoSrcFrame,&tAudioSrcFrame
        ,strDstFilePath,sizeof(strDstFilePath),&tVideoDstFrame,&tAudioDstFrame);
        if(iRet < 0)
        {
            CODEC_LOGE("CreateCodecFrameFromJSON err %s iReadLen%d\r\n",i_strJsonFileName,iReadLen);
            break;
        }
        iRet =Transcode(strSrcFilePath,strDstFilePath,&tVideoSrcFrame,&tAudioSrcFrame,&tVideoDstFrame,&tAudioDstFrame);
    }while(0);
    
    if(NULL != strFileBuf)
    {
        delete [] strFileBuf;
    } 
    if(NULL != pJsonFile)
    {
        fclose(pJsonFile);//fseek(m_pMediaFile,0,SEEK_SET); 
    } 
    return iRet;
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
int MediaTranscodeDemo :: Transcode(const char * i_strSrcFileName,const char * i_strDstFileName,T_CodecFrame *i_ptVideoSrcFrame,T_CodecFrame *i_ptAudioSrcFrame,T_CodecFrame *i_ptVideoDstFrame,T_CodecFrame *i_ptAudioDstFrame)
{
	int iRet = -1,iWriteLen=0;
    MediaTranscodeInf oMediaTranscodeInf;
    T_MediaFrameInfo tFileFrameInfo;
    MediaHandle oMediaHandle;
    unsigned char * pbFileBuf=NULL;
    unsigned char * pbOutFileBuf=NULL;
    MediaHandle oOutMediaHandle;
	FILE  *pMediaFile=NULL;
    T_CodecFrame tSrcFrame,tDstFrame;
    T_CodecFrame *ptSrcFrame=NULL;
    T_CodecFrame *ptDstFrame=NULL;
    
    if(NULL== i_strSrcFileName ||NULL== i_strDstFileName)
    {
        CODEC_LOGE("NULL== i_strSrcFileName ||NULL== i_strDstFileName err \r\n");
        return iRet;
    }
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
        pbOutFileBuf = new unsigned char [MEDIA_TRANS_FRAME_BUF_MAX_LEN];
        if(NULL == pbOutFileBuf)
        {
            CODEC_LOGE("NULL == pbOutFileBuf err\r\n");
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
            
            if(tFileFrameInfo.eFrameType == MEDIA_FRAME_TYPE_AUDIO_FRAME)
            {
                ptSrcFrame=i_ptAudioSrcFrame;
                ptDstFrame=i_ptAudioDstFrame;
            } 
            else
            {
                ptSrcFrame=i_ptVideoSrcFrame;
                ptDstFrame=i_ptVideoDstFrame;
            }
            if(NULL == ptDstFrame)
            {
                memset(&tDstFrame,0,sizeof(T_CodecFrame));
                CreateCodecFrameDefault(i_strDstFileName,pbFileBuf, MEDIA_TRANS_FRAME_BUF_MAX_LEN,&tDstFrame);
            } 
            else
            {
                memset(&tDstFrame,0,sizeof(T_CodecFrame));
                memcpy(&tDstFrame,ptDstFrame,sizeof(T_CodecFrame));
                tDstFrame.pbFrameBuf=pbFileBuf;
                tDstFrame.iFrameBufMaxLen=MEDIA_TRANS_FRAME_BUF_MAX_LEN;
            }
            if(NULL != ptSrcFrame)
            {
                memset(&tSrcFrame,0,sizeof(T_CodecFrame));
                memcpy(&tSrcFrame,ptSrcFrame,sizeof(T_CodecFrame));
            } 

            //CODEC_LOGD("oMediaTranscodeInf.Transform eEncType %d\r\n",tDstFrame.eEncType);
            MediaFrameToCodecFrame(&tFileFrameInfo, &tSrcFrame);
            iRet =oMediaTranscodeInf.Transform(&tSrcFrame, &tDstFrame);
            if(iRet < 0)
            {
                CODEC_LOGE("oMediaTranscodeInf.Transform err %s\r\n",i_strDstFileName);
                break;
            } 
            do
            {
                if(tDstFrame.iFrameBufLen == 0)
                {
                    CODEC_LOGW("tDstFrame.iFrameBufLen == 0 %s\r\n",i_strDstFileName);
                    break;
                } 
                iWriteLen = SaveDstFrame(&tDstFrame,&oOutMediaHandle,GetStreamType(i_strDstFileName),pbOutFileBuf,MEDIA_TRANS_FRAME_BUF_MAX_LEN);
                if(iWriteLen <= 0)
                {
                    CODEC_LOGE("SaveDstFrame err %s\r\n",i_strDstFileName);
                    iRet=-1;
                    break;
                } 
                iRet = fwrite(pbOutFileBuf,1,iWriteLen,pMediaFile);
                if(iRet != iWriteLen)
                {
                    CODEC_LOGE("fwrite err %d iWriteLen%d\r\n",iRet,iWriteLen);
                    iRet=-1;
                    break;
                }
                
                iRet =oMediaTranscodeInf.GetDstFrame(&tSrcFrame, &tDstFrame);
                if(iRet < 0)
                {
                    CODEC_LOGE("oMediaTranscodeInf.Transform err %s\r\n",i_strDstFileName);
                    break;
                } 
            }while(iRet>0);
            
            if(iRet < 0)
            {
                CODEC_LOGE("MediaTranscodeInf.Transform err %s\r\n",i_strDstFileName);
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
    if(NULL != pbOutFileBuf)
    {
        delete [] pbOutFileBuf;
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
int MediaTranscodeDemo :: SaveDstFrame(T_CodecFrame * i_ptCodecFrame,MediaHandle *i_pMediaHandle,E_StreamType i_eStreamType,unsigned char *o_pbBuf,int i_iMaxLen)
{
    int iRet = -1,iHeaderLen;
    T_MediaFrameInfo tMediaFrame;

    if(NULL== i_ptCodecFrame || NULL== o_pbBuf || NULL== i_pMediaHandle)
    {
        CODEC_LOGE("NULL== i_ptCodecFrame || NULL== o_pbBuf || NULL== i_pMediaHandle err \r\n");
        return iRet;
    }
    memset(&tMediaFrame,0,sizeof(T_MediaFrameInfo));
    iRet = CodecFrameToMediaFrame(i_ptCodecFrame,&tMediaFrame);
    tMediaFrame.eStreamType=STREAM_TYPE_MUX_STREAM;

    tMediaFrame.iFrameLen = 0;
    iRet =i_pMediaHandle->GetFrame(&tMediaFrame);
    if(tMediaFrame.iFrameLen <= 0)
    {
        CODEC_LOGE("tMediaFrame.iFrameLen <= 0 %d\r\n",tMediaFrame.iFrameLen);
        return iRet;
    } 

    return i_pMediaHandle->FrameToContainer(&tMediaFrame,i_eStreamType,o_pbBuf,(unsigned int)i_iMaxLen,&iHeaderLen);
}

/*****************************************************************************
-Fuction		: GetStreamType
-Description	: GetStreamType
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
E_StreamType MediaTranscodeDemo :: GetStreamType(const char *i_strFileName)
{
    int iRet = -1,iHeaderLen;
    E_StreamType eStreamType=STREAM_TYPE_UNKNOW;
    
    if(NULL== i_strFileName)
    {
        CODEC_LOGE("NULL== i_strFileName err \r\n");
        return eStreamType;
    }

    if(NULL!= strstr(i_strFileName,".h265") ||NULL!= strstr(i_strFileName,".h264"))
    {
        eStreamType=STREAM_TYPE_VIDEO_STREAM;
    }
    else if(NULL!= strstr(i_strFileName,".aac")||NULL!= strstr(i_strFileName,".g711a"))
    {
        eStreamType=STREAM_TYPE_AUDIO_STREAM;
    }
    else if(NULL!= strstr(i_strFileName,".flv"))
    {
        eStreamType=STREAM_TYPE_ENHANCED_FLV_STREAM;
    }
    else if(NULL!= strstr(i_strFileName,".mp4"))
    {
        eStreamType=STREAM_TYPE_FMP4_STREAM;
    }
    else
    {
        CODEC_LOGE("i_strDstFileName unknow %s \r\n",i_strFileName);
    }
    return eStreamType;
}

/*****************************************************************************
-Fuction		: CodecFrameToMediaFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int MediaTranscodeDemo :: CodecFrameToMediaFrame(T_CodecFrame * i_ptCodecFrame,T_MediaFrameInfo * m_ptMediaFrame)
{
    int iRet = -1;

    if(NULL== i_ptCodecFrame || NULL== m_ptMediaFrame)
    {
        CODEC_LOGE("NULL== i_ptCodecFrame || NULL== m_ptMediaFrame err \r\n");
        return iRet;
    }
    m_ptMediaFrame->pbFrameBuf=i_ptCodecFrame->pbFrameBuf;
    m_ptMediaFrame->iFrameBufMaxLen=i_ptCodecFrame->iFrameBufLen;//
    m_ptMediaFrame->iFrameBufLen=i_ptCodecFrame->iFrameBufLen;

    switch (i_ptCodecFrame->eFrameType)
    {
        case CODEC_FRAME_TYPE_VIDEO_I_FRAME:
        {
            m_ptMediaFrame->eFrameType=MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
            break;
        }
        case CODEC_FRAME_TYPE_VIDEO_P_FRAME:
        {
            m_ptMediaFrame->eFrameType=MEDIA_FRAME_TYPE_VIDEO_P_FRAME;
            break;
        }
        case CODEC_FRAME_TYPE_VIDEO_B_FRAME:
        {
            m_ptMediaFrame->eFrameType=MEDIA_FRAME_TYPE_VIDEO_B_FRAME;
            break;
        }
        case CODEC_FRAME_TYPE_AUDIO_FRAME:
        {
            m_ptMediaFrame->eFrameType=MEDIA_FRAME_TYPE_AUDIO_FRAME;
            break;
        }
        default:
        {
            CODEC_LOGE("m_ptMediaFrame->eFrameType err%d %d\r\n",m_ptMediaFrame->eFrameType,i_ptCodecFrame->eFrameType);
            return iRet;
        }
    }
    switch (i_ptCodecFrame->eEncType)
    {
        case CODEC_TYPE_H264:
        {
            m_ptMediaFrame->eEncType=MEDIA_ENCODE_TYPE_H264;
            break;
        }
        case CODEC_TYPE_H265:
        {
            m_ptMediaFrame->eEncType=MEDIA_ENCODE_TYPE_H265;
            break;
        }
        case CODEC_TYPE_AAC:
        {
            m_ptMediaFrame->eEncType=MEDIA_ENCODE_TYPE_AAC;
            break;
        }
        case CODEC_TYPE_G711A:
        {
            m_ptMediaFrame->eEncType=MEDIA_ENCODE_TYPE_G711A;
            break;
        }
        case CODEC_TYPE_G711U:
        {
            m_ptMediaFrame->eEncType=MEDIA_ENCODE_TYPE_G711U;
            break;
        }
        default:
        {
            CODEC_LOGE("m_ptMediaFrame->eEncType err%d %d\r\n",m_ptMediaFrame->eEncType,i_ptCodecFrame->eEncType);
            return iRet;
        }
    }
    m_ptMediaFrame->dwTimeStamp=(unsigned int)i_ptCodecFrame->ddwPTS;
    m_ptMediaFrame->dwSampleRate=i_ptCodecFrame->dwSampleRate;
    m_ptMediaFrame->dwWidth=i_ptCodecFrame->dwWidth;
    m_ptMediaFrame->dwHeight=i_ptCodecFrame->dwHeight;
    m_ptMediaFrame->tAudioEncodeParam.dwChannels=i_ptCodecFrame->dwChannels;
    
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
        {
            CODEC_LOGE("ptFrameInfo->eFrameType err%d %d\r\n",i_ptMediaFrame->eFrameType,i_ptCodecFrame->eFrameType);
            return iRet;
        }
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
            CODEC_LOGE("ptFrameInfo->eEncType err%d %d\r\n",i_ptMediaFrame->eEncType,i_ptCodecFrame->eEncType);
            return iRet;
        }
    }
    i_ptCodecFrame->ddwDTS=(int64_t)i_ptMediaFrame->dwTimeStamp;
    i_ptCodecFrame->ddwPTS=(int64_t)i_ptMediaFrame->dwTimeStamp;
    i_ptCodecFrame->iFrameRate=i_ptCodecFrame->iFrameRate>0?i_ptCodecFrame->iFrameRate : 25;
    i_ptCodecFrame->dwSampleRate=i_ptMediaFrame->dwSampleRate>0?i_ptMediaFrame->dwSampleRate : i_ptCodecFrame->dwSampleRate;//文件中能解析到则以文件为准
    i_ptCodecFrame->dwWidth=i_ptMediaFrame->dwWidth;
    i_ptCodecFrame->dwHeight=i_ptMediaFrame->dwHeight;
    i_ptCodecFrame->dwChannels=i_ptMediaFrame->tAudioEncodeParam.dwChannels;
    //CODEC_LOGD("ptFrameInfo->dwTimeStamp %d ,%lld %lld\r\n",i_ptMediaFrame->dwTimeStamp,i_ptCodecFrame->ddwPTS,i_ptCodecFrame->ddwDTS);
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
int MediaTranscodeDemo :: CreateCodecFrameDefault(const char * i_strDstFileName,unsigned char *pbCodecBuf,int iCodecBufMaxLen,T_CodecFrame * i_ptCodecFrame)
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
        CODEC_LOGW("i_strDstFileName unknow,use default enc %s \r\n",i_strDstFileName);
        i_ptCodecFrame->eEncType=CODEC_TYPE_H264;
    }
    i_ptCodecFrame->pbFrameBuf=pbCodecBuf;
    i_ptCodecFrame->iFrameBufMaxLen=iCodecBufMaxLen;

    i_ptCodecFrame->iFrameRate=25;
    i_ptCodecFrame->dwWidth=640;//宽高最好要和原来保持一致(否则要用filter去缩放处理)//,scale=xx:h=xx //scale=w=iw/2:h=ih/2 //scale=320:-1
    i_ptCodecFrame->dwHeight=480;//如果不一致，特别是变小的情况下，直接去编码，会导致画面被裁减 乃至绿屏，甚至程序会奔溃
    
    return 0;
}

/*****************************************************************************
-Fuction		: CreateCodecFrameFromJSON
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int MediaTranscodeDemo :: CreateCodecFrameFromJSON(const char * i_strJSON,char * o_strSrcFilePath,int i_iSrcBufMax,T_CodecFrame * o_ptVideoSrcCodecFrame,T_CodecFrame * o_ptAudioSrcCodecFrame,char * o_strDstFilePath,int i_iDstBufMax,T_CodecFrame * o_ptVideoDstCodecFrame,T_CodecFrame * o_ptAudioDstCodecFrame)
{
    int iRet = -1;
    cJSON * ptRootJson = NULL;
    cJSON * ptSrcJson = NULL;
    cJSON * ptDstJson = NULL;
    cJSON * ptNode = NULL;


    if(NULL== i_strJSON || NULL== o_ptVideoSrcCodecFrame  || NULL== o_ptAudioSrcCodecFrame || NULL== o_strSrcFilePath || NULL== o_strDstFilePath|| NULL== o_ptVideoDstCodecFrame || NULL== o_ptAudioDstCodecFrame)
    {
        CODEC_LOGE("NULL== i_strJSON || NULL== o_strSrcFilePath || NULL== o_strDstFilePath ||NULL== o_ptSrcCodecFrame || NULL== o_ptDstCodecFram err \r\n");
        return iRet;
    }
    
    ptRootJson = cJSON_Parse(i_strJSON);
    if(NULL== ptRootJson)
    {
        CODEC_LOGE("cJSON_Parse err %s\r\n",i_strJSON);
        return iRet;
    }
    ptSrcJson = cJSON_GetObjectItem(ptRootJson,"src");
    iRet =GetFrameInfo(ptSrcJson,o_ptVideoSrcCodecFrame,o_ptAudioSrcCodecFrame,o_strSrcFilePath,i_iSrcBufMax);
    ptDstJson = cJSON_GetObjectItem(ptRootJson,"dst");
    iRet |=GetFrameInfo(ptDstJson,o_ptVideoDstCodecFrame,o_ptAudioDstCodecFrame,o_strDstFilePath,i_iDstBufMax);
    
    cJSON_Delete(ptRootJson);
    
    return iRet;
}

/*****************************************************************************
-Fuction		: GetFrameInfo
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int MediaTranscodeDemo :: GetFrameInfo(cJSON * i_ptFrameJson,T_CodecFrame * o_ptVideoCodecFrame,T_CodecFrame * o_ptAudioCodecFrame,char * o_strFilePath,int i_iBufMax)
{
    int iRet = -1;
    cJSON * ptNode = NULL;
    E_CodecType eVideoEncType=CODEC_TYPE_UNKNOW;
    E_CodecType eAudioEncType=CODEC_TYPE_UNKNOW;
    int iFrameRate=0;//
    unsigned int dwSampleRate=0;//dwSamplesPerSecond
    unsigned int dwWidth=0;//
    unsigned int dwHeight=0;//
    unsigned int dwChannels=0;//音频通道个数
    //如果不设置则使用转码后的格式需要多少采样就用多少,如果转码后的格式对采样数没要求，则解码出多少采样就使用多少采样
    int iAudioFrameSize=0;//音频数据每帧固定长度大小,比如8000采样率的pcma 固定长度是160。


    if(NULL== i_ptFrameJson ||NULL== o_strFilePath || NULL== o_ptVideoCodecFrame|| NULL== o_ptAudioCodecFrame)
    {
        CODEC_LOGE("NULL== i_ptFrameJson || NULL== o_strFilePath ||NULL== o_ptCodecFrame err \r\n");
        return iRet;
    }
    ptNode = cJSON_GetObjectItem(i_ptFrameJson,"path");
    if(NULL != ptNode && NULL != ptNode->valuestring)
    {
        snprintf(o_strFilePath,i_iBufMax,"%s",ptNode->valuestring);
        ptNode = NULL;
    }
    ptNode = cJSON_GetObjectItem(i_ptFrameJson,"VideoCodec");
    if(NULL != ptNode && NULL != ptNode->valuestring)
    {
        if(0 == strcmp(ptNode->valuestring,"h264"))
        {
            eVideoEncType=CODEC_TYPE_H264;
        }
        else if(0 == strcmp(ptNode->valuestring,"h265"))
        {
            eVideoEncType=CODEC_TYPE_H265;
        }
        else
        {
            CODEC_LOGE("ptNode->valuestring err %s\r\n",ptNode->valuestring);
            return iRet;
        }
        ptNode = NULL;
    }
    ptNode = cJSON_GetObjectItem(i_ptFrameJson,"AudioCodec");
    if(NULL != ptNode && NULL != ptNode->valuestring)
    {
        if(0 == strcmp(ptNode->valuestring,"g711a"))
        {
            eAudioEncType=CODEC_TYPE_G711A;
        }
        else if(0 == strcmp(ptNode->valuestring,"g711u"))
        {
            eAudioEncType=CODEC_TYPE_G711U;
        }
        else if(0 == strcmp(ptNode->valuestring,"aac"))
        {
            eAudioEncType=CODEC_TYPE_AAC;
        }
        else
        {
            CODEC_LOGE("ptNode->valuestring err %s\r\n",ptNode->valuestring);
            return iRet;
        }
        ptNode = NULL;
    }
    ptNode = cJSON_GetObjectItem(i_ptFrameJson,"FrameRate");
    if(NULL != ptNode)
    {
        iFrameRate=ptNode->valueint;
        ptNode = NULL;
    }
    ptNode = cJSON_GetObjectItem(i_ptFrameJson,"SampleRate");
    if(NULL != ptNode)
    {
        dwSampleRate=(unsigned int)ptNode->valueint;
        ptNode = NULL;
    }
    ptNode = cJSON_GetObjectItem(i_ptFrameJson,"VideoWidth");
    if(NULL != ptNode)
    {
        dwWidth=(unsigned int)ptNode->valueint;
        ptNode = NULL;
    }
    ptNode = cJSON_GetObjectItem(i_ptFrameJson,"VideoHeight");
    if(NULL != ptNode)
    {
        dwHeight=(unsigned int)ptNode->valueint;
        ptNode = NULL;
    }
    ptNode = cJSON_GetObjectItem(i_ptFrameJson,"AudioChannels");
    if(NULL != ptNode)
    {
        dwChannels=(unsigned int)ptNode->valueint;
        ptNode = NULL;
    }
    ptNode = cJSON_GetObjectItem(i_ptFrameJson,"AudioFrameSize");
    if(NULL != ptNode)
    {
        iAudioFrameSize=(unsigned int)ptNode->valueint;
        ptNode = NULL;
    }

    CODEC_LOGD("CodecFrame->eEncType Video%d Audio%d ,iFrameRate %d,dwSampleRate %d,dwWidth %d,dwHeight %d,dwChannels %d,iAudioFrameSize%d\r\n",
    eVideoEncType,eAudioEncType,iFrameRate,dwSampleRate,dwWidth,dwHeight,dwChannels,iAudioFrameSize);
    o_ptVideoCodecFrame->eEncType=eVideoEncType;
    o_ptVideoCodecFrame->iFrameRate=iFrameRate;
    o_ptVideoCodecFrame->dwSampleRate=0;
    o_ptVideoCodecFrame->dwWidth=dwWidth;//宽高最好要和原来保持一致(否则要用filter去缩放处理)//,scale=xx:h=xx //scale=w=iw/2:h=ih/2 //scale=320:-1
    o_ptVideoCodecFrame->dwHeight=dwHeight;//如果不一致，特别是变小的情况下，直接去编码，会导致画面被裁减 乃至绿屏，甚至程序会奔溃
    o_ptVideoCodecFrame->dwChannels=dwChannels;
    o_ptVideoCodecFrame->iAudioFrameSize=iAudioFrameSize;

    o_ptAudioCodecFrame->eEncType=eAudioEncType;
    o_ptAudioCodecFrame->iFrameRate=iFrameRate;
    o_ptAudioCodecFrame->dwSampleRate=dwSampleRate;
    o_ptAudioCodecFrame->dwWidth=dwWidth;//宽高最好要和原来保持一致(否则要用filter去缩放处理)//,scale=xx:h=xx //scale=w=iw/2:h=ih/2 //scale=320:-1
    o_ptAudioCodecFrame->dwHeight=dwHeight;//如果不一致，特别是变小的情况下，直接去编码，会导致画面被裁减 乃至绿屏，甚至程序会奔溃
    o_ptAudioCodecFrame->dwChannels=dwChannels;
    o_ptAudioCodecFrame->iAudioFrameSize=iAudioFrameSize;
    return 0;
}


