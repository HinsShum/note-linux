/**
 * @file driver\include\ms523.h
 *
 * Copyright (C) 2021
 *
 * ms523.h is free software: you can redistribute it and/or modify
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
#ifndef __MS523_H
#define __MS523_H

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
#define IOCTL_MS523_PCD_HALT                                (IOCTL_USER_START + 0x00)
#define IOCTL_MS523_PCD_REQUEST_IDLE                        (IOCTL_USER_START + 0x01)
#define IOCTL_MS523_PCD_REQUEST_ALL                         (IOCTL_USER_START + 0x02)
#define IOCTL_MS523_PCD_ANTICOLL                            (IOCTL_USER_START + 0x03)
#define IOCTL_MS523_PCD_SELECT                              (IOCTL_USER_START + 0x04)

/*---------- type define ----------*/
typedef struct {
    uint8_t card_type[2];
    uint8_t card_id[4];
    bool (*init)(void);
    void (*deinit)(void);
    uint8_t (*xfer)(uint8_t data);
    void (*cs_ctrl)(bool ctrl);
    void (*reset_ctrl)(bool ctrl);
} ms523_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __MS523_H */
