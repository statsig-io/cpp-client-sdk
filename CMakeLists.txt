cmake_minimum_required(VERSION 3.24)
project(Statsig)

set(CMAKE_CXX_STANDARD 17)

file(GLOB STATSIG_SOURCES src/*)
add_library(${PROJECT_NAME} ${STATSIG_SOURCES})

