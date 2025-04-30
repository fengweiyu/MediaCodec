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
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

#define WATERMARK_FILTER_NAME "watermark"
#define OVERLAY_FILTER_NAME "overlay"
#define SCALE_FILTER_NAME "scale"


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
    m_ptFilterGraph = avfilter_graph_alloc();
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
-Fuction        : CreateWaterMarkFilter
-Description    : ����ˮӡ�㣨������ת��
color=size=iwxih:color=0x00000000,  # ����͸����������ԭ��Ƶͬ�ߴ磩  
 drawtext=text='��תˮӡ':fontfile=/path/to/font.ttf:fontsize=50:fontcolor=white@0.7:x=(W-tw)/2:y=(H-th)/2,  # ��͸�������Ͼ��л����ı�  
 rotate=0.5*PI:ow=iw:oh=ih,          # ��תˮӡ��0.5*PI=90�ȣ�  ����������������һ�£�ȷ��ˮӡ�ߴ�����Ƶƥ��
 format=rgba [wm];                    # ȷ�������͸��ͨ��  
 [0:v][wm] overlay=W-w-10:H-h-10      # ����ת���ˮӡ���ӵ���Ƶ���½� 
 [base];[base]���涺�ţ�ʹ�ø���ȷ�����ӷ�ʽ��ֹ����


"color=black@0:s='640x360',format=yuva420p[bg];[bg]trim=end_frame=1,
drawtext=text='yuweifeng08016':fontcolor=white@0.5:fontsize=18:x=32:y=18:fontfile=msyh.ttf, 
drawtext=text='yuweifeng08016':fontcolor=white@0.5:fontsize=18:x=472:y=273:fontfile=msyh.ttf,
rotate=-30*PI/180:c=black@0,trim=end_frame=1[rot];[0:v][rot]overlay=repeatlast=1[outV]"
���ַ�ʽ�������´���
if (avfilter_graph_create_filter(&buffer_src_ctx, buffersrc, "in", args, nullptr, filter_graph) < 0) {
    logger << "avfilter_graph_parse_ptr failed.";
    break;
}
if (avfilter_graph_parse_ptr(filter_graph, filter.c_str(), &inputs, &outputs, nullptr) < 0) {
    buffer_src_ctx = NULL;
    buffer_sink_ctx = NULL;
    logger << "avfilter_graph_parse_ptr failed.";
    break;
}
// 4. ������Դ���˾����� [0:v]
if (inputs) {
    int ret = avfilter_link(buffer_src_ctx, 0, inputs->filter_ctx, inputs->pad_idx);
    if (ret < 0) {
        logger << "avfilter_link inputs->filter_ctx failed.";
        break;
    }
}
// 5. ������� sink
if (avfilter_graph_create_filter(
        &buffer_sink_ctx, avfilter_get_by_name("buffersink"), "out", nullptr, nullptr, filter_graph) < 0) {
    logger << "avfilter_graph_parse_ptr failed.";
    break;
}
enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};
if (av_opt_set_int_list(buffer_sink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN) < 0) {
    logger << "avfilter_graph_parse_ptr failed.";
    break;
}
// 6. ������˵� ([outv] -> buffersink)
if (outputs) {
    int ret = avfilter_link(outputs->filter_ctx, outputs->pad_idx, buffer_sink_ctx, 0);
    if (ret < 0) {
        logger << "avfilter_link buffer_sink_ctx failed.";
        break;
    }
}









ˮӡ�ο�����:
color=black@0:s={{}}x{{}},format=yuva420p[wmover_{}]; \
[wmover_{}]trim=end_frame=1, \
drawtext={}={}:text='{}':fontcolor={}:alpha=0.8:fontsize={}:x={{}}:y={{}}, \
rotate={}*PI/180:c=black@0, \
loop=-1:1:0,tile=3x2,trim=end_frame=1[{}];",

