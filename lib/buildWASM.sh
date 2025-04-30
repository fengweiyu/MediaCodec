#!/bin/bash
#在cygwin编译使用，注意cygwin要安装好gcc-g++ 12,make,cmake,nasm-2.13以上，pkt-config 2.3.0 libtool 2.5.3  gtk-doc 1.33
#cygwin注意环境编译设置不要带有空格，否则编译会报错(sh Program 等错误)，cygwin中执行echo $PATH查看环境变量，然后设置环境变量export PATH=/usr/local/bin:/usr/bin就可以了

#考虑后续和build.sh合并
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
#    source 
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
    mkdir -p $OutputPath
    
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
#   cmake -DCMAKE_TOOLCHAIN_FILE=crosscompile.cmake -G "Unix Makefiles" ../../source && ccmake ../../source #cmake过程中提示界面依次输入c e g即可安装到指定目录 -G "Unix Makefiles"
    sed -i '/message(STATUS "Found Nasm ${NASM_VERSION_STRING} to build assembly primitives")/{N; s/.*/    option(ENABLE_ASSEMBLY "Enable use of assembly coded primitives" OFF)/;}' ../../source/CMakeLists.txt
    sed -i '/add_definitions(-march=i686)/d' ../../source/CMakeLists.txt
    emcmake cmake ../../source -DCMAKE_INSTALL_PREFIX=$OutputPath/$x265Name 
    make -j6;make install
    cd -
    cd $x264Name
#   ./configure  --host=aarch64-apple-darwin --prefix=/opt/local --enable-shared --enable-static --disable-asm  --host=i386-unknown-linux-gnu --extra-cflags=-fdeclspec --disable-asm --disable-thread
    emconfigure ./configure --host=i386-unknown-linux-gnu --extra-cflags=-fdeclspec --disable-asm --disable-thread --enable-static --prefix=$OutputPath/$x264Name 
    make -j6;make install
    cd -
    
    cd $freetypeName/builds/unix
#    emconfigure ./configure --disable-shared --enable-static --prefix=$OutputPath/$freetypeName 
    emcmake cmake ../.. -DCMAKE_INSTALL_PREFIX=$OutputPath/$freetypeName
    make -j6;make install
    cd -
    cd $harfbuzzName
    sh autogen.sh 
#   ./configure --prefix=/mnt/d/linuxCode/third/MediaCodec/lib/linux --host=aarch64-linux-gnu --enable-shared CC=aarch64-linux-gnu-gcc --with-glib=no --with-cairo=no --with-chafa=no --with-icu=no --with-freetype=no
#    emconfigure ./configure --disable-shared --enable-static --with-glib=no --with-cairo=no --with-chafa=no --with-icu=no --with-freetype=yes --prefix=/ff/linux/x86/harfbuzzName FREETYPE_CFLAGS=-I/ff/linux/x86/freetype-2.13.2/include/freetype2 FREETYPE_LIBS="-L/ff/linux/x86/freetype-2.13.2/lib -lfreetype"
    emconfigure ./configure --disable-shared --enable-static --with-glib=no --with-cairo=no --with-chafa=no --with-icu=no --with-freetype=yes --prefix=$OutputPath/$harfbuzzName FREETYPE_CFLAGS=-I$OutputPath/$freetypeName/include/freetype2 FREETYPE_LIBS="-L$OutputPath/$freetypeName/lib -lfreetype"
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
    #export PKG_CONFIG_PATH=/ff/linux/x86/freetype-2.13.2/lib/pkgconfig:/ff/linux/x86/libxml2-2.9.12/lib/pkgconfig:$PKG_CONFIG_PATH
    cd $fontconfigName
    if [ $1 == cygwin ]; then
        ./configure --prefix=$OutputPath/$fontconfigName --enable-libxml2 --enable-static --disable-shared 
    else #wasm时不能带emconfigure 否则无法使用pkg-config --with-pkgconfigdir 
        ./configure  CC="emcc" CFLAGS="-D__linux__=1" --enable-libxml2 --enable-static --disable-shared --prefix=$OutputPath/$fontconfigName
        #emconfigure ./configure CFLAGS="-D__linux__=1" --enable-libxml2 --enable-static --disable-shared --prefix=$OutputPath/$fontconfigName LIBXML2_CFLAGS=-I$OutputPath/$xml2Name/include/libxml2 LIBXML2_LIBS="-L$OutputPath/$xml2Name/lib -lxml2" FREETYPE_CFLAGS=-I$OutputPath/$freetypeName/include/freetype2 FREETYPE_LIBS="-L$OutputPath/$freetypeName/lib -lfreetype"
    fi
    make -j6;make install
    if [ ! -e "$OutputPath/$fontconfigName/lib/pkgconfig" ]; then
        echo "no exist $OutputPath/$fontconfigName/lib/pkgconfig"
        mkdir $OutputPath/$fontconfigName/lib/pkgconfig
        cp fontconfig.pc $OutputPath/$fontconfigName/lib/pkgconfig
    fi  
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
    #export PKG_CONFIG_PATH=/ff/linux/x86/libfontconfig-5.1.0/lib/pkgconfig:/ff/linux/x86/fdk-aac-2.0.3/lib/pkgconfig:/ff/linux/x86/freetype-2.13.2/lib/pkgconfig:/ff/linux/x86/harfbuzz-8.5.0/lib/pkgconfig:/ff/linux/x86/x265_2.7/lib/pkgconfig:/ff/linux/x86/x264-stable/lib/pkgconfig:$PKG_CONFIG_PATH
    cd $ffmpegName
    #--enable-vaapi 后续支持vaapi(封装底层显卡编解码库的硬件加速库，例如win端的d3dxl)，ERROR: vaapi requested but not found如果想要使用drawtext，编译时需要加上--enable-libfreetype -enable-libharfbuzz。要选择多种字体，需要加上--enable-libfontconfig。还需要字体变形，需要加上--enable-libfribidi
    ADD_FEATURE="--enable-static --disable-shared --enable-gpl --enable-debug --enable-libx264 --enable-libx265 --enable-libfreetype --enable-libharfbuzz --enable-libfontconfig --enable-nonfree --enable-libfdk-aac" #--extra-cflags=-I$x264INC -I$x265INC --extra-ldflags=-L$x264LIB -L$x265LIB" 有pkgconfig就暂不需要指定
