cmake_minimum_required(VERSION 3.24)

project(registerMON)

add_library(${PROJECT_NAME}_lib
    src/RegisterMonitor.c
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
