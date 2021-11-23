/**
 * @file driver\include\buzzer.h
 *
 * Copyright (C) 2021
 *
 * buzzer.h is free software: you can redistribute it and/or modify
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
#ifndef __BUZZER_H
#define __BUZZER_H

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
#define IOCTL_BUZZER_ON                             (IOCTL_USER_START + 0x00)
#define IOCTL_BUZZER_OFF                            (IOCTL_USER_START + 0x01)
#define IOCTL_BUZZER_TOGGLE                         (IOCTL_USER_START + 0x02)
#define IOCTL_BUZZER_SET_CYCLE                      (IOCTL_USER_START + 0x03)
#define IOCTL_BUZZER_GET_CYCLE                      (IOCTL_USER_START + 0x04)

#define BUZZER_CYCLE_COUNT_MAX                      (0xFFFFFFFF)

/*---------- type define ----------*/
typedef struct {
    uint32_t cycle_time;
    uint32_t cycle_count;
} buzzer_cycle_t;

typedef struct {
    uint32_t freq;
    buzzer_cycle_t cycle;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        void (*ctrl)(bool on);
        void (*toggle)(void);
    } ops;
} buzzer_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __BUZZER_H */
