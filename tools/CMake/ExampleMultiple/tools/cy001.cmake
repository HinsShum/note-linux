# @file tools/cy001.cmake
# @author HinsShum hinsshum@qq.com
# @date 2021/11/22 15:15:08
# @encoding utf-8
# @brief Here's the first line of every cy001.cmake,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

add_definitions(-DYMODEM_CONFIG_FILE="config/${BOARD_NAME}/ymodem_config.h")

add_subdirectory(arch/stm32f10x)
add_subdirectory(bsp/cy001)
add_subdirectory(lib/st/stm32f10x/peripheral)
