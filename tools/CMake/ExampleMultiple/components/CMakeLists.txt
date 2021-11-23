# @file components/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2021/01/25 08:43:34
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

set(TARGET_LIB_NAME components)
set(CMAKELISTS_DIRS)
set(TARGET_C_FILES)
set(TARGET_INC_DIRS)

#get sub directory
file(GLOB_RECURSE CMAKELISTS "CMakeLists.txt")
dirname(CMAKELISTS CMAKELISTS_DIRS)
filter_out(CMAKE_CURRENT_SOURCE_DIR CMAKELISTS_DIRS CMAKELISTS_DIRS)

# enter sub directory automatically to execute CMakeLists.txt
foreach(SUB_DIRECTORY ${CMAKELISTS_DIRS})
    add_subdirectory(${SUB_DIRECTORY})
endforeach(SUB_DIRECTORY)

add_library(${TARGET_LIB_NAME} STATIC ${TARGET_C_FILES})
target_include_directories(${TARGET_LIB_NAME} PRIVATE ${TARGET_INC_DIRS} ${G_SOURCE_INCLUDE_DIRS})

set(C_SOURCE_LIBS ${C_SOURCE_LIBS} ${TARGET_LIB_NAME} PARENT_SCOPE)
set(G_SOURCE_INCLUDE_DIRS ${G_SOURCE_INCLUDE_DIRS} ${TARGET_INC_DIRS} PARENT_SCOPE)