-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoRawHandle::CreateWaterMarkFilter(int i_iWidth, int i_iHeight,const char * i_strText,const char * i_strFontFile,string *o_pstrFilterDescr)
{
    // ��̬������������С
    int iBaseFontSize = MAX(10, MIN(i_iHeight/20, 40-(strlen(i_strText) / 2)));

    // �߾����Ϊ��Сֵ������ˮӡ�ܶ�
    int iMarginX = i_iWidth / 20;  // ���ݿ�ȶ�̬�߾�
    int iMarginY = i_iHeight / 20; // ���ݸ߶ȶ�̬�߾�

    // ��̬����������
    int iApproxTextWidth = strlen(i_strText) * iBaseFontSize * 0.7; // �����ı����
    int iColCnt = MAX(1, (i_iWidth - iMarginX) / (iApproxTextWidth + iMarginX));
    int iRowCnt = 4;

    // ȷ��������Ƶ���㹻������
    if (i_iWidth < i_iHeight) 
    {
        iColCnt = MAX(1, i_iWidth / (iApproxTextWidth + iMarginX)); // ��������
        iRowCnt = 4;                                           // �ʵ���������
    }

    // ��ȷ����ˮӡ�ߴ�
    int iTextWidth = (i_iWidth - iMarginX * (iColCnt + 1)) / iColCnt;
    int iTextHeight = (i_iHeight - iMarginY * (iRowCnt + 1)) / iRowCnt;

    char strFilterDesc[1024];
    int iDescLen=0;
    iDescLen=snprintf(strFilterDesc,sizeof(strFilterDesc),"color=black@0:size=%dx%d,format=yuva420p[bg];[bg]trim=end_frame=1,",i_iWidth,i_iHeight);
    //iDescLen=snprintf(strFilterDesc,sizeof(strFilterDesc),"color=black@0:s=%dx%d[bg];[bg]",i_iWidth,i_iHeight);//trim=end_frame=1 ��ȡ��һ֡

    // ���ɽ���ˮӡ����
    for (int row = 0; row < iRowCnt; ++row) 
    {   
        for (int col = 0; col < iColCnt; ++col) 
        {
            // ����ƫ�����Ӹ�����
            int xOffset = (row % 2) ? iTextWidth / 2 : 0;

            int xPos = col * (iTextWidth + iMarginX) + iMarginX + xOffset;
            int yPos = row * (iTextHeight + iMarginY) + iMarginY;
            
            iDescLen+=snprintf(strFilterDesc+iDescLen,sizeof(strFilterDesc)-iDescLen,"drawtext=text='%s':fontcolor=white@0.7:fontsize=%d:x=%d:y=%d:fontfile=%s,",i_strText,iBaseFontSize,xPos,yPos,i_strFontFile);
        }
    }

    // �Ľ���ת��ƽ�̴���
    iDescLen+=snprintf(strFilterDesc+iDescLen,sizeof(strFilterDesc)-iDescLen,"rotate=-30*PI/180:c=black@0,trim=end_frame=1[%s];",WATERMARK_FILTER_NAME);//�����������������ں�������
    o_pstrFilterDescr->assign(strFilterDesc);
    //o_strFilterDescr->append("rotate=-30*PI/180:c=black@0,trim=end_frame=1[rot];[0:v][rot]overlay=repeatlast=1[outv]");// ����һ����ɫ��͸���ȣ������ظ�ƽ��

    //if (avfilter_graph_create_filter(o_pptFilter,avfilter_get_by_name("buffer"),WATERMARK_FILTER_NAME, o_pstrFilterDescr->c_str(), NULL, m_ptFilterGraph) < 0) 
    {//ʹ�����ַ�������AVFilterContext������avfilter_get_by_name�����û��name��û�������˾�����ᱨ��Ҫ����avfilter_graph_segment_xxx����   
        //CODEC_LOGE("avfilter_graph_create_filter wm err\n");//ͬʱ�÷���ֻ�ܴ���һ���˾����Ե�ǰ��strFilterDescr��Ҫ�����������������ÿ�ε�name�ֱ���color drawtext rotate �������˾�
        //return -1; //ʹ�� "buffer" �����˾��������ڶ�������Դ������ˮӡ�㣩
    } //"buffer -> color -> drawtext -> rotate"  // ˮӡ������ 
    return 0;
}

