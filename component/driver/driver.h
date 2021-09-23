/**
 * @file driver/include/driver.h
 *
 * Copyright (C) 2021
 *
 * driver.h is free software: you can redistribute it and/or modify
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
#ifndef __DRIVER_H
#define __DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "config/misc.h"

/*---------- macro ----------*/
#undef _DEV_SECTION_PREFIX
#if defined(__linux__) || defined(_WIN32)
#define _DEV_SECTION_PREFIX
#else
#define _DEV_SECTION_PREFIX                     "."
#endif

#define DRIVER_DEFINED(name, open, close, write, read, ioctl, irq_handler) \
        driver_t driver_##name __attribute__((used, section(_DEV_SECTION_PREFIX "drv_defined"))) \
        __attribute__((aligned(4))) = {#name, open, close, write, read, ioctl, irq_handler}

/*---------- type define ----------*/
typedef struct st_driver {
    char drv_name[10];
    int32_t (*open)(struct st_driver **drv);
    void (*close)(struct st_driver **drv);
    int32_t (*write)(struct st_driver **drv, void *buf, uint32_t addition, uint32_t len);
    int32_t (*read)(struct st_driver **drv, void *buf, uint32_t addition, uint32_t len);
    int32_t (*ioctl)(struct st_driver **drv, uint32_t cmd, void *args);
    int32_t (*irq_handler)(struct st_driver **drv, uint32_t irq_handler, void *args, uint32_t len);
} driver_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern int32_t driver_search_device(void);

#ifdef __cplusplus
}
#endif
#endif /* __DRIVER_H */
