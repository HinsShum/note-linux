# @file components/cymodem/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2021/01/25 08:49:22
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

if(DEFINED BOARD_NAME)
    file(GLOB FILES "*.c")

    # set filter out files
    set(FILES_OUT "")
    filter_out(FILES_OUT FILES FILES)

    set(TARGET_C_FILES ${TARGET_C_FILES} ${FILES} PARENT_SCOPE)
    set(TARGET_INC_DIRS ${TARGET_INC_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/inc" PARENT_SCOPE)
endif()
