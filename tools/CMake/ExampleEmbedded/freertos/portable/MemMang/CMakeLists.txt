# @file /freertos/portable/MemMang/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2020-07-16
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

set(C_SOURCE_FILES "${C_SOURCE_FILES};${CMAKE_CURRENT_SOURCE_DIR}/heap_4.c" PARENT_SCOPE)