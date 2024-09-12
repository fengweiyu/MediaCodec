/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       AudioDecode.cpp
* Description           : 	    
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "AudioDecode.h"
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
-Fuction        : AudioDecode
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
AudioDecode::AudioDecode()
{
    m_ptPacket=NULL;
    m_ptParser = NULL;
    m_ptCodecContext = NULL;
    m_ptFrame = NULL;
}
/*****************************************************************************
-Fuction        : ~~AudioDecode
-Description    : ~~AudioDecode
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
AudioDecode::~AudioDecode()
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
int AudioDecode::Init(E_CodecType i_eCodecType,int i_iSampleRate,int i_iChannels)
{
    int iRet = -1;
    AVCodec *ptCodec;//编码器，使用函数avcodec_find_decoder或者，该函数需要的id参数，来自于ptCodecContext中的codec_id成员
    int iCodecID=AV_CODEC_ID_NONE;
    
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
        CODEC_LOGE("AudioDecode::Init NULL==ptCodec err %d\r\n",i_eCodecType);
        return iRet;
    }
    m_ptParser = av_parser_init(ptCodec->id);
    if(NULL==m_ptParser)
    {
        CODEC_LOGE("AudioDecode::Init NULL==m_ptParser %#x \r\n",ptCodec->id);
        //return iRet;//g711a不支持av_parser_init，暂时也不用m_ptParser
    }
    m_ptCodecContext = avcodec_alloc_context3(ptCodec);
    if(NULL==m_ptCodecContext)
    {
        CODEC_LOGE("AudioDecode::Init NULL==m_ptCodecContext err \r\n");
        return iRet;
    }
    // Decoder Setting
    m_ptCodecContext->sample_rate = i_iSampleRate;
    //m_ptCodecContext->channels = i_iChannels;//新版本已经删除
    //m_ptCodecContext->ch_layout.nb_channels = i_iChannels;//新版本
    iRet = SelectChannelLayout((const AVCodec *)ptCodec, &m_ptCodecContext->ch_layout,i_iChannels);
    if (iRet < 0)
    {
        CODEC_LOGE("AudioDecode::Init SelectChannelLayout err \r\n");
        return iRet;
    }

    if (avcodec_open2(m_ptCodecContext, ptCodec, NULL)<0)
    {//打开解码器
        CODEC_LOGE("AudioDecode::Init avcodec_open2 err %s \r\n",avcodec_get_name((enum AVCodecID)iCodecID));
        return iRet;
    }
    
    int iSrcBitsSample = av_get_bits_per_sample(m_ptCodecContext->codec_id);//源每个样本(帧)多少位
    int iSrcBytesSample= iSrcBitsSample * m_ptCodecContext->ch_layout.nb_channels / 8;//iSrcBitsSample * m_ptCodecContext->channels / 8;//源每个样本(帧)多少字节(已计算通道数)

    m_ptFrame = av_frame_alloc();
    if(NULL==m_ptFrame)
    {
        CODEC_LOGE("AudioDecode::Init NULL==m_ptFrame err \r\n");
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
int AudioDecode::Decode(unsigned char * i_pbFrameData,unsigned int  i_dwFrameLen,int64_t i_ddwPTS,int64_t i_ddwDTS,AVFrame *o_ptAVFrame)
{
    int iRet = -1;
    unsigned char *pbFrameData=NULL;
    unsigned int dwFrameLen=0;
    int64_t ddwPTS=AV_NOPTS_VALUE;//ms
    int64_t ddwDTS=AV_NOPTS_VALUE;//ms


    if(NULL==m_ptFrame)
    {
        CODEC_LOGE("AudioDecode NULL==m_ptFrame Decode err \r\n");
        return iRet;
    }
    if(NULL==i_pbFrameData ||NULL==o_ptAVFrame)
    {
        CODEC_LOGE("AudioDecode NULL==i_pbFrameData ||NULL==o_ptAVFrame err \r\n");
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
        av_init_packet(m_ptPacket);
        m_ptPacket->data = pbFrameData;
        m_ptPacket->size = dwFrameLen;
        m_ptPacket->pts = ddwPTS;
        m_ptPacket->dts = ddwDTS;
        pbFrameData += m_ptPacket->size;
        dwFrameLen -= m_ptPacket->size;
                
        /*//初始化AVPacket对象，av_parser_parse2会导致解码出的frame pts不会被赋值
        av_init_packet(m_ptPacket);
        iRet = av_parser_parse2(m_ptParser,m_ptCodecContext, &m_ptPacket->data, &m_ptPacket->size,pbFrameData, dwFrameLen, ddwPTS, ddwDTS, 0);
        if (iRet < 0) 
        {
            CODEC_LOGE("AudioDecode av_parser_parse2 err \r\n");
            return iRet;
        }
        pbFrameData += iRet;
        dwFrameLen -= iRet;*/
        
        if (m_ptPacket->size<=0)
        {
            CODEC_LOGE("AudioDecode av_parser_parse2 err m_ptPacket->size<=0\r\n");
            return iRet;
        }

        iRet = avcodec_send_packet(m_ptCodecContext, m_ptPacket);
        if (iRet < 0) 
        {
            CODEC_LOGE("AudioDecode avcodec_send_packet err \r\n");
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
                CODEC_LOGE("AudioDecode avcodec_receive_frame err \r\n");
                av_frame_unref(m_ptFrame);
                return iRet;
            }
            /* the picture is allocated by the decoder. no need to
               free it */
            CODEC_LOGD("AudioDecode pts %lld,nb_samples %d frame->linesize[0]%d,data[0]%x,width%d, height%d \r\n", m_ptFrame->pts,m_ptFrame->nb_samples,
            m_ptFrame->linesize[0],m_ptFrame->data[0],m_ptFrame->width, m_ptFrame->height);
            /*data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);//ffmepg4.2.1取数据示例
            if (data_size < 0) {
                //This should not occur, checking just for paranoia
                fprintf(stderr, "Failed to calculate data size\n");
                exit(1);
            }
            for (i = 0; i < frame->nb_samples; i++)
                for (ch = 0; ch < dec_ctx->channels; ch++)
                    fwrite(frame->data[ch] + data_size*i, 1, data_size, outfile);*/
            if(o_ptAVFrame->nb_samples>0)//nb_samples采样点数量>0
            {//暂不支持多帧取出，后续o_ptAVFrame优化为数组或者list
                iRet=0;
                CODEC_LOGW("AudioDecode already save frame,frame%d->nb_samples%d,width%d, height%d\r\n",m_ptFrame->data[0], m_ptFrame->nb_samples,m_ptFrame->width, m_ptFrame->height);
                break;
            }
            //av_frame_move_ref(o_ptAVFrame,m_ptFrame);//后续下面的操作可以优化为这一个
            iRet=av_frame_ref(o_ptAVFrame,m_ptFrame);
            if (iRet < 0)
            {
                CODEC_LOGE("AudioDecode av_frame_ref err %d\r\n",iRet);
                av_frame_unref(m_ptFrame);
                return iRet;
            }
            iRet=av_frame_copy(o_ptAVFrame,m_ptFrame);
            if (iRet < 0)
            {
                CODEC_LOGE("AudioDecode av_frame_copy err %d \r\n",iRet);
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
int AudioDecode::GetCodecContext(AVCodecContext **o_ptCodecContext)
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
int AudioDecode::CodecTypeToAvCodecId(E_CodecType eCodecType)
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

/*****************************************************************************
-Fuction        : SelectChannelLayout
-Description    : 
select layout with the highest channel count 
from ffmpeg select_channel_layout
-Input          : i_iChannels 为0则自己推导
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2023/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioDecode::SelectChannelLayout(const AVCodec *codec, AVChannelLayout *dst,int i_iChannels)
{
    const AVChannelLayout *p=NULL, *best_ch_layout=NULL;
    int best_nb_channels = 0;
    AVChannelLayout ch1_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_MONO;//首选单声道
    AVChannelLayout ch2_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;//双声道
    AVChannelLayout ch3_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_2POINT1;//三声道

    switch(i_iChannels)
    {
        case 1:
        {
            best_ch_layout=&ch1_layout;
            break;
        }
        case 2:
        {
            best_ch_layout=&ch2_layout;
            break;
        }
        case 3:
        {
            best_ch_layout=&ch3_layout;
            break;
        }
        default:
        {
            CODEC_LOGW("AudioDecode i_iChannels err %d,use default ch1_layout\r\n",i_iChannels);
            best_ch_layout=&ch1_layout;
            break;
        }
    }
    
    if (!codec->ch_layouts)
    {
        CODEC_LOGW("AudioDecode !codec->ch_layouts ,use best_ch_layout%d i_iChannels%d\r\n",best_ch_layout->nb_channels,i_iChannels);
        return av_channel_layout_copy(dst, best_ch_layout);
    }

    p = codec->ch_layouts;
    while (p->nb_channels) 
    {
        int nb_channels = p->nb_channels;

        if (i_iChannels>0 && nb_channels == i_iChannels) 
        {
            best_ch_layout = p;
            best_nb_channels = nb_channels;
            break;
        }
        else if(nb_channels > best_nb_channels) 
        {
            best_ch_layout = p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return av_channel_layout_copy(dst, best_ch_layout);
}

