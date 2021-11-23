/**
 * @file /arch/stm32f103xx/inc/stm32f1xx_ll_conf.h
 *
 * Copyright (C) 2020
 *
 * stm32f1xx_ll_conf.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author HinsShum hinsshum@qq.com
 */
#ifndef __STM32F1XX_LL_CONF_H
#define __STM32F1XX_LL_CONF_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Uncomment/Comment the line below to enable/disable peripheral header file inclusion */
//#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_cortex.h"
//#include "stm32f1xx_ll_crc.h"
//#include "stm32f1xx_ll_dac.h"
//#include "stm32f1xx_ll_dma.h"
//#include "stm32f1xx_ll_exti.h"
//#include "stm32f1xx_ll_fsmc.h"
#include "stm32f1xx_ll_gpio.h"
//#include "stm32f1xx_ll_i2c.h"
//#include "stm32f1xx_ll_iwdg.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_rcc.h"
//#include "stm32f1xx_ll_rtc.h"
//#include "stm32f1xx_ll_sdmmc.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_system.h"
//#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_usart.h"
//#include "stm32f1xx_ll_usb.h"
#include "stm32f1xx_ll_utils.h"
//#include "stm32f1xx_ll_wwdg.h"

/*---------- macro ----------*/
#ifdef USE_FULL_ASSERT
#include "stm32_assert.h"
#else
#define assert_param(expr)  ((void)0U)
#endif

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
#endif /* __STM32F1XX_LL_CONF_H */
