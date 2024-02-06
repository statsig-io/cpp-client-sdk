cmake_minimum_required(VERSION 3.24)
project(StatsigClientSDK)

set(CMAKE_CXX_STANDARD 17)

# Define the library source files
file(GLOB STATSIG_SOURCES "src/*.cpp")

# Define the include directory
set(STATSIG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create a shared library target
add_library(${PROJECT_NAME} SHARED ${STATSIG_SOURCES})

# Specify the include directories for the library
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${STATSIG_INCLUDE_DIR}> libs)

