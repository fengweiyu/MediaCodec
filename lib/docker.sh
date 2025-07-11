#!/bin/bash
#在cygwin编译使用，注意cygwin要安装好gcc-g++ 12,make,cmake,nasm-2.13以上，pkt-config 2.3.0 libtool 2.5.3  gtk-doc 1.33
#cygwin注意环境编译设置不要带有空格，否则编译会报错(sh Program 等错误)，cygwin中执行echo $PATH查看环境变量，然后设置环境变量export PATH=/usr/local/bin:/usr/bin就可以了
docker ps 
docker images
docker run -it 4f8243b3b9e9 /bin/bash
docker run -v D:\code\ThirdSources\MediaCodec\MediaCodec\lib\ff:/ff -it 50407bba74f0 /bin/bash



docker run --rm -it -v D:\code\202311MyEye_SDK\XProtocolN:/home -v C:\Users\Admin\.ssh:/root/.ssh --privileged 50407bba74f0 /bin/bash

cd /home/ && mkdir -p build && cd build && emcmake cmake ..  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../package_static -DTARGET_OS=WASM  && make -j6 && make install


docker run --rm -it -v D:\code\202311MyEye_SDK\XProtocolN:/home -v C:\Users\Admin\.ssh:/root/.ssh --privileged 50407bba74f0 bash -c "chmod 600 /root/.ssh/* && source /emsdk/emsdk_env.sh && cd /home/ && mkdir -p build && cd build && emcmake cmake ..  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../package_static -DTARGET_OS=WASM  && make -j6 && make install"


docker cp D:\code\ThirdSources\MediaCodec\MediaCodec\lib\ff 19dcdc790911:/tmp
docker stop 19dcdc790911

docker commit <container-id> <new-image-name>
docker save -o <output-file-name>.tar <image-name>:<tag>
docker save -o my-app.tar my-app:latest
docker load -i /path/to/your/directory/my-app.tar  





































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
    echo -e "Start building lib..."
    CurPwd=$PWD
    OutputPath="$CurPwd/linux/$1"
    if [ $1 == cygwin ]; then
        OutputPath="$CurPwd/win/$1"
    fi
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
    
    x265Name="x265_2.7"
    if [ -e "$x265Name" ]; then
        rm $x265Name -rf
    fi  
    tar -xf $x265Name.tar.gz
    x264Name="x264-stable"
    if [ -e "$x264Name" ]; then
        rm $x264Name -rf
    fi  
    tar -xf $x264Name.tar.gz
    freetypeName="freetype-2.13.2"
    if [ -e "$freetypeName" ]; then
        rm $freetypeName -rf
    fi  
    tar -xf $freetypeName.tar.gz
    harfbuzzName="harfbuzz-8.5.0"
    if [ -e "$harfbuzzName" ]; then
        rm $harfbuzzName -rf
    fi  
    tar -xf $harfbuzzName.tar.xz
    iconvName="libiconv-1.17"
    if [ -e "$iconvName" ]; then
        rm $iconvName -rf
    fi  
    tar -xf $iconvName.tar.gz
    aacName="fdk-aac-2.0.3"
    if [ -e "$aacName" ]; then
        rm $aacName -rf
    fi  
    tar -xf $aacName.tar.gz
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
    
    
    if [ $1 == cygwin ]; then
        export PATH=/usr/local/bin:/usr/bin #防止cygwin环境编译设置有空格导致的编译报错
    fi
    
    
    cd $x265Name/build/linux
#   cmake -DCMAKE_TOOLCHAIN_FILE=crosscompile.cmake -G "Unix Makefiles" ../../source && ccmake ../../source #cmake过程中提示界面依次输入c e g即可安装到指定目录
    sed -i '/add_definitions(-march=i686)/d' ../../source/CMakeLists.txt
    emcmake cmake -DCMAKE_INSTALL_PREFIX=$OutputPath/$x265Name -G "Unix Makefiles" ../../source 
    make -j6;make install
    cd -
    cd $x264Name
#   ./configure  --host=aarch64-apple-darwin --prefix=/opt/local --enable-shared --enable-static --disable-asm  --host=i386-unknown-linux-gnu --extra-cflags=-fdeclspec --disable-asm --disable-thread
    emconfigure ./configure --host=i386-unknown-linux-gnu --extra-cflags=-fdeclspec --disable-asm --disable-thread --enable-static --prefix=$OutputPath/$x264Name 
    make -j6;make install
    cd -
    
    cd $freetypeName
    emconfigure ./configure --prefix=$OutputPath/$freetypeName 
    make -j6;make install
    cd -
    cd $harfbuzzName
    sh autogen.sh 
