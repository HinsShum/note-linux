# @file /toolchain_cortexm3.cmake
# @author HinsShum hinsshum@qq.com
# @date 2020-07-08
# @brief usage -DCMAKE_TOOLCHAIN_FILE="pathname"
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR Arm)

# set dir variables
set(CROSS_COMPILER_DIR "")

# set compiler
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY") # to avoid error when cmake -B build
set(CMAKE_C_COMPILER "${CROSS_COMPILER_DIR}arm-none-eabi-gcc")
set(CMAKE_OBJCOPY "${CROSS_COMPILER_DIR}arm-none-eabi-objcopy")
set(CMAKE_SIZE "${CROSS_COMPILER_DIR}arm-none-eabi-size")

# set definitions
add_definitions(-DEFM32LG280F256 -DEFM32_HFXO_FREQ=16000000 -DCONFIG_PRINTK)

# set compiler flags
set(MCU_FLAGS "-mcpu=cortex-m3 -mthumb")
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Wall -Werror -std=c99 -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_BUILD_TYPE "Debug")

# set linker flags
set(LINK_SCRIPT_FLAGS "-Wl,--gc-sections,-T${PROJECT_SOURCE_DIR}/config/linker.ld")
set(LINK_MAP_FLAGS "-Wl,-Map,${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map,--cref")
set(LINK_MISC_FLAGS "-fno-exceptions -nostartfiles --specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS "${LINK_MISC_FLAGS} ${LINK_SCRIPT_FLAGS} ${LINK_MAP_FLAGS}")
