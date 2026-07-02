cmake_minimum_required(VERSION 3.20.0)

project(rpv-http-server
        VERSION 0.0.1
        DESCRIPTION "rpv http server"
        LANGUAGES C)

set(EXECUTABLE ${PROJECT_NAME})
file(GLOB C_SOURCES CONFIGURE_DEPENDS *.c)

add_executable(${EXECUTABLE} ${C_SOURCES})

target_include_directories(${EXECUTABLE} PRIVATE ./include)
target_link_libraries(${EXECUTABLE} PRIVATE rpv-mem rpv-net-tcp)

add_custom_target(run-http-server DEPENDS ${EXECUTABLE}
        COMMENT "running http server..."
        COMMAND $<TARGET_FILE:${EXECUTABLE}> 3434)