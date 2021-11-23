# @file src/linux/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2021/01/06 14:44:47
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

if("${BOARD_NAME}" STREQUAL "linux")
    file(GLOB_RECURSE FILES "*.c")

    # set filter out files
    set(FILES_OUT "")
    filter_out(FILES_OUT FILES FILES)

    set(C_SOURCE_FILES ${C_SOURCE_FILES} ${FILES} PARENT_SCOPE)
    set(G_SOURCE_INCLUDE_DIRS ${G_SOURCE_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
endif()