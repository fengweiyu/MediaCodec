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
    AVCodec * ptCodec;//编码器，使用函数avcodec_find_decoder或者，该函数需要的id参数，来自于ptCodecContext中的codec_id成员
    int iCodecID=AV_CODEC_ID_NONE;
    
    //注册编解码器对象
    //avcodec_register_all(); 
    
    m_ptPacket = av_packet_alloc();
    if(NULL==m_ptPacket)
    {
        CODEC_LOGE("NULL==m_pPacket err \r\n");
        return iRet;
    }
    iCodecID=CodecTypeToAvCodecId(i_eCodecType);
    ptCodec = (AVCodec *)avcodec_find_decoder((enum AVCodecID)iCodecID);//查找解码器
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
    m_ptCodecContext->time_base = { 1, 1000 };//设置时间(基)以ms为单位
    //m_ptCodecContext->skip_frame = AVDISCARD_NONREF; // 跳过非参考帧
    //错误检测,丢掉有问题的帧, 防止一两个有问题的帧导致后面帧的编解码一直有问题(部分绿屏)
    //m_ptCodecContext->err_recognition = AV_EF_CRCCHECK | AV_EF_BITSTREAM | AV_EF_EXPLODE | AV_EF_COMPLIANT;
    m_ptCodecContext->err_recognition |= (AV_EF_CRCCHECK | AV_EF_EXPLODE | AV_EF_BITSTREAM | AV_EF_BUFFER | AV_EF_CAREFUL | AV_EF_COMPLIANT | AV_EF_AGGRESSIVE);
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
    // 设置常规参数，比如：  
    m_ptCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;  // 或其他支持的软件像素格式  
    
    //if (ptCodec->capabilities & AV_CODEC_CAP_TRUNCATED) 
    {
        //m_ptCodecContext->flags |= AV_CODEC_FLAG_TRUNCATED;
    }
    
    AVDictionary * ptOpts = NULL;
    av_dict_set(&ptOpts, "preset", "ultrafast", 0);
    av_dict_set(&ptOpts, "tune", "stillimage,fastdecode,zerolatency", 0);
    //av_dict_set_int(&ptOpts, "ignore_poc", 1, 0);// 忽略poc的影响，默认设备没有B帧
    //if (avcodec_open2(m_ptCodecContext, ptCodec, &ptOpts) < 0)
    if (avcodec_open2(m_ptCodecContext, ptCodec, NULL)<0)
    {//打开解码器
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
-Input          : i_ddwPTS i_ddwDTS -1表示时间戳无效，无时间戳,等同AV_NOPTS_VALUE
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
    if(NULL==i_pbFrameData ||NULL==o_ptAVFrame ||0==i_dwFrameLen)
    {
        CODEC_LOGE("NULL==i_pbFrameData ||NULL==o_ptAVFrame||0==i_dwFrameLen err \r\n");
        return iRet;
    }
    pbFrameData=i_pbFrameData;
    dwFrameLen=i_dwFrameLen;
    if(i_ddwPTS >= 0)
        ddwPTS=i_ddwPTS;//ms
    if(i_ddwDTS >= 0)
        ddwDTS=i_ddwDTS;//ms

    while (dwFrameLen > 0) 
    {
        av_packet_unref(m_ptPacket);//av_init_packet(m_ptPacket);//attribute_deprecated
        m_ptPacket->data = pbFrameData;
        m_ptPacket->size = dwFrameLen;
        m_ptPacket->pts = ddwPTS;
        m_ptPacket->dts = ddwDTS;
        pbFrameData += m_ptPacket->size;
        dwFrameLen -= m_ptPacket->size;
            
        /*//初始化AVPacket对象
        av_init_packet(m_ptPacket);
        m_ptParser->flags |=PARSER_FLAG_COMPLETE_FRAMES;//等同上面手动赋值
        iRet = av_parser_parse2(m_ptParser,m_ptCodecContext, &m_ptPacket->data, &m_ptPacket->size,pbFrameData, dwFrameLen, ddwPTS, ddwDTS, 0);
        if (iRet < 0) 
        {
            CODEC_LOGE("av_parser_parse2 err %d\r\n",iRet);
            return iRet;
        }
        pbFrameData += iRet;
        dwFrameLen -= iRet;*/
        if (m_ptPacket->size<=0)
        {
            CODEC_LOGE("av_parser_parse2 err m_ptPacket->size<=0\r\n");
            return -1;
        }

        iRet = avcodec_send_packet(m_ptCodecContext, m_ptPacket);
        if (iRet < 0) 
        {
            CODEC_LOGE("avcodec_send_packet err \r\n");
            return iRet;
        }
        while (iRet >= 0) 
        {
            //av_frame_unref(m_ptFrame);//可能输出多帧
            iRet = avcodec_receive_frame(m_ptCodecContext, m_ptFrame);//不av_frame_unref内部也会初始化
            if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF)
            {
                iRet=0;
                //CODEC_LOGD("iRet %d,,frame->data[0]%d, frame->linesize[0]%d,frame->width%d, frame->height%d \r\n", iRet,m_ptFrame->data[0], m_ptFrame->linesize[0],m_ptFrame->width, m_ptFrame->height);
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
            CODEC_LOGD("dec%d frame->linesize[0]%d,pts %lld,ddwPTS,ddwDTS%lld,%lld, data[0]%x,width%d, height%d \r\n",m_ptPacket->size,m_ptFrame->linesize[0],m_ptFrame->pts,ddwPTS,ddwDTS,
            /*m_ptCodecContext->frame_number,*/m_ptFrame->data[0],m_ptFrame->width, m_ptFrame->height);

            if(o_ptAVFrame->linesize[0]>0)
            {//暂不支持多帧取出，后续o_ptAVFrame优化为数组或者list
                iRet=0;
                CODEC_LOGW("already save frame,frame%d->linesize%d,width%d, height%d\r\n",m_ptFrame->data[0], m_ptFrame->linesize[0],m_ptFrame->width, m_ptFrame->height);
                break;
            }
            //av_frame_move_ref(o_ptAVFrame,m_ptFrame);//后续下面的操作可以优化为这一个
            iRet=av_frame_ref(o_ptAVFrame,m_ptFrame);
            if (iRet < 0)
            {
                CODEC_LOGE("Decode av_frame_ref err %d\r\n",iRet);
                av_frame_unref(m_ptFrame);
                return iRet;
            }
            iRet=av_frame_copy(o_ptAVFrame,m_ptFrame);
            if (iRet < 0)
            {
                CODEC_LOGE("Decode av_frame_copy err %d \r\n",iRet);
                av_frame_unref(o_ptAVFrame);
            }
            av_frame_unref(m_ptFrame);
        }
    }
    /*
    //av_frame_move_ref(o_ptAVFrame,m_ptFrame);//后续下面的操作可以优化为这一个
    iRet=av_frame_ref(o_ptAVFrame,m_ptFrame);
    if (iRet < 0)
    {
        CODEC_LOGE("Decode av_frame_ref err %d\r\n",iRet);
        av_frame_unref(m_ptFrame);
        return iRet;
    }
    iRet=av_frame_copy(o_ptAVFrame,m_ptFrame);
    if (iRet < 0)
    {
        CODEC_LOGE("Decode av_frame_copy err %d \r\n",iRet);
        av_frame_unref(o_ptAVFrame);
    }
    av_frame_unref(m_ptFrame);*/
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