#   ./configure --prefix=/mnt/d/linuxCode/third/MediaCodec/lib/linux --host=aarch64-linux-gnu --enable-shared CC=aarch64-linux-gnu-gcc --with-glib=no --with-cairo=no --with-chafa=no --with-icu=no --with-freetype=no
    emconfigure ./configure --prefix=$OutputPath/$harfbuzzName --enable-static --with-glib=no --with-cairo=no --with-chafa=no --with-icu=no --with-freetype=yes FREETYPE_CFLAGS=-I$OutputPath/$freetypeName/include/freetype2 FREETYPE_LIBS="-L$OutputPath/$freetypeName/lib -lfreetype"
#    mkdir build;cd build
#    cmake -DCMAKE_INSTALL_PREFIX=$OutputPath/$harfbuzzName ..
    if [ $1 == cygwin ]; then
        cp ../VarCompositeGlyph.hh ./src/OT/glyf/VarCompositeGlyph.hh -f
    fi
    make -j6;make install
#    cp ./src/hb-ft.h $OutputPath/$harfbuzzName/include/harfbuzz  #因为其不会主动安装这个头文件，ffmpeg又需要，所以暂时这么拷贝
    cd $CurPwd

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
    if [ $1 == cygwin ]; then
        ./configure --prefix=$OutputPath/$xml2Name -without-python -without-zlib --enable-static --disable-shared #解决 编译器会以连接动态库的方式去连接静态库(undefined reference to `__imp xxx)
    else
        emconfigure ./configure --prefix=$OutputPath/$xml2Name -without-python -without-zlib 
    fi
    make -j6;make install
    cd -
    export PKG_CONFIG_PATH=$freetypeLIB/pkgconfig:$OutputPath/$xml2Name/lib/pkgconfig:$PKG_CONFIG_PATH
    cd $fontconfigName
    if [ $1 == cygwin ]; then
        ./configure --prefix=$OutputPath/$fontconfigName --enable-libxml2 --enable-static --disable-shared 
    else
        emconfigure ./configure --prefix=$OutputPath/$fontconfigName --enable-libxml2 --enable-static 
    fi
    make -j6;make install
    cd -
    cd $iconvName
    emconfigure ./configure --enable-static --disable-shared --prefix=$OutputPath/$iconvName
    make -j6;make install
    cd -
    cd $aacName
    emconfigure ./configure --enable-static --disable-shared --prefix=$OutputPath/$aacName 
    make -j6;make install
    cd -
    export PKG_CONFIG_PATH=$OutputPath/$fontconfigName/lib/pkgconfig:$OutputPath/$aacName/lib/pkgconfig:$OutputPath/$harfbuzzName/lib/pkgconfig:$freetypeLIB/pkgconfig:$x264LIB/pkgconfig:$x265LIB/pkgconfig:$PKG_CONFIG_PATH #找到 x264 库的 pkg-config 文件
    cd $ffmpegName
    #--enable-vaapi 后续支持vaapi(封装底层显卡编解码库的硬件加速库，例如win端的d3dxl)，ERROR: vaapi requested but not found如果想要使用drawtext，编译时需要加上--enable-libfreetype -enable-libharfbuzz。要选择多种字体，需要加上--enable-libfontconfig。还需要字体变形，需要加上--enable-libfribidi
    ADD_FEATURE="--enable-static --enable-shared --enable-gpl --enable-debug --enable-libx264 --enable-libx265 --enable-libfreetype --enable-libharfbuzz --enable-libfontconfig --enable-nonfree --enable-libfdk-aac" #--extra-cflags=-I$x264INC -I$x265INC --extra-ldflags=-L$x264LIB -L$x265LIB" 有pkgconfig就暂不需要指定
#./configure --prefix=/mnt/d/linuxCode/third/ffmpegmsvc --target-os=win64 --arch=x86_64 --enable-shared --toolchain=msvc --enable-gpl --enable-debug --enable-libx264 --enable-libx265 --enable-libvpx --enable-vaapi
#./configure --enable-gpl --enable-libx264 --enable-libx265 --enable-libvpx --enable-vaapi #--pkg-config="pkg-config --static"
#NASM 是 x86 平台汇编器，不需要交叉编译。若是 arm 等其他平台，交叉编译工具链中包含有对应的汇编器，则交叉编译 ffmpeg 时需要 --disable-x86asm 选项
#./configure --prefix=/mnt/d/linuxCode/third/ffmpeglinux --arch=x86_64 --enable-static --enable-gpl --enable-debug --enable-libx264 --enable-libx265 --enable-libvpx --enable-vaapi --disable-x86asm asm汇编加速 会导致scale滤镜花屏(ffmpeg 5.0以前的版本不会,编译要装nasm,否则编译不过,最终链接的程序不需要链接asm库 即无需交叉编译asm库)
    emconfigure ./configure --cc="emcc" --cxx="em++" --ar="emar" --pkg-config-flags="--static" --prefix=$OutputPath/$ffmpegName $ADD_FEATURE 
    if [ $1 == cygwin ]; then
        cp ../wchar_filename.h ./libavutil/wchar_filename.h -f
    fi
    make -j6;make install
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




