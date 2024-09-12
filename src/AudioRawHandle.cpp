/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       AudioRawHandle.cpp
* Description           : 	    小采样率转大采样率可能样本不够，需要添加静音帧
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "AudioRawHandle.h"
#include "MediaTranscodeCom.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <functional>

//#include <sys/time.h>
//#include "cJSON.h"


/*****************************************************************************
-Fuction        : AudioRawHandle
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
AudioRawHandle::AudioRawHandle()
{
    m_ptAudioFifo = NULL;
    m_ptResmapleCtx = NULL;//
    m_ppbConvertedSamples=NULL; 
    m_iCurConvertedSamplesSize = 0;//
    m_iDstFrameSize = 0;//
    m_ptAudioFrame = av_frame_alloc();
    m_iAudioFrameBufSamples=0;
    m_ddwAudioFramePTS=0;
    m_ddwAudioFrameBasePTS=0;
}
/*****************************************************************************
-Fuction        : ~AudioRawHandle
-Description    : ~AudioRawHandle
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
AudioRawHandle::~AudioRawHandle()
{
    if(NULL!=m_ptResmapleCtx)
    {
        swr_free(&m_ptResmapleCtx);
        m_ptResmapleCtx=NULL;
    }
    if(NULL!=m_ptAudioFifo)
    {
        av_audio_fifo_free(m_ptAudioFifo);
        m_ptAudioFifo=NULL;
    }
    if (NULL != m_ppbConvertedSamples) 
    {
        av_freep(&m_ppbConvertedSamples[0]);
        av_freep(&m_ppbConvertedSamples);
        m_ppbConvertedSamples = NULL;
    }
    if(NULL!=m_ptAudioFrame)
    {
        av_frame_free(&m_ptAudioFrame);
        m_ptAudioFrame=NULL;
        m_iAudioFrameBufSamples=0;
        m_ddwAudioFramePTS=0;
        m_ddwAudioFrameBasePTS=0;
    }
}
/*****************************************************************************
-Fuction        : Init
-Description    : FilterInit
* @example transcode_aac.c
*
* @example resampling_audio.c
* libswresample API use example.
* 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioRawHandle::Init(AVCodecContext *i_ptSrcDecodeCtx,AVCodecContext *i_ptDstEncodeCtx,int i_iDstFrameSize)
{
    int iRet = -1;
    
    if (i_ptDstEncodeCtx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) 
    {
        m_iDstFrameSize = i_iDstFrameSize;//
    }
    /* Initialize the resampler to be able to convert audio sample formats. */
    /*
     * Create a resampler context for the conversion.
     * Set the conversion parameters.
     */
    iRet = swr_alloc_set_opts2(&m_ptResmapleCtx,&i_ptDstEncodeCtx->ch_layout,i_ptDstEncodeCtx->sample_fmt,i_ptDstEncodeCtx->sample_rate,
                           &i_ptSrcDecodeCtx->ch_layout,i_ptSrcDecodeCtx->sample_fmt,i_ptSrcDecodeCtx->sample_rate,0, NULL);
    if (iRet < 0)
    {
        CODEC_LOGE("Could not allocate resample context\n");
        return iRet;
    }
    CODEC_LOGW("AudioRawHandle dst->nb_channels%d sample_fmt%d sample_rate%d,src->nb_channels%d sample_fmt%d sample_rate%d\r\n",
    i_ptDstEncodeCtx->ch_layout.nb_channels,i_ptDstEncodeCtx->sample_fmt,i_ptDstEncodeCtx->sample_rate,
    i_ptSrcDecodeCtx->ch_layout.nb_channels,i_ptSrcDecodeCtx->sample_fmt,i_ptSrcDecodeCtx->sample_rate);
    /*
    * Perform a sanity check so that the number of converted samples is
    * not greater than the number of samples to be converted.
    * If the sample rates differ, this case has to be handled differently
    */
    //av_assert0(output_codec_context->sample_rate == input_codec_context->sample_rate);

    /* Open the resampler with the specified parameters. */
    if ((iRet = swr_init(m_ptResmapleCtx)) < 0) 
    {
        CODEC_LOGE("Could not open resample context\n");
        swr_free(&m_ptResmapleCtx);
        m_ptResmapleCtx=NULL;
        return iRet;
    }



    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(m_ptAudioFifo = av_audio_fifo_alloc(i_ptDstEncodeCtx->sample_fmt,i_ptDstEncodeCtx->ch_layout.nb_channels, 1))) 
    {
        CODEC_LOGE( "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }

    return 0;

}

/*****************************************************************************
-Fuction        : RawHandle
-Description    : 调用完这个函数需要再调用GetFrame，才能获取到处理后的音频帧
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioRawHandle::RawHandle(AVFrame *m_ptAVFrame,AVCodecContext *i_ptDstEncodeCtx)
{
    int iRet = -1;
    unsigned char *pbFrameData=NULL;
    unsigned int dwFrameLen=0;
    int iSampleNum =0;

    if(NULL==m_ptAVFrame || NULL==i_ptDstEncodeCtx)
    {
        CODEC_LOGE("NULL==i_ptAVFrame || NULL==i_ptDstEncodeCtx RawHandle err \r\n");
        return iRet;
    }
    m_ddwAudioFrameBasePTS=m_ptAVFrame->pts;
    if(0 == m_ddwAudioFramePTS)//理论上时间戳初始化一次就好后面需要重新计算//实测还是得使用外部传入的时间戳，否则时间戳会被加大，导致音频放的很慢
    {//第一次使用外面传进来的帧时间戳，后续使用自己计算的时间戳
        m_ddwAudioFramePTS=m_ddwAudioFrameBasePTS;//因为转码，特别是采样率变化的转码，外部传入的时间戳是不准的
    }//有时会出现输入帧但是不输出帧的情况，因为样本数可能不够要更多数据，也有可能输入一帧数据太多，输出就会有多帧
    m_ptAVFrame->pts = m_ptAVFrame->best_effort_timestamp;//当该标志被设置时，FFmpeg会尝试使用最接近的时间戳来表示每个解码帧的时间戳，即尽可能接近原始媒体中的时间戳

    /* If there is decoded data, convert and store it. */
    /* Initialize the temporary storage for the converted input samples. */
    iRet = InitConvertedSamples(i_ptDstEncodeCtx,m_ptAVFrame->nb_samples);
    if(iRet<0)
    {
        CODEC_LOGE("InitConvertedSamples err \r\n");
        return iRet;
    }
    
    /* Convert the samples using the resampler. *///重采样，低采样率的样本数转换为高采样的样本数要用类似插值法
    iSampleNum = swr_convert(m_ptResmapleCtx, m_ppbConvertedSamples, m_iCurConvertedSamplesSize, (const uint8_t**)m_ptAVFrame->data, m_ptAVFrame->nb_samples);//高转低则要去掉多余的样本
    if (0 > iSampleNum) //最后输出目标采样率对应的(所需要的)样本数
    {
        CODEC_LOGE("swr_convert (error '%d')\n",iSampleNum);
        //CODEC_LOGE("Could not convert input samples (error '%s')\n",av_err2str(iSampleNum));//error: taking address of temporary array
        return iSampleNum;
    }
    CODEC_LOGD("swr_convert nb_samples%d iSampleNum%d m_iCurConvertedSamplesSize%d\n",m_ptAVFrame->nb_samples,iSampleNum,m_iCurConvertedSamplesSize);
    
    /* Make the FIFO as large as it needs to be to hold both, the old and the
     * new samples. */
    int iUsedSize = av_audio_fifo_size(m_ptAudioFifo);
    int iSpaceSize = av_audio_fifo_space(m_ptAudioFifo);
    int iAllSize = iUsedSize + iSpaceSize;
    if (iAllSize > (1024 * 1024 * 10)) //最大缓存10M
    {
        CODEC_LOGE("iAllSize%d > %d %d(1024 * 1024 * 10)\n",iAllSize,iUsedSize,iSpaceSize);
        return -1;
    }
    if (iSpaceSize < iSampleNum) //<frame_size
    {
        if ((iRet = av_audio_fifo_realloc(m_ptAudioFifo, iUsedSize + iSampleNum)) < 0) 
        {
            CODEC_LOGE("Could not reallocate FIFO\n");
            return iRet;
        }
    }
    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(m_ptAudioFifo, (void**)m_ppbConvertedSamples, iSampleNum) < iSampleNum) 
    {
        CODEC_LOGE("Could not write data to FIFO\n");
        return -1;
    }
    return 0;
}

