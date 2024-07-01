#!/bin/bash

function PrintUsage()
{
    echo -e "Usage:"
    echo -e "./build.sh $ToolChain"
    echo -e "ToolChain: arm-linux/x86"
    echo -e "EGG:"
    echo -e "./build.sh arm-linux"
    echo -e " or ./build.sh x86"
}
function GenerateCmakeFile()
{
#   mkdir -p build
    CmakeFile="$2/ToolChain.cmake"
    echo "SET(CMAKE_SYSTEM_NAME \"Linux\")" > $CmakeFile
    if [ $1 == x86 ]; then
        echo "SET(CMAKE_C_COMPILER \"gcc\")" >> $CmakeFile  
        echo "SET(CMAKE_CXX_COMPILER \"g++\")" >> $CmakeFile    
    else
        echo "SET(CMAKE_C_COMPILER \"$1-gcc\")" >> $CmakeFile
        echo "SET(CMAKE_CXX_COMPILER \"$1-g++\")" >> $CmakeFile
    fi
}
function BuildLib()
{
    echo -e "Start building cJSON..."
    CurPwd=$PWD
    OutputPath="$PWD/cJSON-1.7.12/build"
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

    cd $OutputPath
    cmakeOption=-DBUILD_SHARED_AND_STATIC_LIBS=ON
    if [ $1 == x86 -o $1 == x64 ]; then
        cmakeOption=-DBUILD_SHARED_AND_STATIC_LIBS=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
    else
        cmakeOption=-DBUILD_SHARED_AND_STATIC_LIBS=ON -DCMAKE_C_COMPILER=$1-gcc -DCMAKE_CXX_COMPILER=$1-g++
    fi
    cmake $cmakeOption -DCMAKE_INSTALL_PREFIX=$OutputPath/install ..
    if [ -e "Makefile" ]; then  
        make clean
        make -j4;make install
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
    cd $CurPwd
}

function CopyLib()
{
#   CurPwd = $PWD
    CurPwd=$PWD
    cd $1
    if [ -e "json" ]; then
        echo "json exit"
    else
        mkdir json
    fi
    
    cd json
    if [ -e "cJSON" ]; then
        echo "cJSON exit"
    else
        mkdir cJSON
    fi
    
    cd cJSON
    cp $CurPwd/cJSON-1.7.12/build/install/lib64/libcjson.a .
    cp $CurPwd/cJSON-1.7.12/build/install/include/cjson/cJSON.h .
    cd $CurPwd
}

if [ $# == 0 ]; then
    PrintUsage
    exit -1
else
#   GenerateCmakeFile $1
    BuildLib $1
    CopyLib $2
fi




