cmake_minimum_required(VERSION 3.20)

project(RegisterMonitor_UIT VERSION 1.0.0.0)

# CMP0135 is only available starting with CMake 3.24.
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME}
    RegisterMonitor_UIT.cpp
)

target_link_libraries(${PROJECT_NAME}
    GTest::gtest_main
    RegisterMonitor_lib
)
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})