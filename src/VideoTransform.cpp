/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       VideoTransform.cpp
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "VideoTransform.h"
#include "MediaTranscodeCom.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <functional>
#include <time.h>

//#include <sys/time.h>

/*****************************************************************************
-Fuction        : VideoTransform
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
VideoTransform::VideoTransform()
{
    m_pVideoDecode = NULL;
    m_pVideoRawHandle = NULL;
    m_pVideoEncode = NULL;
    m_ptAVFrame = av_frame_alloc();
    m_ptAVFrameRGBA = av_frame_alloc();
    m_iSetWaterMarkFlag=0;
    m_strWaterMarkText.assign("yuweifeng08016");
    m_strWaterMarkFontFile.assign("msyh.ttf");
}
/*****************************************************************************
-Fuction        : ~VideoTransform
-Description    : ~VideoTransform
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
VideoTransform::~VideoTransform()
{
    if(NULL!= m_pVideoDecode)
    {
        delete m_pVideoDecode;
        m_pVideoDecode = NULL;
    }
    if(NULL!= m_pVideoRawHandle)
    {
        delete m_pVideoRawHandle;
        m_pVideoRawHandle = NULL;
    }
    if(NULL!= m_pVideoEncode)
    {
        delete m_pVideoEncode;
        m_pVideoEncode = NULL;
    }
    if(NULL!=m_ptAVFrame)
    {
        av_frame_free(&m_ptAVFrame);
        m_ptAVFrame=NULL;
    }
    if(NULL!=m_ptAVFrameRGBA)
    {
        av_frame_free(&m_ptAVFrameRGBA);
        m_ptAVFrameRGBA=NULL;
    }
}
/*****************************************************************************
-Fuction        : ~VideoTransform
-Description    : ~VideoTransform
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoTransform::SetWaterMarkParam(int i_iEnable,const char * i_strText,const char * i_strFontFile)
{
    if(0 == i_iEnable)
    {
        m_iSetWaterMarkFlag=0;
        CODEC_LOGI("m_iSetWaterMarkFlag %d \r\n",m_iSetWaterMarkFlag);
        return 0;
    }
    if(NULL==i_strText||NULL==i_strFontFile)
    {
        CODEC_LOGE("SetWaterMarkParam NULL err \r\n");
        return -1;
    }
    CODEC_LOGI("i_iEnable %d i_strText %s i_strFontFile %s\r\n",m_iSetWaterMarkFlag,i_strText,i_strFontFile);
    m_iSetWaterMarkFlag=1;
    m_strWaterMarkText.assign(i_strText);
    m_strWaterMarkFontFile.assign(i_strFontFile);
    return 0;
}

/*****************************************************************************
-Fuction        : DecodeToRGB
-Description    : //return ResLen,<0 err
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoTransform::DecodeToRGB(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame)
{
    int iRet = -1;
    char strTemp[1024] = { 0 };
    struct SwsContext * ptSwsCtx = NULL; // 转换上下文，可以设置为成员变量，但是用了sws_getCachedContext可以避免重复初始化，所以临时变量也可以
    unsigned char * pbRgbaBuffer = NULL;
    int iRgbaSize = 0;

    if(NULL==m_ptAVFrame)
    {
        CODEC_LOGE("NULL==m_ptAVFrame err \r\n");
        return iRet;
    }

    
    if(NULL== m_pVideoDecode)
    {
        m_pVideoDecode = new VideoDecode();
        if(NULL==m_pVideoDecode)
        {
            CODEC_LOGE("NULL==m_pVideoDecode err \r\n");
            return iRet;
        }
        iRet=m_pVideoDecode->Init(i_pSrcFrame->eEncType);
    }
    av_frame_unref(m_ptAVFrame);
    iRet=m_pVideoDecode->Decode(i_pSrcFrame->pbFrameBuf,i_pSrcFrame->iFrameBufLen,i_pSrcFrame->ddwPTS,i_pSrcFrame->ddwDTS,m_ptAVFrame);
    if(iRet<0)
    {
        CODEC_LOGE("m_pVideoDecode->Decode err \r\n");
        return iRet;
    }
    
    m_ptAVFrame->pts = m_ptAVFrame->best_effort_timestamp;//当该标志被设置时，FFmpeg会尝试使用最接近的时间戳来表示每个解码帧的时间戳，即尽可能接近原始媒体中的时间戳

    if(NULL== m_pVideoRawHandle)
    {
        m_pVideoRawHandle = new VideoRawHandle();
        if(NULL==m_pVideoRawHandle)
        {
            CODEC_LOGE("NULL==m_pVideoRawHandle err \r\n");
            return iRet;
        }
        AVCodecContext *ptCodecContext=NULL;
        iRet=m_pVideoDecode->GetCodecContext(&ptCodecContext);
        string strWaterMarkFilter;
        string strScaleFilter;
        string * aptFilter[2];
        int i=0;
        m_pVideoRawHandle->CreateScaleFilter(o_pDstFrame->dwWidth,o_pDstFrame->dwHeight,&strScaleFilter);
        aptFilter[i++]=&strScaleFilter;
        if(m_iSetWaterMarkFlag!=0)
        {
            m_pVideoRawHandle->CreateWaterMarkFilter(o_pDstFrame->dwWidth,o_pDstFrame->dwHeight,m_strWaterMarkText.c_str(),m_strWaterMarkFontFile.c_str(),&strWaterMarkFilter);
            aptFilter[i++]=&strWaterMarkFilter;
        }
        iRet=m_pVideoRawHandle->Init(ptCodecContext,aptFilter,i);
    }
    iRet=m_pVideoRawHandle->RawHandle(m_ptAVFrame);
    if(iRet<0)
    {
        CODEC_LOGE("m_pVideoRawHandle->RawHandle err \r\n");
        return iRet;
    }

    //2. 分配RGBA缓冲区  
    iRgbaSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_ptAVFrame->width, m_ptAVFrame->height, 1); //int rgba_size = m_ptAVFrame->width * m_ptAVFrame->height * 4; // RGBA: 每像素4字节  
    if(o_pDstFrame->iFrameBufMaxLen<iRgbaSize)
    {
        CODEC_LOGE("o_pDstFrame->iFrameBufMaxLen%d<%diRgbaSize err \r\n",o_pDstFrame->iFrameBufMaxLen,iRgbaSize);
        return iRet;
    }
    pbRgbaBuffer = o_pDstFrame->pbFrameBuf;//pbRgbaBuffer = (uint8_t *)av_malloc(iRgbaSize);  
    av_image_fill_arrays(m_ptAVFrameRGBA->data, m_ptAVFrameRGBA->linesize,
       pbRgbaBuffer, AV_PIX_FMT_RGBA, m_ptAVFrame->width, m_ptAVFrame->height,1); // m_ptAVFrameRGBA用于接收转换结果  


    //3. 初始化SWSContext（YUV →RGBA）  
    ptSwsCtx = sws_getCachedContext(ptSwsCtx,m_ptAVFrame->width, m_ptAVFrame->height,(AVPixelFormat)m_ptAVFrame->format, // 输入格式（如YUV420P）  
        m_ptAVFrame->width, m_ptAVFrame->height,AV_PIX_FMT_RGBA,             // 输出格式（RGBA）  
        SWS_BILINEAR, NULL, NULL, NULL  );  

    
    // 4. 执行转换（YUV →RGBA）  
    sws_scale(ptSwsCtx, m_ptAVFrame->data, m_ptAVFrame->linesize,  // 输入YUV数据  
        0, m_ptAVFrame->height,              // 起始行和高度  
        m_ptAVFrameRGBA->data, m_ptAVFrameRGBA->linesize // 输出RGBA数据  
    );  
    // 此时 m_ptAVFrameRGBA->data[0] 包含连续的RGBA数据  // 每个像素占4字节（R,G,B,A），可直接用于渲染或保存为PNG  
    // 此时 rgba_buffer 中已是连续的RGBA数据  // 格式: [R0,G0,B0,A0, R1,G1,B1,A1, ..., Rn,Gn,Bn,An]  
            
    // 5. 释放资源  //av_free(rgba_buffer);  
    sws_freeContext(ptSwsCtx); 
    
    // 计算帧数据长度  // 数据总字节数  
    o_pDstFrame->iFrameBufLen = m_ptAVFrameRGBA->width * m_ptAVFrameRGBA->height * 4; // RGBA 每个像素4字节  

    iRet=o_pDstFrame->iFrameBufLen;
    return iRet;
}

/*****************************************************************************
-Fuction        : Transform
-Description    : //return ResLen,<0 err
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int VideoTransform::Transform(T_CodecFrame *i_pSrcFrame,T_CodecFrame *o_pDstFrame)
{
    int iRet = -1;
    char strTemp[1024] = { 0 };
    
    if(NULL==m_ptAVFrame)
    {
        CODEC_LOGE("NULL==m_ptAVFrame err \r\n");
        return iRet;
    }

    
    if(NULL== m_pVideoDecode)
    {
        m_pVideoDecode = new VideoDecode();
        if(NULL==m_pVideoDecode)
        {
            CODEC_LOGE("NULL==m_pVideoDecode err \r\n");
            return iRet;
        }
        iRet=m_pVideoDecode->Init(i_pSrcFrame->eEncType);
    }
    av_frame_unref(m_ptAVFrame);
    iRet=m_pVideoDecode->Decode(i_pSrcFrame->pbFrameBuf,i_pSrcFrame->iFrameBufLen,i_pSrcFrame->ddwPTS,i_pSrcFrame->ddwDTS,m_ptAVFrame);
    if(iRet<0)
    {
        CODEC_LOGE("m_pVideoDecode->Decode err \r\n");
        return iRet;
    }
    
    m_ptAVFrame->pts = m_ptAVFrame->best_effort_timestamp;//当该标志被设置时，FFmpeg会尝试使用最接近的时间戳来表示每个解码帧的时间戳，即尽可能接近原始媒体中的时间戳
#if 1
    if(NULL== m_pVideoRawHandle)
    {
        m_pVideoRawHandle = new VideoRawHandle();
        if(NULL==m_pVideoRawHandle)
        {
            CODEC_LOGE("NULL==m_pVideoRawHandle err \r\n");
            return iRet;
        }
        AVCodecContext *ptCodecContext=NULL;
        iRet=m_pVideoDecode->GetCodecContext(&ptCodecContext);
        time_t rawtime = time(NULL);
        struct tm *timeinfo = localtime(&rawtime); //
        char strBuffer[64] = { 0 };
        strftime(strBuffer,sizeof(strBuffer),"%Y-%m-%d %H-%M-%S",timeinfo);//,scale=xx:h=xx //scale=w=iw/2:h=ih/2 //scale=320:-1
        //drawtext=fnotallow=FreeSans.ttf:fnotallow=green:fnotallow=25:x=50:y=20:text= ///path/to/msyh.ttf 
        //drawtext=fontfile=msyhbd.ttc:fontcolor=red:fontsize=25:x=50:y=20:text=
        //-vf drawtext=fontcolor=white:fontsize=40:fontfile=msyh.ttf:text='Hello World':x=0:y=100
        //drawtext=fontfile=msyh.ttf:fontcolor=white:fontsize=25:x=50:y=20:text=\"%s\"
        snprintf(strTemp,sizeof(strTemp),"scale=%d:%d,drawtext=fontfile=msyh.ttf:fontcolor=white:alpha=0.8:fontsize=20:x=20:y=20:text=%s",o_pDstFrame->dwWidth,o_pDstFrame->dwHeight,strBuffer);//asctime(timeinfo)
        string strWaterMarkFilter;
        string strScaleFilter;
        string * aptFilter[2];
        int i=0;
        m_pVideoRawHandle->CreateScaleFilter(o_pDstFrame->dwWidth,o_pDstFrame->dwHeight,&strScaleFilter);
        aptFilter[i++]=&strScaleFilter;
        if(m_iSetWaterMarkFlag!=0)
        {
            m_pVideoRawHandle->CreateWaterMarkFilter(o_pDstFrame->dwWidth,o_pDstFrame->dwHeight,m_strWaterMarkText.c_str(),m_strWaterMarkFontFile.c_str(),&strWaterMarkFilter);
            aptFilter[i++]=&strWaterMarkFilter;
        }
        //CODEC_LOGD("RawHandle:%s \r\n",strTemp);
        iRet=m_pVideoRawHandle->Init(ptCodecContext,aptFilter,i);
    }
    iRet=m_pVideoRawHandle->RawHandle(m_ptAVFrame);
    if(iRet<0)
    {
        CODEC_LOGE("m_pVideoRawHandle->RawHandle err \r\n");
        return iRet;
    }
#endif 

    if(NULL== m_pVideoEncode)
    {
        m_pVideoEncode = new VideoEncode();
        if(NULL==m_pVideoEncode)
        {
            CODEC_LOGE("NULL==m_pVideoEncode err \r\n");
            return iRet;
        }
        iRet=m_pVideoEncode->Init(o_pDstFrame->eEncType,o_pDstFrame->iFrameRate,(int)o_pDstFrame->dwWidth,(int)o_pDstFrame->dwHeight);
    }
    iRet=m_pVideoEncode->Encode(m_ptAVFrame,o_pDstFrame->pbFrameBuf,(unsigned int)o_pDstFrame->iFrameBufMaxLen,
    &o_pDstFrame->iFrameRate,&o_pDstFrame->eFrameType,&o_pDstFrame->ddwPTS,&o_pDstFrame->ddwDTS);
    if(iRet<=0)
    {
        CODEC_LOGE("m_pVideoEncode->Encode err \r\n");
        return -1;
    }
    o_pDstFrame->iFrameBufLen=iRet;
    
    return iRet;
}

