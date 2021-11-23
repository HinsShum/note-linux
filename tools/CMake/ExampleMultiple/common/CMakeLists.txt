# @file common/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2020/12/13 18:27:48
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

if(DEFINED BOARD_NAME)
    file(GLOB FILES "*.c")

    # set filter out files
    set(FILES_OUT "")
    filter_out(FILES_OUT FILES FILES)

    set(C_SOURCE_FILES ${C_SOURCE_FILES} ${FILES} PARENT_SCOPE)
    set(G_SOURCE_INCLUDE_DIRS ${G_SOURCE_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
endif()