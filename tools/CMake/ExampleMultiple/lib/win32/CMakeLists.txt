# @file lib/win32/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2021/01/22 09:38:13
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

set(TARGET_LIB_NAME lib)
set(TARGET_C_FILES)
SET(TARGET_INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB FILES "*.c")
set(FILES_OUT "")
filter_out(FILES_OUT FILES TARGET_C_FILES)

add_library(${TARGET_LIB_NAME} STATIC ${TARGET_C_FILES})
target_include_directories(${TARGET_LIB_NAME} PRIVATE ${TARGET_INC_DIRS})

set(C_SOURCE_LIBS ${C_SOURCE_LIBS} ${TARGET_LIB_NAME} PARENT_SCOPE)
set(G_SOURCE_INCLUDE_DIRS ${G_SOURCE_INCLUDE_DIRS} ${TARGET_INC_DIRS} PARENT_SCOPE)
