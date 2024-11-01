cmake_minimum_required(VERSION 2.8)

set (CMAKE_CXX_STANDARD 11)
project(opencv)
add_executable(${PROJECT_NAME} src/moyac.cpp)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})