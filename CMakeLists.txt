cmake_minimum_required(VERSION 3.22)
project(StatsigClientSDK)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files to the Library
file(GLOB_RECURSE STATSIG_SOURCES "src/**/*.cpp")


# Get Third Party Deps
include(FetchContent)
include(cmake/httplib.cmake)
include(cmake/json.cmake)

# Setup Targets

function(setup_target target_name target_type)
 add_library(${target_name} ${target_type} ${STATSIG_SOURCES})
 set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "StatsigClientSDK")

 # Include the header files in the Library
 target_include_directories(${target_name} PUBLIC ${PROJECT_SOURCE_DIR}/src)

 # SDK specific compiler options
 if (NOT WIN32)
  target_compile_options(${target_name} PRIVATE -Wswitch -Werror=switch)
 endif()

 # HttpLib
 target_link_libraries(${target_name} PUBLIC httplib::httplib)
 target_compile_definitions(${target_name} PRIVATE CPPHTTPLIB_ZLIB_SUPPORT)

 # Zlib (Required for HttpLib)
 find_package(ZLIB REQUIRED)
 target_link_libraries(${target_name} PUBLIC ${ZLIB_LIBRARIES})

 # Json
 target_link_libraries(${target_name} PUBLIC nlohmann_json::nlohmann_json)
endfunction()

# Set up the static and shared libraries
setup_target(${PROJECT_NAME}_static STATIC)
setup_target(${PROJECT_NAME}_shared SHARED)
