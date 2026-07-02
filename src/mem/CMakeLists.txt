cmake_minimum_required(VERSION 3.20.0)

project(rpv-mem
        VERSION 0.0.1
        DESCRIPTION "rpv memory"
        LANGUAGES C)

file(GLOB C_SOURCES CONFIGURE_DEPENDS *.c)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE ${C_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ./include)