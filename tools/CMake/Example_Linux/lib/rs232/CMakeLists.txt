# @file /lib/rs232/CMakeLists.txt
# @author HinsShum
# @date 2020-06-22
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

# add c source files to compile
set(C_SOURCE_FILES "rs232.c")
set(TARGET_LIB_NAME rs232)

# compile rs232 library
add_library(${TARGET_LIB_NAME} STATIC ${C_SOURCE_FILES})
set(C_SOURCE_LIBS "${C_SOURCE_LIBS};${TARGET_LIB_NAME}" PARENT_SCOPE)