/*****************************************************************************
-Fuction        : GetFrame
-Description    : 
先调用RawHandle把源数据放入缓冲区(缓冲区中数据足够帧长要求再取出进行编码)
再不断调用并判断返回值为0才表示缓存中的帧数据已经读完
-Input          : (0 != i_iUseAllSampleFlag) //剩余样本也强制处理(不按照设置的帧长去处理)
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioRawHandle::GetFrame(AVFrame *m_ptAVFrame,AVCodecContext *i_ptDstEncodeCtx,int i_iUseAllSampleFlag) 
{
    int iRet = -1;
    int iUpperSampleSize =0;

    
    if(NULL==m_ptAudioFifo)
    {
        CODEC_LOGE("NULL==m_ptAudioFifo GetFrame err \r\n");
        return iRet;
    }
    int iSamplesInFifo = av_audio_fifo_size(m_ptAudioFifo);
    if (0 >= iSamplesInFifo) 
    {
        CODEC_LOGW("av_audio_fifo_size 0 >= iSamplesInFifo %d\r\n",iSamplesInFifo);
        return 0;
    }
    // 默认使用自定义采样大小,便于固定帧长
    int iFrameSize = m_iDstFrameSize;
    if (0 >= iFrameSize) 
    {
        iFrameSize = i_ptDstEncodeCtx->frame_size > 0 ? i_ptDstEncodeCtx->frame_size : iSamplesInFifo;
    }

    iRet=0;
    if (av_audio_fifo_size(m_ptAudioFifo) >= iFrameSize) 
    {
        iRet = this->GetFrameFromFifo(m_ptAVFrame,i_ptDstEncodeCtx,iFrameSize);// 按标准长度读取并编码
        if (0 > iRet) 
        {
            return iRet;
        }
    }
    
    if (0 != i_iUseAllSampleFlag) //剩余样本也强制处理(不按照设置的帧长去处理)
    {
        iFrameSize = av_audio_fifo_size(m_ptAudioFifo);
        if (0 < iFrameSize) 
        {
            return this->GetFrameFromFifo(m_ptAVFrame,i_ptDstEncodeCtx,iFrameSize);
        }
    }// i_ptAVFrame->pkt_size, 只有视频帧才会赋值，音频帧看i_ptAVFrame->nb_samples
    CODEC_LOGD("av_audio_fifo_size iSamplesInFifo %d,m_iDstFrameSize %d,i_ptDstEncodeCtx->frame_size %d,iFrameSize %d AVFrame->nb_samples%d\r\n",
    iSamplesInFifo,m_iDstFrameSize,i_ptDstEncodeCtx->frame_size,iFrameSize,m_ptAVFrame->nb_samples);
    return iRet;
}

/*****************************************************************************
-Fuction        : InitConvertedSamples
-Description    : 初始化重采样缓存，如果之前的缓存大小够用，则
不要再分配内存
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioRawHandle::InitConvertedSamples(AVCodecContext *i_ptDstEncodeCtx,int i_iSampleNum) 
{
    int iRet = -1;
    int iUpperSampleSize =0;

    iUpperSampleSize = swr_get_out_samples(m_ptResmapleCtx,i_iSampleNum);
    if (0 > iUpperSampleSize) 
    {
        return AVERROR(ENOMEM);
    }
    //已经分配过足够大小的缓存
    if (m_iCurConvertedSamplesSize > iUpperSampleSize) 
    {
        return 0;
    }


    if (NULL != m_ppbConvertedSamples) 
    {
        av_freep(&m_ppbConvertedSamples[0]);
        av_freep(&m_ppbConvertedSamples);
        m_ppbConvertedSamples = NULL;
    }
    //CODEC_LOGD("av_samples_alloc_array_and_samples iUpperSampleSize %d ,%p\n",iUpperSampleSize,i_ptDstEncodeCtx);

    /* Allocate as many pointers as there are audio channels.
     * Each pointer will point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     * Allocate memory for the samples of all channels in one consecutive
     * block for convenience. */
    if ((iRet = av_samples_alloc_array_and_samples(&m_ppbConvertedSamples, NULL,
                                  i_ptDstEncodeCtx->ch_layout.nb_channels,
                                  iUpperSampleSize,
                                  i_ptDstEncodeCtx->sample_fmt, 0)) < 0) 
    {
        //CODEC_LOGE("Could not allocate converted input samples (error '%s')\n",av_err2str(iRet));//error: taking address of temporary array
        CODEC_LOGE("Could not allocate converted input samples (error '%d')\n",iRet);
        return iRet;
    }
    m_iCurConvertedSamplesSize=iUpperSampleSize;

    return 0;
}

