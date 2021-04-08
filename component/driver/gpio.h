/**
 * @file APP/driver/include/gpio.h
 *
 * Copyright (C) 2021
 *
 * gpio.h is free software: you can redistribute it and/or modify
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
#ifndef __DRIVER_GPIO_H
#define __DRIVER_GPIO_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"

/*---------- macro ----------*/
#define IOCTL_GPIO_GET                              (IOCTL_USER_START + 0x00)
#define IOCTL_GPIO_SET                              (IOCTL_USER_START + 0x01)
#define IOCTL_GPIO_TOGGLE                           (IOCTL_USER_START + 0x02)

/*---------- type define ----------*/
typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    bool (*get)(void);
    void (*set)(bool val);
    void (*toggle)(void);
} gpio_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
#endif /* __DRIVER_GPIO_H */
