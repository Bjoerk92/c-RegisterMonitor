cmake_minimum_required(VERSION 3.20)

project(RegisterMonitor VERSION 1.0.0.0)

enable_testing()

add_library(${PROJECT_NAME}_lib
    src/RegisterMonitor.c
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)


add_subdirectory(UnitTest)