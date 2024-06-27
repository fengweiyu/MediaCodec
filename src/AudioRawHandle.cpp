/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       AudioRawHandle.cpp
* Description           : 	
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
    
    m_iDstFrameSize = i_iDstFrameSize;//
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

    if(NULL==m_ptAVFrame)
    {
        CODEC_LOGE("NULL==i_ptAVFrame RawHandle err \r\n");
        return iRet;
    }
    m_ddwAudioFramePTS=m_ptAVFrame->pts;
    m_ddwAudioFrameBasePTS=m_ptAVFrame->pts;
    m_ptAVFrame->pts = m_ptAVFrame->best_effort_timestamp;//当该标志被设置时，FFmpeg会尝试使用最接近的时间戳来表示每个解码帧的时间戳，即尽可能接近原始媒体中的时间戳

    /* If there is decoded data, convert and store it. */
    /* Initialize the temporary storage for the converted input samples. */
    iRet = InitConvertedSamples(i_ptDstEncodeCtx,m_ptAVFrame->nb_samples);
    if(iRet<0)
    {
        CODEC_LOGE("InitConvertedSamples err \r\n");
        return iRet;
    }
    
    /* Convert the samples using the resampler. */
    iSampleNum = swr_convert(m_ptResmapleCtx, m_ppbConvertedSamples, m_iCurConvertedSamplesSize, (const uint8_t**)m_ptAVFrame->data, m_ptAVFrame->nb_samples);
    if (0 > iSampleNum) 
    {
        CODEC_LOGE("swr_convert (error '%d')\n",iSampleNum);
        //CODEC_LOGE("Could not convert input samples (error '%s')\n",av_err2str(iSampleNum));//error: taking address of temporary array
        return iSampleNum;
    }

    
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
先调用RawHandle把源数据放入缓冲区
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
        return 0;
    }
    // 默认使用自定义采样大小,便于固定帧长
    int iFrameSize = m_iDstFrameSize;
    if (0 >= iFrameSize) 
    {
        iFrameSize = i_ptDstEncodeCtx->frame_size > 0 ? i_ptDstEncodeCtx->frame_size : iSamplesInFifo;
    }
    
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
    }
    return 0;
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


