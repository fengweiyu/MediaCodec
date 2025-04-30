cmake_minimum_required(VERSION 3.10) 


project (MediaCodec)
 
#set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/ThirdLib/AudioCodec) #CMAKE_INSTALL_PREFIX 外面设置，内部设置无效

# 设置C++标准为C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("PROJECT_SOURCE_DIR is: ${PROJECT_SOURCE_DIR} ${CMAKE_INSTALL_PREFIX}")





# 收集所有源文件  
file(GLOB src_list 
    src/MediaTranscodeInf.cpp
    src/MediaTranscode.cpp
    src/AudioTransform.cpp
    src/AudioDecode.cpp
    src/AudioRawHandle.cpp
    src/AudioEncode.cpp
    src/VideoTransform.cpp
    src/VideoDecode.cpp
    src/VideoRawHandle.cpp
    src/VideoEncode.cpp
    )  # 可以根据你的目录结构调整 

#set(src_list
#    src/xxx.cpp
#)


# 定义库  依赖ffmpeg头文件CODEC_FFMPEG_INC_PATH
add_library(MediaCodec ${src_list}) 

target_include_directories(MediaCodec PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${CODEC_FFMPEG_INC_PATH}
)



# 安装库文件  
install(TARGETS MediaCodec  
        DESTINATION lib)  # 库文件的安装目录  

# 安装头文件  
file(GLOB PUBLIC_HEADERS "${PROJECT_SOURCE_DIR}/src/include/*.h")
install(FILES ${PUBLIC_HEADERS}  
        DESTINATION include)  # 头文件的安装目录


#SET_TARGET_PROPERTIES(base64_static PROPERTIES OUTPUT_NAME base64)
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

#TARGET_LINK_LIBRARIES(webrtc pthread -lm)

#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










