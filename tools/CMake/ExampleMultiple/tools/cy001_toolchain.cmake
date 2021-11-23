# @file cy001_toolchain.cmake
# @author HinsShum hinsshum@qq.com
# @date 2021-11-22
# @brief usage -DCMAKE_TOOLCHAIN_FILE="pathname"
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR Arm)

# set dir variables
if(NOT DEFINED CROSS_COMPILER_DIR)
    set(CROSS_COMPILER_DIR "")
endif()

# set compiler
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY") # to avoid error when cmake -B build
set(CMAKE_C_COMPILER "${CROSS_COMPILER_DIR}arm-none-eabi-gcc")
set(CMAKE_OBJCOPY "${CROSS_COMPILER_DIR}arm-none-eabi-objcopy")
set(CMAKE_SIZE "${CROSS_COMPILER_DIR}arm-none-eabi-size")

# set definitions
add_definitions(-DSTM32F103xE -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DHSE_VALUE=8000000)

# set compiler flags
set(MCU_FLAGS "-mcpu=cortex-m3 -mthumb")
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Wall -Werror -Wformat=0 -std=gnu99 -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O1 -DNDEBUG")
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# set linker flags
set(LINK_SCRIPT_FLAGS "-Wl,--gc-sections,-T${PROJECT_SOURCE_DIR}/config/${BOARD_NAME}/gcc/boot_${BOARD_NAME}.ld")
set(LINK_MAP_FLAGS "-Wl,-Map,${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map,--cref")
set(LINK_MISC_FLAGS "-fno-exceptions -nostartfiles --specs=nosys.specs --specs=nano.specs")
set(CMAKE_EXE_LINKER_FLAGS "${LINK_MISC_FLAGS} ${LINK_SCRIPT_FLAGS} ${LINK_MAP_FLAGS}")