/*****************************************************************************
-Fuction        : GetFrameFromFifo
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioRawHandle::GetFrameFromFifo(AVFrame *m_ptAVFrame,AVCodecContext *i_ptDstEncodeCtx,int i_iFrameSize) 
{
    int iRet = -1;

    if(NULL==m_ptAudioFrame)
    {
        CODEC_LOGE("NULL==m_ptAudioFrame GetFrameFromFifo err \r\n");
        return iRet;
    }
    iRet = this->InitAudioFrame(i_ptDstEncodeCtx,i_iFrameSize);
    if (0 > iRet) 
    {
        CODEC_LOGE("InitAudioFrame err \r\n");
        return iRet;
    }
    
    /* Read as many samples from the FIFO buffer as required to fill the
     * frame. The samples are stored in the frame temporarily. */
    int iReadSize = av_audio_fifo_read(m_ptAudioFifo, (void**)m_ptAudioFrame->data, i_iFrameSize);
    if (iReadSize < i_iFrameSize) 
    {
        CODEC_LOGE("av_audio_fifo_read err \r\n");
        return -1;
    }

    m_ptAudioFrame->nb_samples = iReadSize;
    
    // pts重计算ms
    m_ptAudioFrame->pts = m_ddwAudioFramePTS;
    // 计算下一帧时间戳
    m_ddwAudioFramePTS += 1000 * m_ptAudioFrame->nb_samples / m_ptAudioFrame->sample_rate;
    // 时间戳比输入时间戳还小时，将输入时间戳设定为当前帧时间戳
    if (m_ddwAudioFramePTS < m_ddwAudioFrameBasePTS)
    {
        CODEC_LOGD("m_ddwAudioFramePTS < m_ddwAudioFrameBasePTS \r\n");
        m_ptAudioFrame->pts = m_ddwAudioFrameBasePTS;
        m_ddwAudioFramePTS = m_ddwAudioFrameBasePTS + 1000 * m_ptAudioFrame->nb_samples / m_ptAudioFrame->sample_rate;
    }


  
    //av_frame_move_ref(m_ptAVFrame,m_ptAudioFrame);//后续下面的操作可以优化为这一个
    iRet=av_frame_ref(m_ptAVFrame,m_ptAudioFrame);
    if (iRet < 0)
    {
        CODEC_LOGE("GetFrameFromFifo av_frame_ref err %d\r\n",iRet);
        //av_frame_unref(m_ptAudioFrame);//内存还要重复使用,InitAudioFrame里会处理,这里就注释掉
        return iRet;
    }
    iRet=av_frame_copy(m_ptAVFrame,m_ptAudioFrame);
    if (iRet < 0)
    {
        CODEC_LOGE("GetFrameFromFifo av_frame_copy err %d \r\n",iRet);
        av_frame_unref(m_ptAVFrame);
    }
    //av_frame_unref(m_ptAudioFrame);//内存还要重复使用,InitAudioFrame里会处理,这里就注释掉
    return m_ptAudioFrame->nb_samples;
}


