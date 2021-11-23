/**
 * @file config\board\options.h
 *
 * Copyright (C) 2021
 *
 * options.h is free software: you can redistribute it and/or modify
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
 *
 * @encoding utf-8
 */
#ifndef __CONFIG_BOARD_OPTIONS_H
#define __CONFIG_BOARD_OPTIONS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "version.h"
#include "gd32e23x.h"

/*---------- macro ----------*/
#undef assert
#ifdef NDEBUG
#define assert(expr)                    ((void)0U)
#define CONFIG_SILENT
#else
#define assert(expr)                    do { if(!(expr)) { for(;;); }} while(0)
#endif

/* format to string
 */
#define _STRING(x)                      #x              /*<< only format alphabet as string */
#define STRING(x)                       _STRING(x)      /*<< format alphabet or digit as string */

/* system variables defined
 */
#define SYS_VERSION                     ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_FIX << 8) | VERSION_BUILD)
#define SYS_VERSION_STRING              (STRING(VERSION_MAJOR)"."STRING(VERSION_MINOR)"."STRING(VERSION_FIX)"."STRING(VERSION_BUILD))
#define SYS_MODEL_NAME                  ("DS884")
#define SYS_HW_VERSION                  ((HW_VERSION_MAJOR << 8) | HW_VERSION_MINOR)
#define SYS_PRODUCT_TIME                (__DATE__)
#define SYS_VENDOR                      "HinsShum"

/* system delay function
 */
#define __delay_ms(ms)                  mdelay(ms)
#define __delay_us(us)                  udelay(us)
#define __get_ticks()                   tick_get()
#define MS2TICKS(ms)                    (ms)

/* print macros
 */
#ifndef CONFIG_SILENT
#define __debug_message(x, y...)        printf("\033[32;22m" x, ##y)
#define __debug_info(x, y...)           printf("\033[37;22m" x, ##y)
#define __debug_warn(x, y...)           printf("\033[31;22m" x, ##y)
#define __debug_error(x, y...)          printf("\033[31;22m" x, ##y)
#define __debug_cont(x, y...)           printf(x, ##y)
#else
#define __debug_message(x, y...)
#define __debug_info(x, y...)
#define __debug_warn(x, y...)
#define __debug_error(x, y...)
#define __debug_cont(x, y...)
#endif

/* print for app
 */
#define debug_message(x, y...)          printf("\033[32;22m" x, ##y)
#define debug_info(x, y...)             printf("\033[37;22m" x, ##y)
#define debug_warn(x, y...)             printf("\033[31;22m" x, ##y)
#define debug_error(x, y...)            printf("\033[31;22m" x, ##y)
#define debug_cont(x, y...)             printf(x, ##y)

/* SRAM information
 */
#define CONFIG_SRAM_BASE                (SRAM_BASE)
#define CONFIG_SRAM_SIZE                (8 * 1024)
#define CONFIG_SRAM_END                 (CONFIG_SRAM_BASE + CONFIG_SRAM_SIZE)

/* embed flash information
 */
#define CONFIG_EMBED_FLASH_BASE         (FLASH_BASE)
#define CONFIG_EMBED_FLASH_SIZE         (0x10000)       /*<< 64 * 1024 */
#define CONFIG_EMBED_FLASH_BLOCK_SIZE   (0x400)         /*<< 1 * 1024 */
#define CONFIG_EMBED_FLASH_END          (CONFIG_EMBED_FLASH_BASE + CONFIG_EMBED_FLASH_SIZE)
#define CONFIG_EMBED_FLASH_WRITE_GRAN   (32)

/* app location config in flash
 * the base addr is an offset from the flash base addr,
 * not the actual addr of the flash.
 */
#define CONFIG_APP_LOCATION_BASE        (16 * 1024)
#define CONFIG_APP_MAX_SIZE             (23 * 1024)

/* app backup location config in flash
 * the base addr is an offset from the flash base addr,
 * not the actual addr of the flash.
 */
#define CONFIG_APP_BK_INFO_LOCATION     (40 * 1024)
#define CONFIG_APP_BK_LOCATION_BASE     (CONFIG_APP_BK_INFO_LOCATION + CONFIG_EMBED_FLASH_BLOCK_SIZE)
#define CONFIG_APP_BK_MAX_SIZE          (CONFIG_APP_MAX_SIZE)
#define CONFIG_APP_BK_LOCATION_END      (CONFIG_APP_BK_LOCATION_BASE + CONFIG_APP_BK_MAX_SIZE)

/* fifo size config
 */
#define CONFIG_FIFO_SIZE                (1040)

/* recv buffer size
 */
#define CONFIG_RECV_BUFFER_SIZE         (1040)

/* the max time for wait space char
 */
#define CONFIG_YBOOT_WAIT_SPACE_TIME    (MS2TICKS(3000))

/* config interrupt priority
 * all bits for gourp priorities, no bit for sub priorities
 */
#define CONFIG_PRIORITY_MSK             ((1UL << __NVIC_PRIO_BITS) - 1UL)
#define CONFIG_PRIORITY_MAKE(priority)  (CONFIG_PRIORITY_MSK - (priority & CONFIG_PRIORITY_MSK))
#define CONFIG_PRIORITY_SYSTICK         NVIC_EncodePriority(__NVIC_GetPriorityGrouping(), CONFIG_PRIORITY_MAKE(0), 0)
#define CONFIG_PRIORITY_COM             NVIC_EncodePriority(__NVIC_GetPriorityGrouping(), CONFIG_PRIORITY_MAKE(1), 0)

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void udelay(uint32_t us);
extern void mdelay(uint32_t delay);
extern uint32_t tick_get(void);

#ifdef __cplusplus
}
#endif
#endif /* __CONFIG_BOARD_OPTIONS_H */