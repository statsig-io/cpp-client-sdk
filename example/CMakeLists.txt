cmake_minimum_required(VERSION 3.24)
project(StatsigExample)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp)

include_directories(../src)
include_directories(../libs)

target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/../cmake-build-debug/libStatsig.a)

