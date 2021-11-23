# @file driver/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2021/01/22 11:13:07
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

include("${PROJECT_SOURCE_DIR}/config/${BOARD_NAME}/driver.cmake")

set(CFILES "${CMAKE_CURRENT_SOURCE_DIR}/device.c" "${CMAKE_CURRENT_SOURCE_DIR}/driver.c")

if(DEFINED AT24CXX)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/at24cxx.c")
endif()

if(DEFINED BUZZER)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/buzzer.c")
endif()

if(DEFINED D100)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/d100.c")
endif()

if(DEFINED FLASH)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/flash.c")
endif()

if(DEFINED FM13DT160)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/fm13dt160.c")
endif()

if(DEFINED FM25QXX)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/fm25qxx.c")
endif()

if(DEFINED GPIO)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/gpio.c")
endif()

if(DEFINED I2C_BUS)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/i2c_bus.c")
endif()

if(DEFINED INFRARED)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/infrared.c")
endif()

if(DEFINED LED)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/led.c")
endif()

if(DEFINED YT8512X)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/motorcomm_yt8512x.c")
endif()

if(DEFINED MS523)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/ms523.c")
endif()

if(DEFINED MS8837)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/ms8837.c")
endif()

if(DEFINED RTC)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/rtc.c")
endif()

if(DEFINED SEPS525)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/seps525.c")
endif()

if(DEFINED SERIAL)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/serial.c")
endif()

if(DEFINED SILKID)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/silkid.c")
endif()

if(DEFINED TIMER)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/timer.c")
endif()

if(DEFINED WDT)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/wdt.c")
endif()

if(DEFINED ZAZXXX)
    set(CFILES ${CFILES} "${CMAKE_CURRENT_SOURCE_DIR}/zazxxx.c")
endif()

set(C_SOURCE_FILES ${C_SOURCE_FILES} ${CFILES} PARENT_SCOPE)
set(G_SOURCE_INCLUDE_DIRS ${G_SOURCE_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
