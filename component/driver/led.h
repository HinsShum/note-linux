/**
 * @file /driver/inc/led.h
 *
 * Copyright (C) 2020
 *
 * led.h is free software: you can redistribute it and/or modify
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
#ifndef __LED_H
#define __LED_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"

/*---------- macro ----------*/
#define IOCTL_LED_ON                        (IOCTL_USER_START + 0x00)
#define IOCTL_LED_OFF                       (IOCTL_USER_START + 0x01)
#define IOCTL_LED_TOGGLE                    (IOCTL_USER_START + 0x02)
#define IOCTL_LED_SET_BLINK_TIME            (IOCTL_USER_START + 0x03)
#define IOCTL_LED_GET_BLINK_TIME            (IOCTL_USER_START + 0x04)

/*---------- type define ----------*/
typedef struct {
    uint32_t blink_time; //! unit=ms
    void (*init)(void);
    void (*deinit)(void);
    void (*ctrl)(bool on);
    void (*toggle)(void);
} led_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
#endif /* __LED_H */
