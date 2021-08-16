# @file /atlib/l620/CMakeLists.txt
# @author HinsShum
# @date 2020-07-07
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

# set source files to compile
file(GLOB FILES "*.c")

# set filter out files
set(FILES_OUT "${CMAKE_CURRENT_SOURCE_DIR}/_l620_at.c")
filter_out(FILES_OUT FILES FILES)

set(C_SOURCE_FILES "${C_SOURCE_FILES};${FILES}" PARENT_SCOPE)
set(C_PRIVATE_INCLUDE_DIRS ${C_PRIVATE_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)