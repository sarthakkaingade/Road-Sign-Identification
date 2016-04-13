cmake_minimum_required(VERSION 2.8.4)
PROJECT(road-sign-identification)
include(../dlib/dlib/cmake)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_BUILD_TYPE Release)

MACRO(add_source name)
    ADD_EXECUTABLE(${name} src/${name}.cpp)
    TARGET_LINK_LIBRARIES(${name} dlib ${OpenCV_LIBS} )
ENDMACRO()

add_source(hog_detector)
add_source(detect)
add_source(view_hog)

