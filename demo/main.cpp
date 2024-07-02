/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       main.cpp
* Description           : 	    
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>

#include "MediaTranscodeDemo.h"

static void PrintUsage(char *i_strProcName);

/*****************************************************************************
-Fuction        : main
-Description    : main
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int main(int argc, char* argv[]) 
{
    int iRet = -1;
    MediaTranscodeDemo oMediaTranscodeDemo;

    if(argc ==2)
    {
        iRet = oMediaTranscodeDemo.proc(argv[1]);//Transcode(argv[1],argv[2],NULL,NULL)
    }
    else if(argc ==3)
    {
        iRet = oMediaTranscodeDemo.Transcode(argv[1],argv[2]);//Transcode(argv[1],argv[2],NULL,NULL)
    }
    else
    {
        PrintUsage(argv[0]);
        iRet = oMediaTranscodeDemo.proc("demo.json");//oMediaTranscodeDemo.proc("demo.h264","demo.h265");
    }
    if(iRet<0)
    {
        printf("%s run err,please note that usage and demo.json \r\n",argv[0]);
        printf("Usage: %s file.json or\r\n",argv[0]);
        printf("Usage: %s inputFile outputFile\r\n",argv[0]);
    }
    return iRet;
}
/*****************************************************************************
-Fuction        : PrintUsage
-Description    : 
printf("run default args: %s demo.h264 demo.h265\r\n",i_strProcName);
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static void PrintUsage(char *i_strProcName)
{
    printf("Usage: %s file.json or\r\n",i_strProcName);
    printf("Usage: %s inputFile outputFile\r\n",i_strProcName);
    //printf("eg: %s 9112 77.72.169.210 3478\r\n",i_strProcName);
    printf("run default args: %s demo.json\r\n",i_strProcName);
}

