cmake_minimum_required(VERSION 3.20.0)

project(rpv-echo-server
        VERSION 0.0.1
        DESCRIPTION "rpv echo server"
        LANGUAGES C)

set(EXECUTABLE ${PROJECT_NAME})
file(GLOB C_SOURCES CONFIGURE_DEPENDS *.c)

add_executable(${EXECUTABLE} ${C_SOURCES})

target_link_libraries(${EXECUTABLE} PRIVATE rpv-net-tcp)
target_include_directories(${EXECUTABLE} PRIVATE ./include)

add_custom_target(run-echo-server DEPENDS ${EXECUTABLE}
        COMMENT "running echo server..."
        COMMAND $<TARGET_FILE:${EXECUTABLE}> 3434)