cmake_minimum_required(VERSION 3.24)
project(StatsigClientSDK)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files to the Library
file(GLOB STATSIG_SOURCES "src/*.cpp")
add_library(${PROJECT_NAME} SHARED ${STATSIG_SOURCES})

# Include the header files in the Library
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

# === Third Party Deps ===

include(FetchContent)

# HttpLib
include(cmake/httplib.cmake)
target_link_libraries(${PROJECT_NAME} PUBLIC httplib::httplib)
target_compile_definitions(${PROJECT_NAME} PRIVATE CPPHTTPLIB_ZLIB_SUPPORT)

# Zlib (Required for HttpLib)
find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ${ZLIB_LIBRARIES})

# Json
include(cmake/json.cmake)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