/*****************************************************************************
-Fuction        : CreateOverlayFilter
-Description    : ����overlay�˾�������ͼ�����
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoRawHandle::CreateScaleFilter(int i_iDstWidth, int i_iDstHeight,string *o_pstrFilterDescr)
{
    char strFilterDesc[1024];
    int iDescLen=0;
    iDescLen=snprintf(strFilterDesc,sizeof(strFilterDesc),"scale=w=%d:h=%d[%s];",i_iDstWidth,i_iDstHeight,SCALE_FILTER_NAME);
    o_pstrFilterDescr->assign(strFilterDesc);


    //if (avfilter_graph_create_filter(o_pptFilter,avfilter_get_by_name("overlay"),OVERLAY_FILTER_NAME , "x=W-w-10:y=H-h-10", NULL, m_ptFilterGraph) < 0) 
    {  
        //CODEC_LOGE("CreateOverlayFilter overlay err\n");
        //return -1;  
    } 
    return 0;
}

/*****************************************************************************
-Fuction        : Init
-Description    : 
����˾�֮������ӣ�
���������ַ�������","�Ż���"[];[]"��ʽ���������������ַ������﷨�ͱȽϸ��ӣ��������ж������ĸ��ط���
����һ�ַ�ʽ���ú���avfilter_link���ӣ��ṹ����Ҳ���ڶ�λ����
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
int VideoRawHandle::Init(AVCodecContext *i_ptDecodeCtx,string ** i_ppFilterDescrs,int i_iFiltersNum)
{
    int iRet = -1;
    //AVFilterGraph *filter_graph;
    //AVFilterContext *buffersrc_ctx;
    //AVFilterContext *buffersink_ctx;
    int i = 0;
    AVFilterContext *ptWaterMarkFilterCtx=NULL;
    AVFilterContext *ptOverlayFilterCtx=NULL;
    AVFilterContext *ptLastFilterCtx=NULL;
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = NULL;
    AVFilterInOut *inputs = NULL;
    AVFilterGraphSegment * seg = NULL;
    if (!m_ptFilterGraph) 
    {
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
        
        ptLastFilterCtx=m_ptBufferSrcCtx;
        for(i=0;i<i_iFiltersNum;i++)
        {
            iRet = avfilter_graph_segment_parse(m_ptFilterGraph, i_ppFilterDescrs[i]->c_str(), 0, &seg);
            
            iRet = avfilter_graph_segment_apply(seg, 0, &inputs, &outputs);//�˾����������͵�����������ֶ���������
            
            avfilter_graph_segment_free(&seg);
            seg = NULL;
            
            if (inputs) 
            {
                CODEC_LOGI("inputs->name %s \n",inputs->name);
                iRet = avfilter_link(ptLastFilterCtx, 0, inputs->filter_ctx, 0);
                avfilter_inout_free(&inputs);
            }
            
            if (outputs && outputs->name && 0 == strcmp(outputs->name, WATERMARK_FILTER_NAME)) 
            {
                CODEC_LOGI("WATERMARK_FILTER_NAME outputs->name %s \n",outputs->name);
                if (avfilter_graph_create_filter(&ptOverlayFilterCtx,avfilter_get_by_name("overlay"),OVERLAY_FILTER_NAME , "repeatlast=1", NULL, m_ptFilterGraph) < 0) 
                {  
                    CODEC_LOGE("avfilter_graph_create_filter overlay err\n");
                    iRet= -1;  
                    break;
                } 
                iRet = avfilter_link(ptLastFilterCtx, 0, ptOverlayFilterCtx, 0);// 5. �����˾� (���� -> overlay)  
                //ˮӡ���ӵ���Ƶ��
                iRet = avfilter_link(outputs->filter_ctx, 0, ptOverlayFilterCtx, 1);// �����˾� (ˮӡ -> overlay)  
            
                ptLastFilterCtx = ptOverlayFilterCtx;
            } 
            else 
            {
                CODEC_LOGI("outputs->name %s \n",outputs->name);
                ptLastFilterCtx = outputs->filter_ctx;
            }
            if (outputs) 
            {
                avfilter_inout_free(&outputs);
            }
        }
        if (iRet < 0) 
        {
            CODEC_LOGE("avfilter_link err\n");
            break;
        }
        if (avfilter_link(ptLastFilterCtx, 0, m_ptBufferSinkCtx, 0) < 0) 
        {  
            CODEC_LOGE("avfilter_link ptLastFilterCtx m_ptBufferSinkCtx err\n");
            iRet= -1;  
            break;
        }  
        iRet=avfilter_graph_config(m_ptFilterGraph, NULL);
        if (iRet < 0) 
        {
            CODEC_LOGE("avfilter_graph_config err\n");
            break;
        }
        iRet=0;
    }while(0);
    

    m_ptFiltFrame = av_frame_alloc();
    if(NULL==m_ptFiltFrame)
    {
        CODEC_LOGE("NULL==m_ptFrame err \r\n");
        return iRet;
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : Init
-Description    : 
����˾�֮������ӣ�
���������ַ�������","�Ż���"[];[]"��ʽ���������������ַ������﷨�ͱȽϸ��ӣ��������ж������ĸ��ط���
����һ�ַ�ʽ���ú���avfilter_link���ӣ��ṹ����Ҳ���ڶ�λ����
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
-Description    : һ֡ת��֡��֡���任��ʹ��framerate�˾���av_buffersink_get_frame����
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

    //m_ptAVFrame->pts = m_ptAVFrame->best_effort_timestamp;//���ñ�־������ʱ��FFmpeg�᳢��ʹ����ӽ���ʱ�������ʾÿ������֡��ʱ������������ܽӽ�ԭʼý���е�ʱ���
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
            CODEC_LOGD("FiltAVERROR_EOF,frame->pts%lld, data[0]%x, linesize[0]%d,fwidth%d,height%d \r\n", 
            m_ptFiltFrame->pts, m_ptFiltFrame->data[0], m_ptFiltFrame->linesize[0],m_ptFiltFrame->width, m_ptFiltFrame->height);
            break;
        }
        if (iRet < 0)
        {
            break;
        }
        //���ܶ�֡m_ptFiltFrame
        //display_frame(m_ptFiltFrame, m_ptBufferSinkCtx->inputs[0]->time_base);
        //av_frame_unref(m_ptFiltFrame);

        //CODEC_LOGD("FiltFrame ,frame->data[0]%x, linesize[0]%d,fwidth%d,height%d \r\n", m_ptFiltFrame->data[0], m_ptFiltFrame->linesize[0],m_ptFiltFrame->width, m_ptFiltFrame->height);
    }
    //av_frame_move_ref(o_ptAVFrame,m_ptFiltFrame);//��������Ĳ��������Ż�Ϊ��һ��
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




