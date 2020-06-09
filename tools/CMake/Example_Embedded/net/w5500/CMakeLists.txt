# @file /net/w5500/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2020-07-16
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

set(CMAKELISTS_DIRS)
set(C_SOURCE_FILES)
set(TARGET_LIB_NAME w5500)

# get sub directory
file(GLOB_RECURSE CMAKELISTS "CMakeLists.txt")
foreach(CMAKELIST ${CMAKELISTS})
    execute_process(COMMAND dirname ${CMAKELIST} OUTPUT_VARIABLE CMAKELIST_DIR)
    # delete '\n'
    string(REGEX REPLACE "\n$" "" CMAKELIST_DIR "${CMAKELIST_DIR}")
    set(CMAKELISTS_DIRS ${CMAKELISTS_DIRS} ${CMAKELIST_DIR})
endforeach(CMAKELIST)

# filter out current directory
filter_out(CMAKE_CURRENT_SOURCE_DIR CMAKELISTS_DIRS CMAKELISTS_DIRS)

# enter sub directory automatically to execute CMakeLists.txt
foreach(SUB_DIRECTORY ${CMAKELISTS_DIRS})
    add_subdirectory(${SUB_DIRECTORY})
endforeach(SUB_DIRECTORY)

add_library(${TARGET_LIB_NAME} STATIC ${C_SOURCE_FILES})
set(C_SOURCE_LIBS "${C_SOURCE_LIBS};${TARGET_LIB_NAME}" PARENT_SCOPE)

target_include_directories(${TARGET_LIB_NAME} PRIVATE "${G_SOURCE_INCLUDE_DIRS}")