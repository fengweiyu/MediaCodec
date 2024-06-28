/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       VideoRawHandle.cpp
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "VideoRawHandle.h"
#include "MediaTranscodeCom.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <functional>

//#include <sys/time.h>
//#include "cJSON.h"


/*****************************************************************************
-Fuction        : VideoRawHandle
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
VideoRawHandle::VideoRawHandle()
{
    m_ptFiltFrame = NULL;
    m_ptBufferSrcCtx = NULL;//
    m_ptBufferSinkCtx = NULL;//
    m_ptFilterGraph = NULL;//
}
/*****************************************************************************
-Fuction        : ~VideoRawHandle
-Description    : ~VideoRawHandle
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
VideoRawHandle::~VideoRawHandle()
{
    if(NULL!=m_ptFilterGraph)
    {
        avfilter_graph_free(&m_ptFilterGraph);
        m_ptFilterGraph=NULL;
    }
    if(NULL!=m_ptFiltFrame)
    {
        av_frame_free(&m_ptFiltFrame);
        m_ptFiltFrame=NULL;
    }
}
/*****************************************************************************
-Fuction        : Init
-Description    : FilterInit
char strTemp[512] = { 0 };
time_t sec = time(NULL);
snprintf(strTemp,sizeof(strTemp),"drawtext=fontfile=msyhbd.ttc:fontcolor=red:fontsize=25:x=50:y=20:text='%sn'",ToString(sec, strTemp1));
std::string filterStr(strTemp);
//if (!Init(filterStr))
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoRawHandle::Init(AVCodecContext *i_ptDecodeCtx,const char *i_strFiltersDescr)
{
    int iRet = -1;
    //AVFilterGraph *filter_graph;
    //AVFilterContext *buffersrc_ctx;
    //AVFilterContext *buffersink_ctx;

    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    m_ptFilterGraph = avfilter_graph_alloc();
    if (!outputs || !inputs || !m_ptFilterGraph) 
    {
        if (outputs) 
        {
            avfilter_inout_free(&outputs);
        }
        if (inputs) 
        {
            avfilter_inout_free(&inputs);
        }
        if (m_ptFilterGraph) 
        {
            avfilter_graph_free(&m_ptFilterGraph);
        }
        return iRet;
    }
    
    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    char args[512];
    snprintf(args, sizeof(args),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        i_ptDecodeCtx->width, i_ptDecodeCtx->height, i_ptDecodeCtx->pix_fmt,
        1, 1000,
        i_ptDecodeCtx->sample_aspect_ratio.num,
        i_ptDecodeCtx->sample_aspect_ratio.den);
    do
    {
        iRet = avfilter_graph_create_filter(&m_ptBufferSrcCtx, buffersrc, "in",args, NULL, m_ptFilterGraph);
        if (iRet < 0) 
        {
            CODEC_LOGE( "Cannot create buffer source\n");
            break;
        }
        /* buffer video sink: to terminate the filter chain. */
        iRet = avfilter_graph_create_filter(&m_ptBufferSinkCtx, buffersink, "out",NULL, NULL, m_ptFilterGraph);
        if (iRet < 0) 
        {
            CODEC_LOGE("Cannot create buffer sink\n");
            break;
        }
        enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
        iRet = av_opt_set_int_list(m_ptBufferSinkCtx, "pix_fmts", pix_fmts,AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
        if (iRet < 0) 
        {
            CODEC_LOGE("Cannot set output pixel format\n");
            break;
        }
        /*
         * Set the endpoints for the filter graph. The filter_graph will
         * be linked to the graph described by filters_descr.
         */
        /*
         * The buffer source output must be connected to the input pad of
         * the first filter described by filters_descr; since the first
         * filter input label is not specified, it is set to "in" by
         * default.
         */
        outputs->name = av_strdup("in");
        outputs->filter_ctx = m_ptBufferSrcCtx;
        outputs->pad_idx = 0;
        outputs->next = NULL;
        /*
         * The buffer sink input must be connected to the output pad of
         * the last filter described by filters_descr; since the last
         * filter output label is not specified, it is set to "out" by
         * default.
         */
        inputs->name = av_strdup("out");
        inputs->filter_ctx = m_ptBufferSinkCtx;
        inputs->pad_idx = 0;
        inputs->next = NULL;
        
        iRet=avfilter_graph_parse_ptr(m_ptFilterGraph,i_strFiltersDescr,&inputs,&outputs, NULL);
        if (iRet < 0) 
        {
            CODEC_LOGE("avfilter_graph_parse_ptr err %s \r\n",i_strFiltersDescr);
            break;
        }
        iRet=avfilter_graph_config(m_ptFilterGraph, NULL);
        if (iRet < 0) 
        {
            break;
        }
        iRet=0;
    }while(0);
    
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    m_ptFiltFrame = av_frame_alloc();
    if(NULL==m_ptFiltFrame)
    {
        CODEC_LOGE("NULL==m_ptFrame err \r\n");
        return iRet;
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : RawHandle
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoRawHandle::RawHandle(AVFrame *m_ptAVFrame)
{
    int iRet = -1;
    unsigned char *pbFrameData=NULL;
    unsigned int dwFrameLen=0;
    int64_t ddwPTS=AV_NOPTS_VALUE;//ms
    int64_t ddwDTS=AV_NOPTS_VALUE;//ms

    if(NULL==m_ptAVFrame)
    {
        CODEC_LOGE("NULL==i_ptAVFrame RawHandle err \r\n");
        return iRet;
    }

    //m_ptAVFrame->pts = m_ptAVFrame->best_effort_timestamp;//当该标志被设置时，FFmpeg会尝试使用最接近的时间戳来表示每个解码帧的时间戳，即尽可能接近原始媒体中的时间戳
    /* push the decoded frame into the filtergraph */
    if (av_buffersrc_add_frame_flags(m_ptBufferSrcCtx, m_ptAVFrame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) 
    {
        CODEC_LOGE("Error while feeding the filtergraph\n");
        return iRet;
    }
    /* pull filtered frames from the filtergraph */
    while (1) 
    {
        iRet = av_buffersink_get_frame(m_ptBufferSinkCtx, m_ptFiltFrame);
        if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF)
        {
            iRet=0;
            CODEC_LOGD("FiltAVERROR_EOF,frame->data[0]%x, linesize[0]%d,fwidth%d,height%d \r\n", m_ptFiltFrame->data[0], m_ptFiltFrame->linesize[0],m_ptFiltFrame->width, m_ptFiltFrame->height);
            break;
        }
        if (iRet < 0)
        {
            break;
        }
        //可能多帧m_ptFiltFrame
        //display_frame(m_ptFiltFrame, m_ptBufferSinkCtx->inputs[0]->time_base);
        //av_frame_unref(m_ptFiltFrame);

        //CODEC_LOGD("FiltFrame ,frame->data[0]%x, linesize[0]%d,fwidth%d,height%d \r\n", m_ptFiltFrame->data[0], m_ptFiltFrame->linesize[0],m_ptFiltFrame->width, m_ptFiltFrame->height);
    }
    //av_frame_move_ref(o_ptAVFrame,m_ptFiltFrame);//后续下面的操作可以优化为这一个
    av_frame_unref(m_ptAVFrame);
    iRet=av_frame_ref(m_ptAVFrame,m_ptFiltFrame);
    if (iRet < 0)
    {
        CODEC_LOGE("av_frame_ref err \r\n");
        av_frame_unref(m_ptFiltFrame);
        return iRet;
    }
    iRet=av_frame_copy(m_ptAVFrame,m_ptFiltFrame);
    if (iRet < 0)
    {
        CODEC_LOGE("av_frame_copy err \r\n");
        av_frame_unref(m_ptAVFrame);
    }
    av_frame_unref(m_ptFiltFrame);
    
    return iRet;
}




