cmake_minimum_required(VERSION 3.24)
project(StatsigClientSDKTests)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DSTATSIG_TESTS)

file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(${PROJECT_NAME} ${TEST_SOURCES})

# Add Statsig C++ Client
add_subdirectory(.. statsig_sdk)
target_link_libraries(${PROJECT_NAME} StatsigClientSDK)

# === Third Party Deps ===

include(FetchContent)

# Google Test
include(cmake/googletest.cmake)
target_link_libraries(${PROJECT_NAME} gtest gtest_main)
enable_testing()

