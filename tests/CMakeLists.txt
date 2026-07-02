cmake_minimum_required(VERSION 3.20.0)

project(rpv-networking-tests
        LANGUAGES C CXX
        DESCRIPTION "rpv networking tests")

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

add_executable(http_server-tests http_server_tests.cpp)

target_link_libraries(http_server-tests PRIVATE rpv-mem rpv-net-tcp GTest::gtest_main)
target_include_directories(http_server-tests PRIVATE ${CMAKE_SOURCE_DIR}/src/tcp)

include(GoogleTest)

gtest_discover_tests(http_server-tests)