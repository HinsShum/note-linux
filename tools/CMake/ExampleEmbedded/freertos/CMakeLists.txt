# @file /freertos/CMakeLists.txt
# @author HinsShum
# @date 2020-06-22
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

file(GLOB C_SOURCE_FILES "*.c")
set(C_PRIVATE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(CMAKELISTS_DIRS)
set(TARGET_LIB_NAME os)

#get sub directory
file(GLOB_RECURSE CMAKELISTS "CMakeLists.txt")
dirname(CMAKELISTS CMAKELISTS_DIRS)

# filter out current directory
filter_out(CMAKE_CURRENT_SOURCE_DIR CMAKELISTS_DIRS CMAKELISTS_DIRS)

# enter sub directory automatically to execute CMakeLists.txt
foreach(SUB_DIRECTORY ${CMAKELISTS_DIRS})
    add_subdirectory(${SUB_DIRECTORY})
endforeach(SUB_DIRECTORY)

# compile os
add_library(${TARGET_LIB_NAME} STATIC ${C_SOURCE_FILES})
set(C_SOURCE_LIBS "${C_SOURCE_LIBS};${TARGET_LIB_NAME}" PARENT_SCOPE)

# head file directory
target_include_directories(${TARGET_LIB_NAME} PRIVATE "${C_PRIVATE_INCLUDE_DIRS}" "${G_SOURCE_INCLUDE_DIRS}")