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
-Description    : 创建水印层（包含旋转）
color=size=iwxih:color=0x00000000,  # 创建透明背景（与原视频同尺寸）  
 drawtext=text='旋转水印':fontfile=/path/to/font.ttf:fontsize=50:fontcolor=white@0.7:x=(W-tw)/2:y=(H-th)/2,  # 在透明背景上居中绘制文本  
 rotate=0.5*PI:ow=iw:oh=ih,          # 旋转水印（0.5*PI=90度）  设置输出宽高与输入一致，确保水印尺寸与视频匹配
 format=rgba [wm];                    # 确保输出带透明通道  
 [0:v][wm] overlay=W-w-10:H-h-10      # 将旋转后的水印叠加到视频右下角 
 [base];[base]代替逗号，使用更明确的连接方式防止出错


"color=black@0:s='640x360',format=yuva420p[bg];[bg]trim=end_frame=1,
drawtext=text='yuweifeng08016':fontcolor=white@0.5:fontsize=18:x=32:y=18:fontfile=msyh.ttf, 
drawtext=text='yuweifeng08016':fontcolor=white@0.5:fontsize=18:x=472:y=273:fontfile=msyh.ttf,
rotate=-30*PI/180:c=black@0,trim=end_frame=1[rot];[0:v][rot]overlay=repeatlast=1[outV]"
这种方式依赖如下代码
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
// 4. 绑定输入源到滤镜链的 [0:v]
if (inputs) {
    int ret = avfilter_link(buffer_src_ctx, 0, inputs->filter_ctx, inputs->pad_idx);
    if (ret < 0) {
        logger << "avfilter_link inputs->filter_ctx failed.";
        break;
    }
}
// 5. 创建输出 sink
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
// 6. 绑定输出端点 ([outv] -> buffersink)
if (outputs) {
    int ret = avfilter_link(outputs->filter_ctx, outputs->pad_idx, buffer_sink_ctx, 0);
    if (ret < 0) {
        logger << "avfilter_link buffer_sink_ctx failed.";
        break;
    }
}









水印参考代码:
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
    // 动态计算基础字体大小
    int iBaseFontSize = MAX(10, MIN(i_iHeight/20, 40-(strlen(i_strText) / 2)));

    // 边距调整为更小值以增加水印密度
    int iMarginX = i_iWidth / 20;  // 根据宽度动态边距
    int iMarginY = i_iHeight / 20; // 根据高度动态边距

    // 动态计算行列数
    int iApproxTextWidth = strlen(i_strText) * iBaseFontSize * 0.7; // 估算文本宽度
    int iColCnt = MAX(1, (i_iWidth - iMarginX) / (iApproxTextWidth + iMarginX));
    int iRowCnt = 4;

    // 确保竖屏视频有足够覆盖率
    if (i_iWidth < i_iHeight) 
    {
        iColCnt = MAX(1, i_iWidth / (iApproxTextWidth + iMarginX)); // 减少列数
        iRowCnt = 4;                                           // 适当减少行数
    }

    // 精确计算水印尺寸
    int iTextWidth = (i_iWidth - iMarginX * (iColCnt + 1)) / iColCnt;
    int iTextHeight = (i_iHeight - iMarginY * (iRowCnt + 1)) / iRowCnt;

    char strFilterDesc[1024];
    int iDescLen=0;
    iDescLen=snprintf(strFilterDesc,sizeof(strFilterDesc),"color=black@0:size=%dx%d,format=yuva420p[bg];[bg]trim=end_frame=1,",i_iWidth,i_iHeight);
    //iDescLen=snprintf(strFilterDesc,sizeof(strFilterDesc),"color=black@0:s=%dx%d[bg];[bg]",i_iWidth,i_iHeight);//trim=end_frame=1 提取第一帧

    // 生成交错水印布局
    for (int row = 0; row < iRowCnt; ++row) 
    {   
        for (int col = 0; col < iColCnt; ++col) 
        {
            // 交错偏移增加覆盖率
            int xOffset = (row % 2) ? iTextWidth / 2 : 0;

            int xPos = col * (iTextWidth + iMarginX) + iMarginX + xOffset;
            int yPos = row * (iTextHeight + iMarginY) + iMarginY;
            
            iDescLen+=snprintf(strFilterDesc+iDescLen,sizeof(strFilterDesc)-iDescLen,"drawtext=text='%s':fontcolor=white@0.7:fontsize=%d:x=%d:y=%d:fontfile=%s,",i_strText,iBaseFontSize,xPos,yPos,i_strFontFile);
        }
    }

    // 改进旋转和平铺处理
    iDescLen+=snprintf(strFilterDesc+iDescLen,sizeof(strFilterDesc)-iDescLen,"rotate=-30*PI/180:c=black@0,trim=end_frame=1[%s];",WATERMARK_FILTER_NAME);//处理后输出命名，便于后续处理
    o_pstrFilterDescr->assign(strFilterDesc);
    //o_strFilterDescr->append("rotate=-30*PI/180:c=black@0,trim=end_frame=1[rot];[0:v][rot]overlay=repeatlast=1[outv]");// 填充的一个颜色和透明度，允许重复平铺

    //if (avfilter_graph_create_filter(o_pptFilter,avfilter_get_by_name("buffer"),WATERMARK_FILTER_NAME, o_pstrFilterDescr->c_str(), NULL, m_ptFilterGraph) < 0) 
    {//使用这种方法创建AVFilterContext，依赖avfilter_get_by_name，如果没有name即没有内置滤镜，则会报错，要改用avfilter_graph_segment_xxx方法   
        //CODEC_LOGE("avfilter_graph_create_filter wm err\n");//同时该方法只能创建一个滤镜，以当前的strFilterDescr则要调用三次这个函数，每次的name分别是color drawtext rotate 即三个滤镜
        //return -1; //使用 "buffer" 内置滤镜来创建第二个输入源（用于水印层）
    } //"buffer -> color -> drawtext -> rotate"  // 水印生成链 
    return 0;
}

/*****************************************************************************
-Fuction        : CreateOverlayFilter
-Description    : 创建overlay滤镜，用于图层叠加
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
多个滤镜之间的链接，
可以命令字符串中用","号或者"[];[]"形式，但是这样命令字符串的语法就比较复杂，不容易判断问题哪个地方，
还有一种方式是用函数avfilter_link链接，结构清晰也便于定位问题
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
            
            iRet = avfilter_graph_segment_apply(seg, 0, &inputs, &outputs);//滤镜描述符解释到输入输出再手动连接起来
            
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
                iRet = avfilter_link(ptLastFilterCtx, 0, ptOverlayFilterCtx, 0);// 5. 连接滤镜 (输入 -> overlay)  
                //水印叠加到视频上
                iRet = avfilter_link(outputs->filter_ctx, 0, ptOverlayFilterCtx, 1);// 连接滤镜 (水印 -> overlay)  
            
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
多个滤镜之间的链接，
可以命令字符串中用","号或者"[];[]"形式，但是这样命令字符串的语法就比较复杂，不容易判断问题哪个地方，
还有一种方式是用函数avfilter_link链接，结构清晰也便于定位问题
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
-Description    : 一帧转两帧，帧数变换则使用framerate滤镜，av_buffersink_get_frame会变多
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
            CODEC_LOGD("FiltAVERROR_EOF,frame->pts%lld, data[0]%x, linesize[0]%d,fwidth%d,height%d \r\n", 
            m_ptFiltFrame->pts, m_ptFiltFrame->data[0], m_ptFiltFrame->linesize[0],m_ptFiltFrame->width, m_ptFiltFrame->height);
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




