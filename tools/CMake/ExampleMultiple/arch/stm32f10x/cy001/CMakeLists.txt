# @file arch/stm32f10x/cy001/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2021/11/22 16:47:52
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

if("${BOARD_NAME}" STREQUAL "cy001")
    file(GLOB FILES "*.c")
    file(GLOB ASM_FILES "${CMAKE_CURRENT_SOURCE_DIR}/gcc/*.s")

    # set filter out files
    set(FILES_OUT "")
    filter_out(FILES_OUT FILES FILES)

    set(C_SOURCE_FILES ${C_SOURCE_FILES} ${FILES} ${ASM_FILES} PARENT_SCOPE)
    set(ARCH_INC_DIRS ${ARCH_INC_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
endif()