#./configure --prefix=/mnt/d/linuxCode/third/ffmpegmsvc --target-os=win64 --arch=x86_64 --enable-shared --toolchain=msvc --enable-gpl --enable-debug --enable-libx264 --enable-libx265 --enable-libvpx --enable-vaapi
#./configure --enable-gpl --enable-libx264 --enable-libx265 --enable-libvpx --enable-vaapi #--pkg-config="pkg-config --static"
#NASM 是 x86 平台汇编器，不需要交叉编译。若是 arm 等其他平台，交叉编译工具链中包含有对应的汇编器，则交叉编译 ffmpeg 时需要 --disable-x86asm 选项 ，wasm时不能带emconfigure 否则无法使用pkg-config
#./configure --prefix=/mnt/d/linuxCode/third/ffmpeglinux --arch=x86_64 --enable-static --enable-gpl --enable-debug --enable-libx264 --enable-libx265 --enable-libvpx --enable-vaapi --disable-x86asm asm汇编加速 会导致scale滤镜花屏(ffmpeg 5.0以前的版本不会,编译要装nasm,否则编译不过,最终链接的程序不需要链接asm库 即无需交叉编译asm库)
    #ADD_LIB=ranlib="emranlib"  --enable-cross-compile --target-os=none --arch=x86_64 --cpu=generic  禁用汇编优化，防止 Emscripten 尝试编译这些不支持的汇编指令 \ --disable-programs --disable-ffmpeg --disable-ffplay --disable-ffprobe 
    ADD_FLAG="--enable-cross-compile --target-os=none --arch=x86_64 --disable-asm --disable-programs --disable-ffmpeg --disable-ffplay --disable-ffprobe"
    #emconfigure ./configure --cc="emcc" --cxx="em++" --ar="emar" --pkg-config-flags="--static" --prefix=$OutputPath/$ffmpegName $ADD_FEATURE --extra-cflags="-I$OutputPath/$fontconfigName/include -I$OutputPath/$aacName/include -I$freetypeLIB_DIR/include -I$x264INC -I$x265INC" --extra-ldflags="-L$OutputPath/$fontconfigName/lib -lfontconfig -L$OutputPath/$aacName/lib -lfdk-aac -L$freetypeLIB -lfreetype -L$x264LIB -lx264 -L$x265LIB -lx265"
    ./configure --cc="emcc" --cxx="em++" --ar="emar" --pkg-config-flags="--static" --prefix=$OutputPath/$ffmpegName $ADD_FEATURE $ADD_FLAG
    if [ $1 == cygwin ]; then
        cp ../wchar_filename.h ./libavutil/wchar_filename.h -f
    fi
    make -j6;make install
    cd -
#emconfigure ./configure --cc="emcc" --cxx="em++" --ar="emar" --ranlib="emranlib" --prefix=$(pwd)/dist --enable-cross-compile --target-os=none --arch=x86_64 --cpu=generic --enable-gpl --enable-version3 
#--disable-avdevice --disable-swresample --disable-postproc --disable-avfilter --disable-programs --disable-logging --disable-everything --enable-avformat --enable-decoder=hevc --enable-decoder=h264 
#--enable-decoder=aac --enable-decoder=pcm_alaw --enable-decoder=pcm_mulaw --disable-ffplay --disable-ffprobe --enable-asm --disable-doc --disable-devices --disable-network --disable-hwaccels --disable-parsers 
#--disable-bsfs --disable-debug --enable-protocol=file --enable-demuxer=mov --enable-demuxer=flv --disable-indevs --disable-outdevs --disable-pthreads ;
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




