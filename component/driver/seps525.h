/**
 * @file driver\include\seps525.h
 *
 * Copyright (C) 2021
 *
 * seps525.h is free software: you can redistribute it and/or modify
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
#ifndef __DRIVER_SEPS525_H
#define __DRIVER_SEPS525_H

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
#define IOCTL_SEPS525_DRAW_PIXEL                    (IOCTL_USER_START + 0x00)
#define IOCTL_SEPS525_SCREEN_CLEAR                  (IOCTL_USER_START + 0x01)
#define IOCTL_SEPS525_DRAW_RECTANGLE                (IOCTL_USER_START + 0x02)

/* color
 */
#define COLOR_SEPS525_BLACK                         (0x0000)
#define COLOR_SEPS525_WHITE                         (0xFFFF)

/*---------- type define ----------*/
typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    uint8_t (*xfer)(uint8_t ch);
    void (*cs_ctrl)(bool ctrl);
    void (*rst_ctrl)(bool ctrl);
    void (*rs_ctrl)(bool ctrl);
} seps525_describe_t;

typedef union {
    struct {
        uint32_t x;
        uint32_t y;
        uint32_t color;
    } pixel;
    struct {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
        uint16_t *color;
    } rectangle;
} seps525_ioctl_args_un;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __DRIVER_SEPS525_H */
