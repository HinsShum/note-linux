/**
 * @file src\include\platform.h
 *
 * Copyright (C) 2021
 *
 * platform.h is free software: you can redistribute it and/or modify
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
#ifndef __PLATFORM_H
#define __PLATFORM_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
typedef struct {
    struct {
        void *embed_flash;              /*<< embed flash */
        void *backup_flash;             /*<< firmware backup flash */
        void *com;                      /*<< com for receive and send serial data */
        void *fifo;                     /*<< fifo for receive serial data */
        void *wdt;                      /*<< watch dog */
    } dev;
} platform_describe_t;

/*---------- variable prototype ----------*/
extern platform_describe_t g_plat;

/*---------- function prototype ----------*/
extern int32_t plat_init(void);
extern void plat_wdt_feed(void);

#ifdef __cplusplus
}
#endif
#endif /* __PLATFORM_H */
