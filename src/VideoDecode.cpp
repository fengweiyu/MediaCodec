/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       VideoDecode.cpp
* Description           : 	    
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "VideoDecode.h"
#include <regex>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "cJSON.h"

using std::string;
using std::smatch;
using std::regex;


/*****************************************************************************
-Fuction        : VideoDecode
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
VideoDecode::VideoDecode()
{
    m_ptPacket=NULL;
    m_ptParser = NULL;
    m_ptCodecContext = NULL;
    m_ptFrame = NULL;
}
/*****************************************************************************
-Fuction        : ~~VideoDecode
-Description    : ~~VideoDecode
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
VideoDecode::~VideoDecode()
{
    if(NULL!=m_ptParser)
    {
        av_parser_close(m_ptParser);
        m_ptParser=NULL;
    }
    if(NULL!=m_ptCodecContext)
    {
        avcodec_free_context(&m_ptCodecContext);
        m_ptCodecContext=NULL;
    }
    if(NULL!=m_ptFrame)
    {
        av_frame_free(&m_ptFrame);
        m_ptFrame=NULL;
    }
    if(NULL!=m_ptPacket)
    {
        av_packet_free(&m_ptPacket);
        m_ptPacket=NULL;
    }
}
/*****************************************************************************
-Fuction        : Init
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2023/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoDecode::Init(E_CodecType i_eCodecType)
{
    int iRet = -1;
    AVCodec * ptCodec;//��������ʹ�ú���avcodec_find_decoder���ߣ��ú�����Ҫ��id������������ptCodecContext�е�codec_id��Ա
    int iCodecID=AV_CODEC_ID_NONE;
    
    m_ptPacket = av_packet_alloc();
    if(NULL==m_ptPacket)
    {
        CODEC_LOGE("NULL==m_pPacket err \r\n");
        return iRet;
    }
    iCodecID=CodecTypeToAvCodecId(i_eCodecType);
    ptCodec = (AVCodec *)avcodec_find_decoder((enum AVCodecID)iCodecID);//���ҽ�����
    if(NULL==ptCodec)
    {
        CODEC_LOGE("NULL==ptCodec err \r\n");
        return iRet;
    }
    m_ptParser = av_parser_init(ptCodec->id);
    if(NULL==m_ptParser)
    {
        CODEC_LOGE("NULL==m_ptParser err \r\n");
        return iRet;
    }
    m_ptCodecContext = avcodec_alloc_context3(ptCodec);
    if(NULL==m_ptCodecContext)
    {
        CODEC_LOGE("NULL==m_ptCodecContext err \r\n");
        return iRet;
    }
    // Decoder Setting
    //m_ptCodecContext->codec_id = iCodecID;
    //m_ptCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    //m_ptCodecContext->pix_fmt = AV_PIX_FMT_NONE;
    //m_ptCodecContext->width = i_width_;
    //m_ptCodecContext->height = i_height_;
    //m_ptCodecContext->framerate = { 0, 1 };
    m_ptCodecContext->time_base = { 1, 1000 };//����ʱ��(��)��msΪ��λ
    //m_ptCodecContext->skip_frame = AVDISCARD_NONREF; // �����ǲο�֡
    //m_ptCodecContext->err_recognition = AV_EF_CRCCHECK | AV_EF_BITSTREAM | AV_EF_EXPLODE | AV_EF_COMPLIANT;
    if (iCodecID == AV_CODEC_ID_MJPEG) 
    {
        //m_ptCodecContext->gop_size = i_fps;
    }
    // m_ptCodecContext->time_base = {1, AV_TIME_BASE};
    m_ptCodecContext->thread_count = 1;
    if (ptCodec->capabilities | AV_CODEC_CAP_FRAME_THREADS) 
    {
      m_ptCodecContext->thread_type = FF_THREAD_FRAME;
    }
    else if (ptCodec->capabilities | AV_CODEC_CAP_SLICE_THREADS)
    {
      m_ptCodecContext->thread_type = FF_THREAD_SLICE;
    }
    else 
    {
        m_ptCodecContext->thread_count = 1; // don't use multithreading
    }
    //if (ptCodec->capabilities & AV_CODEC_CAP_TRUNCATED) 
    {
        //m_ptCodecContext->flags |= AV_CODEC_FLAG_TRUNCATED;
    }
    
    AVDictionary * ptOpts = NULL;
    av_dict_set(&ptOpts, "preset", "ultrafast", 0);
    av_dict_set(&ptOpts, "tune", "stillimage,fastdecode,zerolatency", 0);
    //if (avcodec_open2(m_ptCodecContext, ptCodec, &ptOpts) < 0)
    if (avcodec_open2(m_ptCodecContext, ptCodec, NULL)<0)
    {//�򿪽�����
        CODEC_LOGE("avcodec_open2 err %s \r\n",avcodec_get_name((enum AVCodecID)iCodecID));
        return iRet;
    }
    
    m_ptFrame = av_frame_alloc();
    if(NULL==m_ptFrame)
    {
        CODEC_LOGE("NULL==m_ptFrame err \r\n");
        return iRet;
    }

    return 0;
}
/*****************************************************************************
-Fuction        : Decode
-Description    : Decode
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2023/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoDecode::Decode(unsigned char * i_pbFrameData,unsigned int  i_dwFrameLen,int64_t i_ddwPTS,int64_t i_ddwDTS,AVFrame *o_ptAVFrame)
{
    int iRet = -1;
    unsigned char *pbFrameData=NULL;
    unsigned int dwFrameLen=0;
    int64_t ddwPTS=AV_NOPTS_VALUE;//ms
    int64_t ddwDTS=AV_NOPTS_VALUE;//ms


    if(NULL==m_ptFrame)
    {
        CODEC_LOGE("NULL==m_ptFrame Decode err \r\n");
        return iRet;
    }
    if(NULL==i_pbFrameData ||NULL==o_ptAVFrame)
    {
        CODEC_LOGE("NULL==i_pbFrameData ||NULL==o_ptAVFrame err \r\n");
        return iRet;
    }
    pbFrameData=i_pbFrameData;
    dwFrameLen=i_dwFrameLen;
    if(0 != i_ddwPTS)
        ddwPTS=i_ddwPTS;//ms
    if(0 != i_ddwDTS)
        ddwDTS=i_ddwDTS;//ms

    while (dwFrameLen > 0) 
    {
        /*av_init_packet(m_ptPacket);
        m_ptPacket->data = pbFrameData;
        m_ptPacket->size = dwFrameLen;
        m_ptPacket->pts = ddwPTS;
        m_ptPacket->dts = ddwDTS;*/
        iRet = av_parser_parse2(m_ptParser,m_ptCodecContext, &m_ptPacket->data, &m_ptPacket->size,pbFrameData, dwFrameLen, ddwPTS, ddwDTS, 0);
        if (iRet < 0) 
        {
            CODEC_LOGE("av_parser_parse2 err \r\n");
            return iRet;
        }
        pbFrameData += iRet;
        dwFrameLen -= iRet;
        if (m_ptPacket->size<=0)
        {
            CODEC_LOGE("av_parser_parse2 err m_ptPacket->size<=0\r\n");
            return iRet;
        }

        iRet = avcodec_send_packet(m_ptCodecContext, m_ptPacket);
        if (iRet < 0) 
        {
            CODEC_LOGE("avcodec_send_packet err \r\n");
            return iRet;
        }
        while (iRet >= 0) 
        {
            //av_frame_unref(m_ptFrame);//���������֡
            iRet = avcodec_receive_frame(m_ptCodecContext, m_ptFrame);
            if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF)
            {
                iRet=0;
                break;
            }
            else if (iRet < 0) 
            {
                CODEC_LOGE("avcodec_receive_frame err \r\n");
                av_frame_unref(m_ptFrame);
                return iRet;
            }
            /* the picture is allocated by the decoder. no need to
               free it */
            CODEC_LOGD("dec frame ,frame->data[0]%d, frame->linesize[0]%d,frame->width%d, frame->height%d \r\n", 
            /*m_ptCodecContext->frame_number,*/m_ptFrame->data[0], m_ptFrame->linesize[0],m_ptFrame->width, m_ptFrame->height);
        }
    }
    //av_frame_move_ref(o_ptAVFrame,m_ptFrame);//��������Ĳ��������Ż�Ϊ��һ��
    iRet=av_frame_ref(o_ptAVFrame,m_ptFrame);
    if (iRet < 0)
    {
        CODEC_LOGE("av_frame_ref err \r\n");
        av_frame_unref(m_ptFrame);
        return iRet;
    }
    iRet=av_frame_copy(o_ptAVFrame,m_ptFrame);
    if (iRet < 0)
    {
        CODEC_LOGE("av_frame_copy err \r\n");
        av_frame_unref(o_ptAVFrame);
    }
    av_frame_unref(m_ptFrame);
    return iRet;
}
/*****************************************************************************
-Fuction        : GetCodecContext
-Description    : GetCodecContext
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2023/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoDecode::GetCodecContext(AVCodecContext **o_ptCodecContext)
{
    int iRet = -1;

    if(NULL==m_ptFrame)
    {
        CODEC_LOGE("NULL==m_ptFrame GetCodecContext err \r\n");
        return iRet;
    }
    *o_ptCodecContext=m_ptCodecContext;
    return 0;
}

/*****************************************************************************
-Fuction        : CodecTypeToAvCodecId
-Description    : CodecTypeToAvCodecId
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2023/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoDecode::CodecTypeToAvCodecId(E_CodecType eCodecType)
{
    switch (eCodecType) 
    {
        case CODEC_TYPE_G711A:
            return AV_CODEC_ID_PCM_ALAW;
        case CODEC_TYPE_G711U:
            return AV_CODEC_ID_PCM_MULAW;
        case CODEC_TYPE_AAC:
            return AV_CODEC_ID_AAC;
        case CODEC_TYPE_H264:
            return AV_CODEC_ID_H264;
        case CODEC_TYPE_H265:
            return AV_CODEC_ID_HEVC;
        case CODEC_TYPE_MJPEG:
            return AV_CODEC_ID_MJPEG;
        case CODEC_TYPE_JPEG:
            return AV_CODEC_ID_MJPEG;
        default:
            return AV_CODEC_ID_NONE;
    }
}

