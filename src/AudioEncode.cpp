/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       VideoEncode.cpp
* Description           : 	   
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "VideoEncode.h"
#include "MediaTranscodeCom.h"
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
-Fuction        : VideoEncode
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
VideoEncode::VideoEncode()
{
    m_ptPacket=NULL;
    m_ptCodecContext = NULL;
}
/*****************************************************************************
-Fuction        : ~HlsServer
-Description    : ~HlsServer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
VideoEncode::~VideoEncode()
{
    if(NULL!=m_ptCodecContext)
    {
        avcodec_free_context(&m_ptCodecContext);
        m_ptCodecContext=NULL;
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
int VideoEncode::Init(E_CodecType i_eCodecType,int i_iFrameRate,int i_iWidth,int i_iHeight)
{
    int iRet = -1;
    AVCodec         *ptCodec;//编码器，使用函数avcodec_find_decoder或者，该函数需要的id参数，来自于ptCodecContext中的codec_id成员
    int iCodecID=AV_CODEC_ID_NONE;
    
    
    iCodecID=CodecTypeToAvCodecId(i_eCodecType);
    ptCodec = avcodec_find_decoder(iCodecID);//查找解码器
    if(NULL==ptCodec)
    {
        CODEC_LOGE("NULL==ptCodec err \r\n");
        return iRet;
    }
    m_ptCodecContext = avcodec_alloc_context3(ptCodec);
    if(NULL==m_ptCodecContext)
    {
        CODEC_LOGE("NULL==m_ptCodecContext err \r\n");
        return iRet;
    }
    // Encoder Setting
    //m_ptCodecContext->bit_rate = 400000;/* put sample parameters */
    //m_ptCodecContext->codec_id = iCodecID;
    //m_ptCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    m_ptCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    m_ptCodecContext->width = i_iWidth;/* resolution must be a multiple of two */
    m_ptCodecContext->height = i_iHeight;
    m_ptCodecContext->me_range = 16;
    m_ptCodecContext->max_qdiff = 4;
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    m_ptCodecContext->gop_size = i_iFrameRate;// 每m_fps_帧插入1个I帧
    // 帧率
    m_ptCodecContext->framerate = { i_iFrameRate, 1 };/* frames per second */
    // m_ptCodecContext->time_base = {1, i_iFrameRate};
    m_ptCodecContext->time_base = { 1, 1000 };//ms
    // 最大和最小量化系数
    m_ptCodecContext->qmin = 20;
    m_ptCodecContext->qmax = 35;
    m_ptCodecContext->flags |= AV_CODEC_FLAG2_LOCAL_HEADER;
    // Optional Param
    m_ptCodecContext->max_b_frames = 0;
    if (iCodecID == AV_CODEC_ID_H264) 
    {
        m_ptCodecContext->profile = FF_PROFILE_H264_BASELINE;
        m_ptCodecContext->level = 31;
        //m_ptCodecContext->profile = FF_PROFILE_H264_MAIN;
        //m_ptCodecContext->profile = FF_PROFILE_H264_HIGH;
        //av_opt_set(m_ptCodecContext->priv_data, "x264-params", "qp=23:crf=32", 0);
        av_opt_set(m_ptCodecContext->priv_data, "x264-params", "qp=23", 0);
        //av_opt_set(m_ptCodecContext->priv_data, "qscale", "0", 0);
    
        //av_opt_set(m_ptCodecContext->priv_data, "preset", "placebo", 0);
        //av_opt_set(m_ptCodecContext->priv_data, "preset", "veryslow", 0);
        av_opt_set(m_ptCodecContext->priv_data, "preset", "slow", 0);
        av_opt_set(m_ptCodecContext->priv_data, "tune", "zerolatency", 0);
    }
    else if (iCodecID == AV_CODEC_ID_HEVC) 
    {
        // HEVC must be between 0.5 and 1.0.
        //m_ptCodecContext->qcompress = 0.6;
        //m_ptCodecContext->profile = FF_PROFILE_HEVC_MAIN;
        //av_opt_set(m_ptCodecContext->priv_data, "x265-params", "qp=31", 0);
        //av_opt_set(m_ptCodecContext->priv_data, "preset", "ultrafast", 0);
        //av_opt_set(m_ptCodecContext->priv_data, "tune", "zero-latency", 0);
        int iQp = 27;
        int iGop = 2;
        m_ptCodecContext->profile = FF_PROFILE_HEVC_MAIN;
        av_opt_set(m_ptCodecContext->priv_data, "preset", "ultrafast", 0); ///< 暂不支持其他预置方式  szPreSet.c_str()
        av_opt_set(m_ptCodecContext->priv_data, "tune", "zero-latency", 0);
        char strX265Params[1024];
        snprintf(strX265Params, sizeof(strX265Params),
            "scenecut-bias=0:ipratio=1:qp=%d:psy-rd=0.0:merge=0:wpp=0:"
            "rskip=0:lookahead-slices=0:pmode=0:pme=0:temporal-mvp=0:"
            "strong-intra-smoothing=0:min-cu-size=8:max-merge=3:me=1:"
            "subme=2:sao=1:rd=3:selective-sao=4:rskip=1:fast-intra=0:"
            "b-intra=1:fps=%d/1:keyint=%d:min-keyint=2",
            iQp, i_iFrameRate, i_iFrameRate * iGop);
        av_opt_set(m_ptCodecContext->priv_data, "x265-params", strX265Params, 0);
    }
    /* open it */
    if (avcodec_open2(m_ptCodecContext, ptCodec, NULL)<0)
    {//打开解码器
        CODEC_LOGE("avcodec_open2 err %s \r\n",avcodec_get_name(iCodecID));
        return iRet;
    }

    
    m_ptPacket = av_packet_alloc();
    if(NULL==m_ptPacket)
    {
        CODEC_LOGE("NULL==m_pPacket err \r\n");
        return iRet;
    }

    return 0;
}
/*****************************************************************************
-Fuction        : Encode
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2023/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoEncode::Encode(AVFrame *i_ptAVFrame,unsigned char * o_pbFrameData,unsigned int i_dwFrameMaxLen,int *o_iFrameRate,E_CodecFrameType *o_iFrameType)
{
    int iRet = -1;
    int iFrameRate = 0;

    if(NULL==m_ptPacket)
    {
        CODEC_LOGE("NULL==m_ptPacket Encode err \r\n");
        return iRet;
    }
    if(NULL==i_ptAVFrame ||NULL==o_pbFrameData ||NULL==o_iFrameRate ||NULL==o_iFrameType)
    {
        CODEC_LOGE("NULL==o_pbFrameData ||NULL==o_iFrameRate err \r\n");
        return iRet;
    }
    i_ptAVFrame->pts = i_ptAVFrame->best_effort_timestamp;
    i_ptAVFrame->pts = av_rescale_q(i_ptAVFrame->pts, {1, 1000}, m_ptCodecContext->time_base);// 时间戳转换

    i_ptAVFrame->pict_type = AV_PICTURE_TYPE_NONE;//
    iRet = avcodec_send_frame(m_ptCodecContext, i_ptAVFrame);
    if (iRet < 0) 
    {
        CODEC_LOGE("Error sending a frame for encoding\n");
        return iRet;
    }
    while (iRet >= 0) 
    {
        iRet = avcodec_receive_packet(m_ptCodecContext, m_ptPacket);
        if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF)
        {
            iRet=0;
            break;
        }
        else if (iRet < 0) 
        {
            CODEC_LOGE("avcodec_receive_packet err \r\n");
            av_packet_unref(m_ptPacket);
            return iRet;
        }
        CODEC_LOGD("enc packet %lld,size %d, data%p \r\n",m_ptPacket->pts, m_ptPacket->size, m_ptPacket->data, m_ptPacket->size);
        //av_packet_unref(m_ptPacket);
    }
    if(m_ptPacket->size>i_dwFrameMaxLen)
    {
        CODEC_LOGE("m_ptPacket->size%d>i_dwFrameMaxLen%d err \r\n",m_ptPacket->size,i_dwFrameMaxLen);
        av_packet_unref(m_ptPacket);
        return -1;
    }
    if (m_ptPacket->size > 0 && AV_PKT_FLAG_CORRUPT != m_ptPacket->flags) 
    {
        *o_iFrameType=m_ptPacket->flags == AV_PKT_FLAG_KEY ? CODEC_FRAME_TYPE_VIDEO_I_FRAME : CODEC_FRAME_TYPE_VIDEO_P_FRAME;
        if (m_ptCodecContext->framerate.den > 0)
        {
            iFrameRate = m_ptCodecContext->framerate.num / m_ptCodecContext->framerate.den;
        }
        if (iFrameRate > 0)
        {
            *o_iFrameRate = iFrameRate;//时间戳外层可以根据帧率计算 m_ptPacket->pts
        }
        memcpy(o_pbFrameData,m_ptPacket->data,m_ptPacket->size);
        iRet=m_ptPacket->size;
    }
    av_packet_unref(m_ptPacket);
    return iRet;
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
int VideoEncode::CodecTypeToAvCodecId(E_CodecType eCodecType)
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


