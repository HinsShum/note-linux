/**
 * @file driver\include\ms8837.h
 *
 * Copyright (C) 2021
 *
 * ms8837.h is free software: you can redistribute it and/or modify
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
#ifndef __MS8837_H
#define __MS8837_H

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
#define IOCTL_MS8837_TURN_ON_FORWARD                    (IOCTL_USER_START + 0x00)
#define IOCTL_MS8837_TURN_ON_BACKWARD                   (IOCTL_USER_START + 0x01)
#define IOCTL_MS8837_TURN_ON_FREE                       (IOCTL_USER_START + 0x02)
#define IOCTL_MS8837_TURN_OFF                           (IOCTL_USER_START + 0x03)
#define IOCTL_MS8837_SLEEP_ENABLE                       (IOCTL_USER_START + 0x04)
#define IOCTL_MS8837_SLEEP_DISABLE                      (IOCTL_USER_START + 0x05)

/*---------- type define ----------*/
typedef enum {
    CTRL_FORWARD,
    CTRL_BACKWARD,
    CTRL_STOP,
    CTRL_FREE
} ms8837_control_en;

typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    bool (*ctrl)(ms8837_control_en ctrl);
    bool (*sleep)(bool ctrl);
    bool (*power)(bool ctrl);
} ms8837_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __MS8837_H */
