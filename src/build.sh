#!/bin/bash

function PrintUsage()
{
    echo -e "Usage:"
    echo -e "./build.sh $ToolChain"
    echo -e "ToolChain: arm-linux/x86/x64"
    echo -e "EGG:"
    echo -e "./build.sh x86"
    echo -e " or ./build.sh x64"
}
function GenerateCmakeFile()
{
#   mkdir -p build
    CmakeFile="$2/ToolChain.cmake"
    echo "SET(CMAKE_SYSTEM_NAME \"Linux\")" > $CmakeFile
    if [ $1 == x86 -o $1 == x64 ]; then
        echo "SET(CMAKE_C_COMPILER \"gcc\")" >> $CmakeFile  
        echo "SET(CMAKE_CXX_COMPILER \"g++\")" >> $CmakeFile        
    else
        echo "SET(CMAKE_C_COMPILER \"$1-gcc\")" >> $CmakeFile
        echo "SET(CMAKE_CXX_COMPILER \"$1-g++\")" >> $CmakeFile
        
    fi
    echo "SET(CMAKE_ToolChain \"$1\")" >> $CmakeFile
}
function BuildLib()
{
    echo -e "Start building src..."
    OutputPath="./build"
    if [ -e "$OutputPath" ]; then
        rm $OutputPath -rf
#防止切换平台编译时由于平台不对应报错，所以删除build重新建立       
#       if [ -e "$OutputPath/lib" ]; then
#           echo "/build/lib exit"
#       else
#           mkdir $OutputPath/lib
#       fi
#   else
#       mkdir $OutputPath
#       mkdir $OutputPath/lib
    fi  
    mkdir $OutputPath
    mkdir $OutputPath/lib
        
    GenerateCmakeFile $1 $OutputPath    
    cd $OutputPath
    cmake ..
    if [ -e "Makefile" ]; then  
        make clean
        make -j4 > /dev/null
        if [ $? == 0 ]; then
            echo "make success! "
        else
            echo "make failed! "
            exit -1
        fi
    else
        echo "Makefile generated failed! "
        exit -1
    fi
    cd ..
}

function CopyLib()
{
#   CurPwd = $PWD
    CurPwd=$PWD
    cd $1
    if [ -e "lib" ]; then
        echo "lib exit"
    else
        mkdir lib
    fi
    
    cd lib
    cp $CurPwd/build/lib/libMediaTranscode.a .

    cp $CurPwd/include . -rf



    cd $1
    rm ./ThirdLib -rf
    mkdir ThirdLib
    cd ThirdLib
    mkdir ffmpeg
    mkdir ffmpeg/lib
#    mkdir ffmpeg/include
    mkdir x264
    mkdir x264/lib
#    mkdir x264/include
    mkdir x265
    mkdir x265/lib
#    mkdir x265/include
    cp $CurPwd/../lib/linux/$2/ffmpeg-7.0.1/lib/*.a $1/ThirdLib/ffmpeg/lib -rf
    cp $CurPwd/../lib/linux/$2/ffmpeg-7.0.1/include $1/ThirdLib/ffmpeg -rf
    cp $CurPwd/../lib/linux/$2/x264-stable/lib/*.a $1/ThirdLib/x264/lib -rf
    cp $CurPwd/../lib/linux/$2/x264-stable/include $1/ThirdLib/x264 -rf
    cp $CurPwd/../lib/linux/$2/x265_2.7/lib/*.a $1/ThirdLib/x265/lib -rf
    cp $CurPwd/../lib/linux/$2/x265_2.7/include $1/ThirdLib/x265 -rf

    cd $CurPwd
}

if [ $# == 0 ]; then
    PrintUsage
    exit -1
else

    
#   GenerateCmakeFile $1
    BuildLib $1
    CopyLib $2 $1
fi




