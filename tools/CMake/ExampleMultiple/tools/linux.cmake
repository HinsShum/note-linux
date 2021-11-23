# @file tools\linux.cmake
# @author HinsShum hinsshum@qq.com
# @date 2021/11/11 13:19:48
# @encoding utf-8
# @brief Here's the first line of every win32.cmake,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

set(C_SOURCE_LIBS ${C_SOURCE_LIBS} "pthread")
add_definitions(-DYMODEM_CONFIG_FILE="config/${BOARD_NAME}/ymodem_config.h")

add_subdirectory(bsp/linux)
add_subdirectory(lib/linux)

set(CMAKE_C_FLAGS "-Wall -Werror -Wformat=0 -std=gnu99 -ffunction-sections -fdata-sections")