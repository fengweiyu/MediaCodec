cmake_minimum_required (VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "build/ToolChain.cmake")
#SET(CMAKE_SYSTEM_NAME "Linux")
#SET(CMAKE_C_COMPILER "gcc")
#SET(CMAKE_CXX_COMPILER "g++")


project (MediaTranscode)
SET(USE_STATIC_LIB_LINKAGE ON)

#SET(CMAKE_C_FLAGS "-g")
SET(CMAKE_CXX_FLAGS "-g -std=c++11 ")
# 设置C++标准为C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(src_list
    main.cpp
    MediaTranscodeDemo.cpp
	)		
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/media/include
    ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/json/cJSON
    ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/lib/include
    ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/ffmpeg/include
	)
ADD_EXECUTABLE(MediaTranscode ${src_list})
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/media/lib/libmedia.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/json/cJSON/libcjson.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/lib/libMediaTranscode.a)

TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/ffmpeg/lib/libavfilter.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/ffmpeg/lib/libpostproc.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/ffmpeg/lib/libavformat.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/ffmpeg/lib/libavdevice.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/ffmpeg/lib/libavcodec.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/ffmpeg/lib/libavutil.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/ffmpeg/lib/libswresample.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/ffmpeg/lib/libswscale.a)

TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/x264/lib/libx264.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/x265/lib/libx265.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/fdk-aac/lib/libfdk-aac.a)

TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/libfontconfig/lib/libfontconfig.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/libxml2/lib/libxml2.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/harfbuzz/lib/libharfbuzz.a)
TARGET_LINK_LIBRARIES(MediaTranscode ${PROJECT_SOURCE_DIR}/../build/linux/${CMAKE_ToolChain}/ThirdLib/freetype/lib/libfreetype.a)

TARGET_LINK_LIBRARIES(MediaTranscode -pthread -ldl -lz -static)


#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)













