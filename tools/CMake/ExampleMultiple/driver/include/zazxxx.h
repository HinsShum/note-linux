/**
 * @file driver\include\zazxxx.h
 *
 * Copyright (C) 2021
 *
 * zazxxx.h is free software: you can redistribute it and/or modify
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
#ifndef __ZAZXXX_H
#define __ZAZXXX_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "serial.h"

/*---------- macro ----------*/
#define IOCTL_ZAZXXX_GET_COMPORT                            (IOCTL_SERIAL_GET_COMPORT)
#define IOCTL_ZAZXXX_SET_IRQ_HANDLER                        (IOCTL_SERIAL_SET_IRQ_HANDLER)
#define IOCTL_ZAZXXX_DIRECTION_CHOOSE                       (IOCTL_SERIAL_SET_DIRECTION)
#define IOCTL_ZAZXXX_GET_BAUDRATE                           (IOCTL_SERIAL_GET_BAUDRATE)
#define IOCTL_ZAZXXX_SET_BAUDRATE                           (IOCTL_SERIAL_SET_BAUDRATE)
#define IOCTL_ZAZXXX_CONNECT                                (IOCTL_SERIAL_SET_BAUDRATE + 0x01)
#define IOCTL_ZAZXXX_DELETE_ALL_TEMPLATE                    (IOCTL_SERIAL_SET_BAUDRATE + 0x02)
#define IOCTL_ZAZXXX_DELETE_ONE_TEMPLATE                    (IOCTL_SERIAL_SET_BAUDRATE + 0x03)
#define IOCTL_ZAZXXX_ENROLL_BY_TEMPLATE                     (IOCTL_SERIAL_SET_BAUDRATE + 0x04)
#define IOCTL_ZAZXXX_GET_ENROLL_NUMBERS                     (IOCTL_SERIAL_SET_BAUDRATE + 0x05)
#define IOCTL_ZAZXXX_GET_IMAGE                              (IOCTL_SERIAL_SET_BAUDRATE + 0x06)
#define IOCTL_ZAZXXX_GENERATE_TEMPLATE                      (IOCTL_SERIAL_SET_BAUDRATE + 0x07)
#define IOCTL_ZAZXXX_SEARCH_TEMPLATE                        (IOCTL_SERIAL_SET_BAUDRATE + 0x08)

/*---------- type define ----------*/
typedef enum {
    RECV_STATE_IDLE,
    RECV_STATE_BUSY
} zazxxx_recv_state_en;

typedef struct {
    uint8_t buffer[530];
    uint16_t offset;
    zazxxx_recv_state_en state;
} zazxxx_private_t;

typedef struct {
    serial_describe_t serial;
    zazxxx_private_t _private;
} zazxxx_describe_t;

/* ioctl args define
 */
typedef struct {
    uint8_t *pdata;
    uint16_t size;
    uint16_t template_id;
    int32_t timeout_ms;
} zazxxx_template_args_t;

typedef struct {
    uint16_t template_id;
    uint8_t score;
    int32_t timeout_ms;
} zazxxx_search_args_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __ZAZXXX_H */
