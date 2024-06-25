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
    echo -e "Start building lib..."
    CurPwd=$PWD
    OutputPath="$CurPwd/linux/$1"
#<<COMMENT
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
    
    freetypeName="freetype-2.13.2"
    if [ -e "$freetypeName" ]; then
        rm $freetypeName -rf
    fi  
    tar -xf $freetypeName.tar.gz
    aacName="fdk-aac-2.0.3"
    if [ -e "$aacName" ]; then
        rm $aacName -rf
    fi  
    tar -xf $aacName.tar.gz
    x264Name="x264-stable"
    if [ -e "$x264Name" ]; then
        rm $x264Name -rf
    fi  
    tar -xf $x264Name.tar.gz
    x265Name="x265_2.7"
    if [ -e "$x265Name" ]; then
        rm $x265Name -rf
    fi  
    tar -xf $x265Name.tar.gz
    ffmpegName="ffmpeg-7.0.1"
    if [ -e "$ffmpegName" ]; then
        rm $ffmpegName -rf
    fi  
    tar -xf $ffmpegName.tar.gz #tar -xvf
    
    xml2Name="libxml2-2.9.12"
    if [ -e "$xml2Name" ]; then
        rm $xml2Name -rf
    fi  
    tar -xf $xml2Name.tar.gz
    fontconfigName="libfontconfig-5.1.0"
    if [ -e "$fontconfigName" ]; then
        rm $fontconfigName -rf
    fi  
    tar -xf $fontconfigName.tar.gz
    
    
    
    cd $freetypeName
    ./configure --prefix=$OutputPath/$freetypeName
    make -j4;make install
    cd -
    cd $x264Name
#   ./configure  --host=aarch64-apple-darwin --prefix=/opt/local --enable-shared --enable-static --disable-asm
    ./configure --prefix=$OutputPath/$x264Name --enable-shared --enable-static --disable-asm
    make -j4;make install
    cd -
    cd $x265Name/build/linux
#   cmake -DCMAKE_TOOLCHAIN_FILE=crosscompile.cmake -G "Unix Makefiles" ../../source && ccmake ../../source #cmake过程中提示界面依次输入c e g即可安装到指定目录
    cmake -DCMAKE_INSTALL_PREFIX=$OutputPath/$x265Name -G "Unix Makefiles" ../../source && ccmake ../../source
    make -j4;make install
    cd -
#COMMENT
    #x264库所在的位置，ffmpeg 需要链接 x264
    x264LIB_DIR=$OutputPath/$x264Name
    x265LIB_DIR=$OutputPath/$x265Name
    freetypeLIB_DIR=$OutputPath/$freetypeName
    #x264的头文件地址
    x264INC="$x264LIB_DIR/include"
    x265INC="$x265LIB_DIR/include"
    #x264的静态库地址export PKG_CONFIG_PATH=/usr/bin/pkg-config:$PKG_CONFIG_PATH
    x264LIB="$x264LIB_DIR/lib"
    x265LIB="$x265LIB_DIR/lib"
    freetypeLIB="$freetypeLIB_DIR/lib"

    cd $xml2Name
    ./configure --prefix=$OutputPath/$xml2Name -without-python -without-zlib
    make -j4;make install
    cd -
    export PKG_CONFIG_PATH=$freetypeLIB/pkgconfig:$OutputPath/$xml2Name/lib/pkgconfig:$PKG_CONFIG_PATH
    cd $fontconfigName
    ./configure --prefix=$OutputPath/$fontconfigName --enable-libxml2 --enable-static
    make -j4;make install
    cd -
    
    cd $aacName
    ./configure --prefix=$OutputPath/$aacName
    make -j4;make install
    cd -
    export PKG_CONFIG_PATH=$OutputPath/$fontconfigName/lib/pkgconfig:$OutputPath/$aacName/lib/pkgconfig:$freetypeLIB/pkgconfig:$x264LIB/pkgconfig:$x265LIB/pkgconfig:$PKG_CONFIG_PATH #找到 x264 库的 pkg-config 文件
    cd $ffmpegName
    #--enable-vaapi 后续支持vaapi(封装底层显卡编解码库的硬件加速库，例如win端的d3dxl)，ERROR: vaapi requested but not found如果想要使用drawtext，编译时需要加上--enable-libfreetype。要选择多种字体，需要加上--enable-libfontconfig。还需要字体变形，需要加上--enable-libfribidi
    ADD_FEATURE="--enable-static --enable-shared --enable-gpl --enable-debug --enable-libx264 --enable-libx265 --enable-libfreetype --enable-libfontconfig --enable-nonfree --enable-libfdk-aac" #--extra-cflags=-I$x264INC -I$x265INC --extra-ldflags=-L$x264LIB -L$x265LIB" 有pkgconfig就暂不需要指定
#./configure --prefix=/mnt/d/linuxCode/third/ffmpegmsvc --target-os=win64 --arch=x86_64 --enable-shared --toolchain=msvc --enable-gpl --enable-debug --enable-libx264 --enable-libx265 --enable-libvpx --enable-vaapi
#./configure --enable-gpl --enable-libx264 --enable-libx265 --enable-libvpx --enable-vaapi #--pkg-config="pkg-config --static"
#./configure --prefix=/mnt/d/linuxCode/third/ffmpeglinux --arch=x86_64 --enable-static --enable-gpl --enable-debug --enable-libx264 --enable-libx265 --enable-libvpx --enable-vaapi
    ./configure --prefix=$OutputPath/$ffmpegName --pkg-config-flags="--static" --arch=x86_64 --disable-x86asm $ADD_FEATURE 
    make -j4;make install
    cd -
    
    echo -e "export PKG_CONFIG_PATH=$PKG_CONFIG_PATH "
    echo -e "./configure --prefix=$OutputPath/$ffmpegName --pkg-config-flags="--static" --arch=x86_64 --disable-x86asm $ADD_FEATURE "
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
    cp $CurPwd/build/lib/libHlsServer.a .

    cp $CurPwd/include . -rf


}

if [ $# == 0 ]; then
    PrintUsage
    exit -1
else
#   GenerateCmakeFile $1
    BuildLib $1
#    CopyLib $2 $1
fi




