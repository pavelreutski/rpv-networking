cmake_minimum_required(VERSION 3.20.0)

project(rpv-networking-toolset
        LANGUAGES NONE
        DESCRIPTION "rpv networking toolset")

# set/add solution cmake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/tools/cmake)

# set custom output directories for binaries and libraries

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# include modules

include(FetchContent)
include(vscode-intellisense-update)

# add projects

add_subdirectory(./src/mem)
add_subdirectory(./src/tcp)

option(BUILD_TESTS "Build unit tests" OFF)

if (BUILD_TESTS)
        enable_testing()
        add_subdirectory(./tests)
else()
        add_subdirectory(./src/echo_server)
        add_subdirectory(./src/http_server)
endif()