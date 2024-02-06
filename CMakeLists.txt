cmake_minimum_required(VERSION 3.24)
project(StatsigClientSDK)

set(CMAKE_CXX_STANDARD 17)

# Add source files to the Library
file(GLOB STATSIG_SOURCES "src/*.cpp")
add_library(${PROJECT_NAME} SHARED ${STATSIG_SOURCES})

# Add Third-Party dependencies
include(FetchContent)
include(cmake/json.cmake)
include(cmake/httplib.cmake)

target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json httplib::httplib)

# Include the header files in the Library
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

