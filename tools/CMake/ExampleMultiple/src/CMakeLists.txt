# @file src/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2021/01/24 11:05:29
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

file(GLOB C_FILES "*.c")
set(CMAKELISTS_DIRS)

# get sub directory
file(GLOB_RECURSE CMAKELISTS "CMakeLists.txt")
dirname(CMAKELISTS CMAKELISTS_DIRS)
filter_out(CMAKE_CURRENT_SOURCE_DIR CMAKELISTS_DIRS CMAKELISTS_DIRS)

foreach(SUB_DIRECTORY ${CMAKELISTS_DIRS})
    add_subdirectory(${SUB_DIRECTORY})
endforeach(SUB_DIRECTORY)

set(C_SOURCE_FILES ${C_SOURCE_FILES} ${C_FILES} PARENT_SCOPE)
set(G_SOURCE_INCLUDE_DIRS ${G_SOURCE_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
