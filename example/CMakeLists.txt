cmake_minimum_required(VERSION 3.24)
project(StatsigClientSDKExample)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} example.cpp)

# Add Statsig C++ Client from local
add_subdirectory(.. statsig_sdk)
target_link_libraries(${PROJECT_NAME} StatsigClientSDK)


# or, uncomment the below to add the SDK from github
#include(FetchContent)
#include(cmake/statsig.cmake)
#target_link_libraries(${PROJECT_NAME} StatsigClientSDK)