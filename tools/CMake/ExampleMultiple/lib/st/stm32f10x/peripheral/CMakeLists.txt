# @file lib/st/stm32f10x/peripheral/Src/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2021/11/22 16:32:29
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

if(DEFINED BOARD_NAME)
    include("${PROJECT_SOURCE_DIR}/config/${BOARD_NAME}/stlib.cmake")
    set(TARGET_LIB_NAME stlib)
    set(TARGET_C_FILES)
    set(TARGET_INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/Inc")

    # set compile files
    if(DEFINED HAL_ADC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_adc.c")
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_adc_ex.c")
    endif()

    if(DEFINED HAL_CAN)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_can.c")
    endif()

    if(DEFINED HAL_CEC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_cec.c")
    endif()

    if(DEFINED HAL_CORTEX)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_cortex.c")
    endif()

    if(DEFINED HAL_CRC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_crc.c")
    endif()

    if(DEFINED HAL_DAC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_dac.c")
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_dac_ex.c")
    endif()

    if(DEFINED HAL_DMA)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_dma.c")
    endif()

    if(DEFINED HAL_ETH)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_eth.c")
    endif()

    if(DEFINED HAL_EXTI)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_exti.c")
    endif()

    if(DEFINED HAL_FLASH)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_flash.c")
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_flash_ex.c")
    endif()

    if(DEFINED HAL_GPIO)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_gpio.c")
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_gpio_ex.c")
    endif()

    if(DEFINED HAL_HCD)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_hcd.c")
    endif()

    if(DEFINED HAL_I2C)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_i2c.c")
    endif()

    if(DEFINED HAL_I2S)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_i2s.c")
    endif()

    if(DEFINED HAL_IRDA)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_irda.c")
    endif()

    if(DEFINED HAL_IWDG)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_iwdg.c")
    endif()

    if(DEFINED HAL_MMC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_mmc.c")
    endif()

    if(DEFINED HAL_MSP)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_msp_template.c")
    endif()

    if(DEFINED HAL_NAND)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_nand.c")
    endif()

    if(DEFINED HAL_NOR)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_nor.c")
    endif()

    if(DEFINED HAL_PCCARD)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_pccard.c")
    endif()

    if(DEFINED HAL_PCD)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_pcd.c")
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_pcd_ex.c")
    endif()

    if(DEFINED HAL_PWR)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_pwr.c")
    endif()

    if(DEFINED HAL_RCC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_rcc.c")
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_rcc_ex.c")
    endif()

    if(DEFINED HAL_RTC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_rtc.c")
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_rtc_ex.c")
    endif()

    if(DEFINED HAL_SD)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_sd.c")
    endif()

    if(DEFINED HAL_SMARTCARD)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_smartcard.c")
    endif()

    if(DEFINED HAL_SPI)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_spi.c")
    endif()

    if(DEFINED HAL_SRAM)
      set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_sram.c")
    endif()

    if(DEFINED HAL_TIME)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_tim.c")
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_tim_ex.c")
    endif()

    if(DEFINED HAL_TIMEBASE_RTC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_timebase_rtc_alarm_template.c")
    endif()

    if(DEFINED HAL_TIMEBASE_TIM)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_timbase_tim_template.c")
    endif()

    if(DEFINED HAL_UART)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_uart.c")
    endif()

    if(DEFINED HAL_USART)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_usart.c")
    endif()

    if(DEFINED HAL_WWDG)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal_wwdg.c")
    endif()

    if(DEFINED HAL)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_hal.c")
    endif()

    if(DEFINED LL_ADC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_adc.c")
    endif()

    if(DEFINED LL_CRC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_crc.c")
    endif()

    if(DEFINED LL_DAC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_dac.c")
    endif()

    if(DEFINED LL_DMA)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_dma.c")
    endif()

    if(DEFINED LL_EXTI)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_exti.c")
    endif()

    if(DEFINED LL_FSMC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_fsmc.c")
    endif()

    if(DEFINED LL_GPIO)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_gpio.c")
    endif()

    if(DEFINED LL_I2C)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_i2c.c")
    endif()

    if(DEFINED LL_PWR)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_pwr.c")
    endif()

    if(DEFINED LL_RCC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_rcc.c")
    endif()

    if(DEFINED LL_RTC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_rtc.c")
    endif()

    if(DEFINED LL_SDMMC)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_sdmmc.c")
    endif()

    if(DEFINED LL_SPI)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_spi.c")
    endif()

    if(DEFINED LL_TIME)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_tim.c")
    endif()

    if(DEFINED LL_USART)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_usart.c")
    endif()

    if(DEFINED LL_USB)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_usb.c")
    endif()

    if(DEFINED LL_UTILS)
        set(TARGET_C_FILES ${TARGET_C_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f1xx_ll_utils.c")
    endif()

    add_library(${TARGET_LIB_NAME} STATIC ${TARGET_C_FILES})
    target_include_directories(${TARGET_LIB_NAME} PRIVATE ${TARGET_INC_DIRS} ${G_SOURCE_INCLUDE_DIRS})

    set(C_SOURCE_LIBS ${C_SOURCE_LIBS} ${TARGET_LIB_NAME} PARENT_SCOPE)
    set(G_SOURCE_INCLUDE_DIRS ${G_SOURCE_INCLUDE_DIRS} ${TARGET_INC_DIRS} PARENT_SCOPE)
endif()