/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :    MediaTranscodeCom.h
* Description           :    ģ���ڲ��ĵײ����������ڻ��õ��ⲿ�Ľӿڣ�
���Էŵ������include��
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
    unsigned char *pbFrameBuf;//������
    int iFrameBufMaxLen;//�������ܴ�С
    int iFrameBufLen;//�����������ݵĴ�С
    int iFrameProcessedLen;//����ʹ��
    
    //������֡����(������,eStreamType��Ҫ����ֵΪ��Ӧ����������ʽ)ʱ������5����������Ҫ�ⲿ��ֵȻ����
    E_CodecType eEncType;
    int64_t ddwDTS;//ms -1��ʾʱ�����Ч����ʱ���
    int64_t ddwPTS;//ms -1��ʾʱ�����Ч����ʱ���
    int iFrameRate;//
    unsigned int dwSampleRate;//dwSamplesPerSecond
    E_CodecFrameType eFrameType;
    unsigned int dwWidth;//
    unsigned int dwHeight;//
    unsigned int dwChannels;//��Ƶͨ������
    //�����������ʹ��ת���ĸ�ʽ��Ҫ���ٲ������ö���,���ת���ĸ�ʽ�Բ�����ûҪ�����������ٲ�����ʹ�ö��ٲ���
    int iAudioFrameSize;//��Ƶ����ÿ֡�̶����ȴ�С(ÿ֡��ȡ����������),����8000�����ʵ�pcma �̶�������160(ͨ��Ϊ1)��aac 44100һ֡����1024(ͨ��Ϊ1)
    //��Ƶ֡1֡���մ�С=iAudioFrameSize*dwChannels

	//���1֡���ݽ��(Ŀǰ�ò��ϣ���ɾ��)
    //unsigned char *pbFrameStartPos;//����00 00 00 01
    //int iFrameLen;
}T_CodecFrame;





#endif
