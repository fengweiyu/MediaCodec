/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       AudioEncode.cpp
* Description           : 	   
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "AudioEncode.h"
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
-Fuction        : AudioEncode
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
AudioEncode::AudioEncode()
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
AudioEncode::~AudioEncode()
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
int AudioEncode::Init(E_CodecType i_eCodecType,int i_iSampleRate,int i_iChannels)
{
    int iRet = -1;
    AVCodec  *ptCodec;//编码器，使用函数avcodec_find_decoder或者，该函数需要的id参数，来自于ptCodecContext中的codec_id成员
    int iCodecID=AV_CODEC_ID_NONE;
    
    /* Find the encoder to be used by its name. */
    iCodecID=CodecTypeToAvCodecId(i_eCodecType);
    ptCodec = (AVCodec *)avcodec_find_encoder((enum AVCodecID)iCodecID);//查找解码器
    if(NULL==ptCodec)
    {
        CODEC_LOGE("AudioEncode::Init NULL==ptCodec err iCodecID%d i_eCodecType%d\r\n",iCodecID,i_eCodecType);
        return iRet;
    }
    m_ptCodecContext = avcodec_alloc_context3(ptCodec);
    if(NULL==m_ptCodecContext)
    {
        CODEC_LOGE("AudioEncode::Init NULL==m_ptCodecContext err \r\n");
        return iRet;
    }
    // Encoder Setting
    /* Set the basic encoder parameters.
     * The input file's sample rate is used to avoid a sample rate conversion.
     */
    //m_ptCodecContext->channel_layout = av_get_default_channel_layout(i_iChannels);
    //m_ptCodecContext->channels = i_iChannels;//新版本已经删除
    //m_ptCodecContext->ch_layout.nb_channels = i_iChannels;//新版本
    iRet = SelectChannelLayout((const AVCodec *)ptCodec, &m_ptCodecContext->ch_layout,i_iChannels);
    if (iRet < 0)
    {
        CODEC_LOGE("AudioEncode::Init SelectChannelLayout err \r\n");
        return iRet;
    }
    m_ptCodecContext->sample_rate = i_iSampleRate;
    if (ptCodec->sample_fmts)
    {
        m_ptCodecContext->sample_fmt = ptCodec->sample_fmts[0];
    }
    m_ptCodecContext->time_base = { 1, 1000 };
    // m_ptCodecContext->bit_rate = bit_rate;

    /* Open the encoder for the audio stream to use it later. */
    if ((iRet = avcodec_open2(m_ptCodecContext, ptCodec, NULL)) < 0) 
    {
        CODEC_LOGE("AudioEncode::Init Could not open codec\n");
        //avcodec_free_context(&m_ptCodecContext);//析构会统一释放
        return iRet;
    }

    
    m_ptPacket = av_packet_alloc();
    if(NULL==m_ptPacket)
    {
        CODEC_LOGE("AudioEncode::Init NULL==m_pPacket err \r\n");
        return iRet;
    }
    CODEC_LOGD("AudioEncode::Init m_ptCodecContext->ch_layout.nb_channels%d,i_iChannels %d \r\n",m_ptCodecContext->ch_layout.nb_channels,i_iChannels);
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
int AudioEncode::Encode(AVFrame *i_ptAVFrame,unsigned char * o_pbFrameData,unsigned int i_dwFrameMaxLen,int64_t *o_ddwPTS)
{
    int iRet = -1;
    int iFrameRate = 0;
    int iFrameLen = 0;


    if(NULL==m_ptPacket)
    {
        CODEC_LOGE("NULL==m_ptPacket AudioEncode err \r\n");
        return iRet;
    }
    if(NULL==i_ptAVFrame ||NULL==o_pbFrameData ||NULL== o_ddwPTS)
    {
        CODEC_LOGE("AudioEncode NULL==o_pbFrameData ||NULL==o_ddwPTS err \r\n");
        return iRet;
    }
    
    //i_ptAVFrame->pts = i_ptAVFrame->best_effort_timestamp;//重采样处理内部已经赋值过
    iRet = avcodec_send_frame(m_ptCodecContext, i_ptAVFrame);
    if (iRet < 0) 
    {
        CODEC_LOGE("AudioEncode Error sending a frame for encoding\n");
        return iRet;
    }
    while (iRet >= 0) 
    {
        iRet = avcodec_receive_packet(m_ptCodecContext, m_ptPacket);
        if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF)
        {
            iRet=iFrameLen;
            //CODEC_LOGD("m_ptPacket %lld,size %d, data%p ,iRet == AVERROR_EOF%d\r\n",m_ptPacket->pts, m_ptPacket->size, m_ptPacket->data,iRet == AVERROR_EOF?1:0);
            break;
        }
        else if (iRet < 0) 
        {
            CODEC_LOGE("AudioEncode avcodec_receive_packet err \r\n");
            av_packet_unref(m_ptPacket);
            return iRet;
        }
        CODEC_LOGD("AudioEncode enc packet %lld,size %d, data%p \r\n",m_ptPacket->pts, m_ptPacket->size, m_ptPacket->data, m_ptPacket->size);
        //av_packet_unref(m_ptPacket);
        if(iFrameLen>0)
        {//暂不支持多帧取出，后续优化为数组或者list
            CODEC_LOGW("AudioEncode already save frame,iFrameLen%d\r\n",iFrameLen);
        }
        if(m_ptPacket->size <= 0 ||m_ptPacket->size>i_dwFrameMaxLen-iFrameLen)
        {
            CODEC_LOGE("AudioEncode m_ptPacket->size%d>i_dwFrameMaxLen%d -iFrameLen%d err \r\n",m_ptPacket->size,i_dwFrameMaxLen,iFrameLen);
            av_packet_unref(m_ptPacket);
            return -1;
        }
        m_ptPacket->pts = i_ptAVFrame->pts;
        if (AV_CODEC_ID_AAC==m_ptCodecContext->codec_id) 
        {
            unsigned char bDataBuf[7];
            memset(bDataBuf,0,sizeof(bDataBuf));
            iRet=GenerateAdtsHeader(bDataBuf,7,m_ptCodecContext->sample_rate,m_ptCodecContext->ch_layout.nb_channels, m_ptPacket->size);
            if(iRet>0)
            {
                memcpy(o_pbFrameData+iFrameLen,bDataBuf,iRet);
                iFrameLen+=iRet;
            }
        }
        memcpy(o_pbFrameData+iFrameLen,m_ptPacket->data,m_ptPacket->size);
        iFrameLen+=m_ptPacket->size;
        *o_ddwPTS=m_ptPacket->pts;
        iRet=iFrameLen;
        
        av_packet_unref(m_ptPacket);
    }
    /*if(m_ptPacket->size>i_dwFrameMaxLen)
    {
        CODEC_LOGE("m_ptPacket->size%d>i_dwFrameMaxLen%d err \r\n",m_ptPacket->size,i_dwFrameMaxLen);
        av_packet_unref(m_ptPacket);
        return -1;
    }
    if (m_ptPacket->size > 0 && AV_PKT_FLAG_CORRUPT != m_ptPacket->flags) 
    {
        *o_iFrameType=m_ptPacket->flags == AV_PKT_FLAG_KEY ? CODEC_FRAME_TYPE_Audio_I_FRAME : CODEC_FRAME_TYPE_Audio_P_FRAME;
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
    av_packet_unref(m_ptPacket);*/
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
int AudioEncode::GetCodecContext(AVCodecContext **o_ptCodecContext)
{
    int iRet = -1;

    if(NULL==m_ptPacket)
    {
        CODEC_LOGE("NULL==m_ptPacket GetCodecContext err \r\n");
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
int AudioEncode::CodecTypeToAvCodecId(E_CodecType eCodecType)
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
int AudioEncode::SelectChannelLayout(const AVCodec *codec, AVChannelLayout *dst,int i_iChannels)
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
            CODEC_LOGW("i_iChannels err%d,use default ch1_layout\r\n",i_iChannels);
            best_ch_layout=&ch1_layout;
            break;
        }
    }
    
    if (!codec->ch_layouts)
    {
        CODEC_LOGW("!codec->ch_layouts ,use best_ch_layout%d\r\n",best_ch_layout->nb_channels);
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


/*****************************************************************************
-Fuction        : GenerateAdtsHeader
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2023/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioEncode::GenerateAdtsHeader(unsigned char * o_pbData,int i_iMaxDataLen,int i_iSampleRate,int i_iChannels,int i_iPayloadLen) 
{
    int iRet = -1;
    unsigned char bDataBuf[7];
    int iLen = i_iPayloadLen + 7;

    if (NULL == o_pbData || i_iMaxDataLen < 7) 
    {
        CODEC_LOGE("NULL==o_pbData err%d \r\n",i_iMaxDataLen);
        return iRet;
    }

    int iSampleIndex = 0x04;
    switch (i_iSampleRate) 
    {
        case 96000:
            iSampleIndex = 0x00;
            break;
        case 88200:
            iSampleIndex = 0x01;
            break;
        case 64000:
            iSampleIndex = 0x02;
            break;
        case 48000:
            iSampleIndex = 0x03;
            break;
        case 44100:
            iSampleIndex = 0x04;
            break;
        case 32000:
            iSampleIndex = 0x05;
            break;
        case 24000:
            iSampleIndex = 0x06;
            break;
        case 22050:
            iSampleIndex = 0x07;
            break;
        case 16000:
            iSampleIndex = 0x08;
            break;
        case 12000:
            iSampleIndex = 0x09;
            break;
        case 11025:
            iSampleIndex = 0x0a;
            break;
        case 8000:
            iSampleIndex = 0x0b;
            break;
        case 7350:
            iSampleIndex = 0x0c;
            break;
    }

    bDataBuf[0] = 0xFF;
    bDataBuf[1] = 0xF1;
    bDataBuf[2] = 0x40 | (iSampleIndex << 2) | (i_iChannels >> 2);
    bDataBuf[3] = ((i_iChannels & 0x03) << 6) | (iLen >> 11);
    bDataBuf[4] = (iLen >> 3) & 0xFF;
    bDataBuf[5] = ((iLen << 5) & 0xFF) | 0x1F;
    bDataBuf[6] = 0xFC;

    memcpy(o_pbData,bDataBuf,sizeof(bDataBuf));
    return 7;
}

