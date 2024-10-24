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
    if [ $1 == x86 -o $1 == x64 -o $1 == cygwin ]; then
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
    LibPath=$PWD/../lib/linux
    if [ $1 == cygwin ]; then
        LibPath=$PWD/../lib/win
    fi
    if [ -e "$LibPath/$1" ]; then
        echo "$LibPath/$1 exist! "
    else
        cd $LibPath
        tar -xvf $1.tar.gz
        cd -
    fi  
    
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


    ThirdLibPath=$CurPwd/../lib/linux
    if [ $2 == cygwin ]; then
        ThirdLibPath=$CurPwd/../lib/win
    fi
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
    mkdir fdk-aac
    mkdir fdk-aac/lib
    cp $ThirdLibPath/$2/ffmpeg-7.0.1/lib/*.a $1/ThirdLib/ffmpeg/lib -rf
    cp $ThirdLibPath/$2/ffmpeg-7.0.1/include $1/ThirdLib/ffmpeg -rf
    cp $ThirdLibPath/$2/x264-stable/lib/*.a $1/ThirdLib/x264/lib -rf
    cp $ThirdLibPath/$2/x264-stable/include $1/ThirdLib/x264 -rf
    cp $ThirdLibPath/$2/x265_2.7/lib/*.a $1/ThirdLib/x265/lib -rf
    cp $ThirdLibPath/$2/x265_2.7/include $1/ThirdLib/x265 -rf
    cp $ThirdLibPath/$2/fdk-aac-2.0.3/lib/*.a $1/ThirdLib/fdk-aac/lib -rf
    cp $ThirdLibPath/$2/fdk-aac-2.0.3/include $1/ThirdLib/fdk-aac -rf
    
    mkdir freetype
    mkdir freetype/lib
    mkdir harfbuzz
    mkdir harfbuzz/lib
    mkdir libxml2
    mkdir libxml2/lib
    mkdir libfontconfig
    mkdir libfontconfig/lib
    mkdir libiconv
    mkdir libiconv/lib
    cp $ThirdLibPath/$2/freetype-2.13.2/lib/*.a $1/ThirdLib/freetype/lib -rf
    cp $ThirdLibPath/$2/freetype-2.13.2/include $1/ThirdLib/freetype -rf
    cp $ThirdLibPath/$2/harfbuzz-8.5.0/lib/*.a $1/ThirdLib/harfbuzz/lib -rf
    cp $ThirdLibPath/$2/harfbuzz-8.5.0/include $1/ThirdLib/harfbuzz -rf
    cp $ThirdLibPath/$2/libxml2-2.9.12/lib/*.a $1/ThirdLib/libxml2/lib -rf
    cp $ThirdLibPath/$2/libxml2-2.9.12/include $1/ThirdLib/libxml2 -rf
    cp $ThirdLibPath/$2/libfontconfig-5.1.0/lib/*.a $1/ThirdLib/libfontconfig/lib -rf
    cp $ThirdLibPath/$2/libfontconfig-5.1.0/include $1/ThirdLib/libfontconfig -rf
    cp $ThirdLibPath/$2/libiconv-1.17/lib/*.a $1/ThirdLib/libiconv/lib -rf
    cp $ThirdLibPath/$2/libiconv-1.17/include $1/ThirdLib/libiconv -rf
    cp $CurPwd/../lib/msyh.ttf $1
    if [ $2 == cygwin ]; then
        cp $CurPwd/../lib/cygwin1.dll $1
    fi
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




