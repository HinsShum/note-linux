# @file /lib/siliconlabs/efm32lgxx/emlib/src/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2020-07-16
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

set(C_SOURCE_FILES "em_adc.c;em_cmu.c;em_emu.c;em_gpio.c;em_letimer.c;em_leuart.c;em_msc.c;em_system.c;em_usart.c")
set(TARGET_LIB_NAME emlib)

add_library(${TARGET_LIB_NAME} STATIC ${C_SOURCE_FILES})
set(C_SOURCE_LIBS "${C_SOURCE_LIBS};${TARGET_LIB_NAME}" PARENT_SCOPE)

target_include_directories(${TARGET_LIB_NAME} PRIVATE ${G_SOURCE_INCLUDE_DIRS})