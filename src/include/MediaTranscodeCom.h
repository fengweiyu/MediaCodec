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



#define  CODEC_LOGW2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  CODEC_LOGE2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  CODEC_LOGD2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  CODEC_LOGW(...)     printf(__VA_ARGS__)
#define  CODEC_LOGE(...)     printf(__VA_ARGS__)
#define  CODEC_LOGD(...)     printf(__VA_ARGS__)
#define  CODEC_LOGI(...)     printf(__VA_ARGS__)




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
    uint64_t ddwDTS;//ms
    uint64_t ddwPTS;//ms
    int iFrameRate;//
    unsigned int dwSampleRate;//dwSamplesPerSecond
    E_CodecFrameType eFrameType;
    unsigned int dwWidth;//
    unsigned int dwHeight;//

	//���1֡���ݽ��(Ŀǰ�ò��ϣ���ɾ��)
    //unsigned char *pbFrameStartPos;//����00 00 00 01
    //int iFrameLen;
}T_CodecFrame;



typedef struct MediaFrameInfo
{
    unsigned char *pbFrameBuf;//������
    int iFrameBufMaxLen;//�������ܴ�С
    int iFrameBufLen;//�������������ݵ��ܴ�С
    int iFrameProcessedLen;
    
    //������֡����(������,eStreamType��Ҫ����ֵΪ��Ӧ����������ʽ)ʱ������5����������Ҫ�ⲿ��ֵȻ����
    E_StreamType eStreamType;//�����ļ�ʱ,eStreamType�ⲿ��ֵ0(��ʾ����������(���ļ�)),����4�����������ڲ���ֵ
    E_MediaEncodeType eEncType;
    unsigned int dwTimeStamp;//ms
    unsigned int dwSampleRate;//dwSamplesPerSecond
    E_MediaFrameType eFrameType;
    unsigned int dwWidth;//
    unsigned int dwHeight;//

	//���1֡���ݽ��
    unsigned char *pbFrameStartPos;//����00 00 00 01
    int iFrameLen;
    unsigned int dwNaluCount;//����sps,pps�Ȳ�������Ӧ��nalu
    T_MediaNaluInfo atNaluInfo[MAX_NALU_CNT_ONE_FRAME];//����һ֡ͼ����Ƭ�ɶ��(nalu)�����
    unsigned char bNaluLenSize;//avcc��ʽ�洢nalu�ǵ�naluLen�Ĵ�С(��ռ�ֽ���)
    //unsigned int adwNaluEndOffset[MAX_NALU_CNT_ONE_FRAME];
    T_VideoEncodeParam tVideoEncodeParam;
    T_AudioEncodeParam tAudioEncodeParam;
}T_MediaFrameInfo;




#endif