/*****************************************************************************
-Fuction        : InitAudioFrame
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioRawHandle::InitAudioFrame(AVCodecContext *i_ptDstEncodeCtx,int i_iFrameSize) 
{
    int iRet = -1;


    if(NULL==i_ptDstEncodeCtx)
    {
        CODEC_LOGE("NULL==i_ptDstEncodeCtx InitAudioFrame err \r\n");
        return iRet;
    }
    
    if(m_iAudioFrameBufSamples>=i_iFrameSize)//减少频繁申请内存
    {
        m_ptAudioFrame->nb_samples = i_iFrameSize;
        //m_ptAudioFrame->channel_layout = i_ptDstEncodeCtx->channel_layout;//旧版本操作ffmpeg4.2.1
        av_channel_layout_copy(&(m_ptAudioFrame)->ch_layout, &i_ptDstEncodeCtx->ch_layout);//新版本操作ffmpeg 7.0.1
        m_ptAudioFrame->format = (int)i_ptDstEncodeCtx->sample_fmt;
        m_ptAudioFrame->sample_rate = i_ptDstEncodeCtx->sample_rate;
        return 0;
    }

    
    av_frame_unref(m_ptAudioFrame);

    /* Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity. */
    m_ptAudioFrame->nb_samples = i_iFrameSize;
    //m_ptAudioFrame->channel_layout = i_ptDstEncodeCtx->channel_layout;//旧版本操作ffmpeg4.2.1
    av_channel_layout_copy(&(m_ptAudioFrame)->ch_layout, &i_ptDstEncodeCtx->ch_layout);//新版本操作ffmpeg 7.0.1
    m_ptAudioFrame->format = (int)i_ptDstEncodeCtx->sample_fmt;
    m_ptAudioFrame->sample_rate = i_ptDstEncodeCtx->sample_rate;

    /* Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified. */
    if ((iRet = av_frame_get_buffer(m_ptAudioFrame, 0)) < 0) 
    {
        CODEC_LOGE("av_frame_get_buffer err \r\n");
        return iRet;
    }
    
    m_iAudioFrameBufSamples=m_ptAudioFrame->nb_samples;
    return 0;
}

