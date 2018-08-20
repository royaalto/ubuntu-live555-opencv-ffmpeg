#CMake versin
cmake_minimum_required(VERSION 3.1)

#project information
set(DRIVER_NAME openRtsp)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_BUILD_TYPE Release)

## Add support for C++11, supported in ROS Kinetic and newer
set(DRIVER_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DRIVER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)
message(STATUS "name: ${_NAME}")
message(STATUS "${DRIVER_INCLUDE}")


set(FFMPEG_INCLUDE_DIRS
    "/usr/local/lib"
    # "/home/roy/softwares/FFmpeg/libavcodec"
    # "/home/roy/softwares/FFmpeg/libavformat"
    # "/home/roy/softwares/FFmpeg/libavutil"
    # "/home/roy/softwares/FFmpeg/libswscale"
    "/usr/local/include"
    "/usr/include"
    "/usr/include/include"
    )

set(LIVE555_INCLUDE_DIRS
    "/home/roy/softwares/live555/liveMedia/include"
    "/home/roy/softwares/live555/groupsock/include"
    "/home/roy/softwares/live555/UsageEnvironment/include"
    "/home/roy/softwares/live555/BasicUsageEnvironment/include"
    )
message(STATUS "${LIVE555_INCLUDE_DIRS}")
set(LIVE555_LIBS
	/usr/local/lib/libliveMedia.a
	/usr/local/lib/libgroupsock.a
	/usr/local/lib/libBasicUsageEnvironment.a    
    /usr/local/lib/libUsageEnvironment.a
    	
    #/usr/lib/x86_64-linux-gnu/
)

set(FFMPEG_LIBS
	#/usr/local/lib/libavformat.a    
	#/usr/local/lib/libavcodec.a
    #/usr/local/lib/libavutil.a
    #/usr/local/lib/libswscale.a
    #/usr/local/lib/libswresample.a

    # /usr/lib/x86_64-linux-gnu/libavcodec.a
    # /usr/lib/x86_64-linux-gnu/libavutil.a
    # /usr/lib/x86_64-linux-gnu/libavformat.a
    # /usr/lib/x86_64-linux-gnu/libswscale.a
    
    #DYNAMIC   
    /usr/lib/x86_64-linux-gnu/libavformat.so
    /usr/lib/x86_64-linux-gnu/libavcodec.so
    /usr/lib/x86_64-linux-gnu/libavutil.so
    /usr/lib/x86_64-linux-gnu/libswscale.so
    /usr/lib/x86_64-linux-gnu/libswresample.so
)


#opencv
find_package(OpenCV REQUIRED)


#include folder
include_directories(
${DRIVER_INCLUDE}
${JPEG_INCLUDE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/include 
${CMAKE_CURRENT_SOURCE_DIR}/lib
${LIVE555_INCLUDE_DIRS}
${OPENCV_INCLUDE_DIRS}
${FFMPEG_INCLUDE_DIRS}
${PROJECT_INCLUDE_DIRS}
)



#
add_executable(${DRIVER_NAME} src/main.cpp  src/ffmpeg_h264.cpp src/rtsp_client.cpp)
#add_executable(openRtsp src/main.cpp src/rtsp_client.cpp)

#add_executable(openRtsp src/main.cpp)
#link to the lib
target_link_libraries(${DRIVER_NAME}
	${PROJECT_LIBS} 
	${Boost_LIBRARIES}       
	${LIVE555_LIBS}
        ${OPENCV_LIBS}
        ${FFMPEG_LIBS}
	jpegencoder
	jpegdecoder
	${JPEG_LIBRARY}
	${JPEG_LIBRARIES}

        opencv_core opencv_highgui
        -lm
        -lz
        -llzma
        -lbz2
        #-lpthread
)
