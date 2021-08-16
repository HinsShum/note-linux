# @file /arch/efm32lgxx/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2020-07-16
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

file(GLOB C_SOURCE_FILES "*.c")
set(C_SOURCE_FILES ${C_SOURCE_FILES} "startup_efm32lg.S")
set(TARGET_LIB_NAME "arch")
set(TARGET_LIB_LINK "emlib")

set_property(SOURCE startup_efm32lg.S PROPERTY LANGUAGE C)

add_library(${TARGET_LIB_NAME} STATIC ${C_SOURCE_FILES})
set(C_SOURCE_LIBS "${C_SOURCE_LIBS};${TARGET_LIB_NAME}" PARENT_SCOPE)

target_link_libraries(${TARGET_LIB_NAME} PUBLIC ${TARGET_LIB_LINK})
target_include_directories(${TARGET_LIB_NAME} PRIVATE "${G_SOURCE_INCLUDE_DIRS}")