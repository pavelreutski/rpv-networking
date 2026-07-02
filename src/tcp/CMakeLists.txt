cmake_minimum_required(VERSION 3.20.0)

project(rpv-net-tcp
        VERSION 0.0.1
        DESCRIPTION "rpv net tcp"
        LANGUAGES C)

file(GLOB C_SOURCES CONFIGURE_DEPENDS *.c)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE ${C_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE rpv-mem)
target_include_directories(${PROJECT_NAME} PUBLIC ./include)

target_compile_definitions(${PROJECT_NAME} PRIVATE _GNU_SOURCE)