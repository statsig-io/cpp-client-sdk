# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

FetchContent_Declare(googletest
        GIT_REPOSITORY    https://github.com/google/googletest.git
        GIT_TAG           v1.14.0
)

FetchContent_MakeAvailable(googletest)