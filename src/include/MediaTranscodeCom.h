/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :    MediaTranscodeCom.h
* Description           :    模块内部的底层依赖，由于会用到外部的接口，
所以放到对外的include里
* Created               :    2023.01.13.
* Author                :    Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef MEDIA_TRANSCODE_COM_H
#define MEDIA_TRANSCODE_COM_H


#ifdef _WIN32
#include <Windows.h>
#define SleepMs(val) Sleep(val)
#else
#include <unistd.h>
#define SleepMs(val) usleep(val*1000)
#endif
#include <stdint.h>


#if defined(__EMSCRIPTEN__)
#define  CODEC_LOGW2(val,fmt,...)      printf(fmt,##__VA_ARGS__)
#define  CODEC_LOGE2(val,fmt,...)      printf(fmt,##__VA_ARGS__)
#define  CODEC_LOGD2(val,fmt,...)      printf(fmt,##__VA_ARGS__)
#define  CODEC_LOGW(...)     printf(__VA_ARGS__)
#define  CODEC_LOGE(...)     printf(__VA_ARGS__)
#define  CODEC_LOGD(...)     
#define  CODEC_LOGI(...)     printf(__VA_ARGS__)
#else
#define  CODEC_LOGW2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  CODEC_LOGE2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  CODEC_LOGD2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  CODEC_LOGW(...)     printf(__VA_ARGS__)
#define  CODEC_LOGE(...)     printf(__VA_ARGS__)
#define  CODEC_LOGD(...)     printf(__VA_ARGS__)
#define  CODEC_LOGI(...)     printf(__VA_ARGS__)
#endif



typedef enum
{
	CODEC_TYPE_UNKNOW = 0,
	CODEC_TYPE_H264,
    CODEC_TYPE_H265,
    CODEC_TYPE_VP8,
    CODEC_TYPE_VP9,
    CODEC_TYPE_AAC,
    CODEC_TYPE_G711A,
    CODEC_TYPE_G711U,
    CODEC_TYPE_G726,
    CODEC_TYPE_MP3,
    CODEC_TYPE_OPUS,
    CODEC_TYPE_LPCM,// Linear PCM, platform endian
    CODEC_TYPE_ADPCM,
    CODEC_TYPE_LLPCM,// Linear PCM, little endian
    CODEC_TYPE_JPEG,
    CODEC_TYPE_MJPEG,
}E_CodecType;

typedef enum
{
	CODEC_FRAME_TYPE_UNKNOW = 0,
    CODEC_FRAME_TYPE_VIDEO_I_FRAME,
    CODEC_FRAME_TYPE_VIDEO_P_FRAME,//inner
    CODEC_FRAME_TYPE_VIDEO_B_FRAME,
    CODEC_FRAME_TYPE_AUDIO_FRAME,
}E_CodecFrameType;

typedef struct CodecFrame
{
    unsigned char *pbFrameBuf;//缓冲区
    int iFrameBufMaxLen;//缓冲区总大小
    int iFrameBufLen;//缓冲区中数据的大小
    int iFrameProcessedLen;//解码使用
    
    //裸流的帧数据(数据流,eStreamType需要被赋值为对应的数据流格式)时，下面5个参数都需要外部赋值然后传入
    E_CodecType eEncType;
    int64_t ddwDTS;//ms -1表示时间戳无效，无时间戳
    int64_t ddwPTS;//ms -1表示时间戳无效，无时间戳
    int iFrameRate;//
    unsigned int dwSampleRate;//dwSamplesPerSecond
    E_CodecFrameType eFrameType;
    unsigned int dwWidth;//
    unsigned int dwHeight;//
    unsigned int dwChannels;//音频通道个数
    //如果不设置则使用转码后的格式需要多少采样就用多少,如果转码后的格式对采样数没要求，则解码出多少采样就使用多少采样
    int iAudioFrameSize;//音频数据每帧固定长度大小(每帧读取的样本数量),比如8000采样率的pcma 固定长度是160(通道为1)。aac 44100一帧是是1024(通道为1)
    //音频帧1帧最终大小=iAudioFrameSize*dwChannels

	//输出1帧数据结果(目前用不上，可删除)
    //unsigned char *pbFrameStartPos;//包含00 00 00 01
    //int iFrameLen;
}T_CodecFrame;





#endif
