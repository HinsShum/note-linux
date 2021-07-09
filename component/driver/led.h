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
#define IOCTL_LED_ON                        (IOCTL_USER_START + 0x00)
#define IOCTL_LED_OFF                       (IOCTL_USER_START + 0x01)
#define IOCTL_LED_TOGGLE                    (IOCTL_USER_START + 0x02)
#define IOCTL_LED_SET_CYCLE                 (IOCTL_USER_START + 0x03)
#define IOCTL_LED_GET_CYCLE                 (IOCTL_USER_START + 0x04)
#define IOCTL_LED_GET_STATUS                (IOCTL_USER_START + 0x05)

#define LED_CYCLE_COUNT_MAX                 (0xFFFFFFFF)

/*---------- type define ----------*/
typedef struct {
    uint32_t cycle_time;
    uint32_t cycle_count;
    void (*init)(void);
    void (*deinit)(void);
    void (*ctrl)(bool on);
    void (*toggle)(void);
    bool (*get)(void);
} led_describe_t;

typedef struct {
    uint32_t cycle_time;
    uint32_t cycle_count;
} led_cycle_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __LED_H */
