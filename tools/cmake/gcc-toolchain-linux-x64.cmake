# gcc-toolchain-linux-x64.cmake
# ------------------------------------------------
# Toolchain file for building with GNU GCC

cmake_minimum_required(VERSION 3.20.0)

# Set system
set(CMAKE_SYSTEM Linux)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set compiler executables

find_program(CMAKE_C_COMPILER NAMES gcc-14 REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES g++-14 REQUIRED)

# Base compile flags for all configurations
set(CXX_FLAGS "-Wall -Wextra -Werror -std=c++23")
set(C_FLAGS "-Wall -Wextra -Werror -Wpedantic -fPIC -std=gnu23")

# Flags per configuration

set(CMAKE_CXX_FLAGS_DEBUG   "${CXX_FLAGS} -Og -g")

set(CMAKE_C_FLAGS_DEBUG     "${C_FLAGS} -Og -g")
set(CMAKE_C_FLAGS_RELEASE   "${C_FLAGS} -Os -DNDEBUG \
                                        -fno-asynchronous-unwind-tables \
                                        -fno-unwind-tables -fno-common \
                                        -fno-stack-protector -flto \
                                        -ffunction-sections -fdata-sections \
                                        -fomit-frame-pointer -fvisibility=hidden \
                                        -fmerge-all-constants -fmerge-constants -fno-ident")

# Optional: set linker flags

set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-s -flto")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "-s -flto -Wl,--gc-sections -Wl,-O2 -Wl,--as-needed")

# Optional: ensure position-independent code for shared libs
set(CMAKE_POSITION_INDEPENDENT_CODE ON)