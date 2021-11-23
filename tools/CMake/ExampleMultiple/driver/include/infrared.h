/**
 * @file driver\include\infrared.h
 *
 * Copyright (C) 2021
 *
 * infrared.h is free software: you can redistribute it and/or modify
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
#ifndef __INFARED_H
#define __INFARED_H

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
#define IOCTL_INFRARED_TURN_ON                      (IOCTL_USER_START + 0x00)
#define IOCTL_INFRARED_TURN_OFF                     (IOCTL_USER_START + 0x01)
#define IOCTL_INFRARED_TOGGLE                       (IOCTL_USER_START + 0x02)
#define IOCTL_INFRARED_SET_CYCLE                    (IOCTL_USER_START + 0x03)
#define IOCTL_INFRARED_GET_CYCLE                    (IOCTL_USER_START + 0x04)
#define IOCTL_INFRARED_SET_FREQ                     (IOCTL_USER_START + 0x05)
#define IOCTL_INFRARED_GET_FREQ                     (IOCTL_USER_START + 0x06)

#define INFRARED_TOGGLE_REPEAT_MAX_COUNT            (0xFFFFFFFF)

/*---------- type define ----------*/
typedef struct {
    uint32_t cycle_time;
    uint32_t cycle_count;
} infrared_cycle_t;

typedef struct {
    uint32_t freq;
    infrared_cycle_t cycle;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        bool (*ctrl)(bool en);
        bool (*toggle)(void);
    } ops;
} infrared_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __INFARED_H */
