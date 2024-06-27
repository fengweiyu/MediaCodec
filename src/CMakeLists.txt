cmake_minimum_required (VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "build/ToolChain.cmake")
#SET(CMAKE_SYSTEM_NAME "Linux")
#SET(CMAKE_C_COMPILER "gcc")
#SET(CMAKE_CXX_COMPILER "g++")


project (MediaTranscode)
 
#SET(CMAKE_C_FLAGS "-g")
SET(CMAKE_CXX_FLAGS "-g -std=c++11 -pthread ")

#add_definitions(-DMEDIA_SEVER_TYPE_WEBRTC)
# 设置C++标准为C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(src_list
    MediaTranscodeInf.cpp
	MediaTranscode.cpp
    AudioTransform.cpp
    AudioDecode.cpp
    AudioRawHandle.cpp
    AudioEncode.cpp
    VideoTransform.cpp
    VideoDecode.cpp
    VideoRawHandle.cpp
    VideoEncode.cpp
	)		
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../lib/linux/x86/ffmpeg-7.0.1/include
	)
ADD_LIBRARY(MediaTranscode_static STATIC ${src_list})
SET_TARGET_PROPERTIES(MediaTranscode_static PROPERTIES OUTPUT_NAME MediaTranscode)


SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)





#TARGET_LINK_LIBRARIES(MediaTranscode pthread -lm)

#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










