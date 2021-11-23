# @file arch/stm32f10x/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2021/11/22 16:46:50
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

file(GLOB C_FILES "*.c")
set(CMAKELISTS_DIRS)
set(ARCH_INC_DIRS ${ARCH_INC_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include")

#get sub directory
file(GLOB_RECURSE CMAKELISTS "CMakeLists.txt")
dirname(CMAKELISTS CMAKELISTS_DIRS)

# filter out current directory
filter_out(CMAKE_CURRENT_SOURCE_DIR CMAKELISTS_DIRS CMAKELISTS_DIRS)

# enter sub directory automatically to execute CMakeLists.txt
foreach(SUB_DIRECTORY ${CMAKELISTS_DIRS})
    add_subdirectory(${SUB_DIRECTORY})
endforeach(SUB_DIRECTORY)

# add c files
set(C_SOURCE_FILES ${C_SOURCE_FILES} ${C_FILES} PARENT_SCOPE)
set(G_SOURCE_INCLUDE_DIRS ${G_SOURCE_INCLUDE_DIRS} ${ARCH_INC_DIRS} PARENT_SCOPE)