/*****************************************************************************
-Fuction        : AudioRawFilter
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
AudioRawFilter::AudioRawFilter()
{
    m_ptFiltFrame = NULL;
    m_ptBufferSrcCtx = NULL;//
    m_ptBufferSinkCtx = NULL;//
    m_ptFilterGraph = NULL;//
}
/*****************************************************************************
-Fuction        : ~AudioRawFilter
-Description    : ~AudioRawFilter
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
AudioRawFilter::~AudioRawFilter()
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
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioRawFilter::Init(AVCodecContext *i_ptDecodeCtx,AVCodecContext* i_ptEncodeCtx)
{
    int iRet = -1;
	char args[256];
    char src_ch_layout[64];
    char filter_descr[256];
    char dst_ch_layout[64];


    if(NULL==i_ptDecodeCtx||NULL==i_ptEncodeCtx)
    {
        CODEC_LOGE("NULL==i_ptDecodeCtx||NULL==i_ptEncodeCtx err \r\n");
        return iRet;
    }
	if (i_ptDecodeCtx->codec_type != AVMEDIA_TYPE_AUDIO)
    {
        CODEC_LOGE("i_ptDecodeCtx->codec_type != AVMEDIA_TYPE_AUDIO err \r\n");
        return iRet;
    }
    
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
        CODEC_LOGE("avfilter_graph_alloc err \r\n");
        return iRet;
    }
    const AVFilter *buffersrc = avfilter_get_by_name("abuffer");
    const AVFilter *buffersink = avfilter_get_by_name("abuffersink");
	if (!buffersrc || !buffersink)
	{
        CODEC_LOGE("avfilter_get_by_name err \r\n");
        return iRet;
	}
	
	/*snprintf(args, sizeof(args),"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channels=%d:channel_layout=0x%I64x",
		i_ptDecodeCtx->time_base.num, i_ptDecodeCtx->time_base.den, i_ptDecodeCtx->sample_rate,
		av_get_sample_fmt_name(i_ptDecodeCtx->sample_fmt),i_ptDecodeCtx->ch_layout.nb_channels,i_ptDecodeCtx->ch_layout.u.mask);*/

    /* buffer audio source: the decoded frames from the decoder will be inserted here. */
    if (i_ptDecodeCtx->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC)
        av_channel_layout_default(&i_ptDecodeCtx->ch_layout, i_ptDecodeCtx->ch_layout.nb_channels);
    memset(src_ch_layout,0,sizeof(src_ch_layout));
    av_channel_layout_describe(&i_ptDecodeCtx->ch_layout, src_ch_layout, sizeof(src_ch_layout));
    iRet = snprintf(args, sizeof(args),"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channels=%d:channel_layout=%s",
    i_ptDecodeCtx->time_base.num, i_ptDecodeCtx->time_base.den, i_ptDecodeCtx->sample_rate,av_get_sample_fmt_name(i_ptDecodeCtx->sample_fmt),i_ptDecodeCtx->ch_layout.nb_channels,src_ch_layout);

    do
    {
        iRet = avfilter_graph_create_filter(&m_ptBufferSrcCtx, buffersrc, "in",args, NULL, m_ptFilterGraph);
        if (iRet < 0) 
        {
            CODEC_LOGE( "Cannot create buffer source err \n");
            break;
        }
        iRet = avfilter_graph_create_filter(&m_ptBufferSinkCtx, buffersink, "out",NULL, NULL, m_ptFilterGraph);
        if (iRet < 0) 
        {
            CODEC_LOGE("Cannot create buffer sink err \n");
            break;
        }
        iRet = av_opt_set_bin(m_ptBufferSinkCtx, "sample_fmts", (uint8_t*)&i_ptEncodeCtx->sample_fmt,sizeof(i_ptEncodeCtx->sample_fmt), AV_OPT_SEARCH_CHILDREN);
        if (iRet < 0)
        {
            CODEC_LOGE("av_opt_set_bin sample_fmts err\n");
            break;
        }
        iRet = av_opt_set_bin(m_ptBufferSinkCtx, "sample_rates", (uint8_t*)&i_ptEncodeCtx->sample_rate,sizeof(i_ptEncodeCtx->sample_rate), AV_OPT_SEARCH_CHILDREN);
        if (iRet < 0)
        {
            CODEC_LOGE("av_opt_set_bin sample_rates err\n");
            break;
        }
        memset(dst_ch_layout,0,sizeof(dst_ch_layout));
        av_channel_layout_describe(&i_ptEncodeCtx->ch_layout, dst_ch_layout, sizeof(dst_ch_layout));
        iRet = av_opt_set(m_ptBufferSinkCtx, "ch_layouts",dst_ch_layout,AV_OPT_SEARCH_CHILDREN);
        if (iRet < 0)
        {
            CODEC_LOGE("av_opt_set ch_layouts err\n");
            break;
        }
        inputs->name = av_strdup("out");
        inputs->filter_ctx = m_ptBufferSinkCtx;
        inputs->pad_idx = 0;
        inputs->next = NULL;
        outputs->name = av_strdup("in");
        outputs->filter_ctx = m_ptBufferSrcCtx;
        outputs->pad_idx = 0;
        outputs->next = NULL;
        if (!inputs->name || !outputs->name)
        {
            CODEC_LOGE("!inputs->name || !outputs->name err\n");
            break;
        }
        snprintf(filter_descr,sizeof(filter_descr),"aresample=%d,aformat=sample_fmts=%s:channel_layouts=%s",i_ptEncodeCtx->sample_rate,av_get_sample_fmt_name(i_ptEncodeCtx->sample_fmt),dst_ch_layout);
        iRet=avfilter_graph_parse_ptr(m_ptFilterGraph,filter_descr,&inputs,&outputs, NULL);//("aresample=%d", _resamplerate);//;"anull"
        if (iRet < 0) 
        {
            CODEC_LOGE("avfilter_graph_parse_ptr err %s \r\n","anull");
            break;
        }
        iRet=avfilter_graph_config(m_ptFilterGraph, NULL);
        if (iRet < 0) 
        {
            CODEC_LOGE("avfilter_graph_config err %s \r\n","anull");
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
int AudioRawFilter::RawHandle(AVFrame *i_ptSrcAVFrame, int i_iFlags)
{
    int iRet = -1;

    if(NULL==i_ptSrcAVFrame)
    {
        CODEC_LOGE("NULL==i_ptSrcAVFrame RawHandle err \r\n");
        return iRet;
    }
    if (av_buffersrc_add_frame_flags(m_ptBufferSrcCtx, i_ptSrcAVFrame, i_iFlags) < 0) 
    {
        CODEC_LOGE("av_buffersrc_add_frame_flags err\n");
        return iRet;
    }
    return 0;
}
/*****************************************************************************
-Fuction        : GetFrame
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int AudioRawFilter::GetFrame(AVFrame *o_ptDstAVFrame) 
{
    int iRet = -1;

    if(NULL==o_ptDstAVFrame)
    {
        CODEC_LOGE("NULL==o_ptDstAVFrame RawHandle err \r\n");
        return iRet;
    }
    /* pull filtered frames from the filtergraph */
    iRet = av_buffersink_get_frame(m_ptBufferSinkCtx, m_ptFiltFrame);
    if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF)
    {
        iRet=0;
        CODEC_LOGD("AudioRawFilter AVERROR_EOF,frame->pts%lld, nb_samples%d,\r\n", m_ptFiltFrame->pts, m_ptFiltFrame->nb_samples);
        return iRet;
    }
    if (iRet < 0)
    {
        return iRet;
    }
    iRet=av_frame_ref(o_ptDstAVFrame,m_ptFiltFrame);
    if (iRet < 0)
    {
        CODEC_LOGE("av_frame_ref err \r\n");
        av_frame_unref(m_ptFiltFrame);
        return iRet;
    }
    iRet=av_frame_copy(o_ptDstAVFrame,m_ptFiltFrame);
    if (iRet < 0)
    {
        CODEC_LOGE("av_frame_copy err \r\n");
        av_frame_unref(o_ptDstAVFrame);
    }
    av_frame_unref(m_ptFiltFrame);
    
    CODEC_LOGD("AudioRawFilter ,frame->pts%lld, nb_samples%d,\r\n", o_ptDstAVFrame->pts, o_ptDstAVFrame->nb_samples);
    return o_ptDstAVFrame->nb_samples;
}

