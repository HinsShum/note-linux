/**
 * @file driver\include\timer.h
 *
 * Copyright (C) 2021
 *
 * timer.h is free software: you can redistribute it and/or modify
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
#ifndef __DRIVER_TIMER_H
#define __DRIVER_TIMER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"

/*---------- macro ----------*/
#define IOCTL_TIMER_GET_INTERVAL                    (IOCTL_USER_START + 0x00)
#define IOCTL_TIMER_SET_INTERVAL                    (IOCTL_USER_START + 0x01)
#define IOCTL_TIMER_SET_IRQ_HANDLER                 (IOCTL_USER_START + 0x02)
#define IOCTL_TIMER_ENABLE                          (IOCTL_USER_START + 0x03)
#define IOCTL_TIMER_DISABLE                         (IOCTL_USER_START + 0x04)

/*---------- type define ----------*/
typedef int32_t (*timer_irq_handler_fn)(uint32_t irq_handler, void *args, uint32_t len);

typedef struct {
    uint32_t interval_ms;
    bool (*init)(void);
    void (*deinit)(void);
    bool (*enable)(bool ctrl);
    timer_irq_handler_fn irq_handler;
} timer_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __DRIVER_TIMER_H */
