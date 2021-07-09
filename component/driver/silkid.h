/**
 * @file driver\include\silkid.h
 *
 * Copyright (C) 2021
 *
 * silkid.h is free software: you can redistribute it and/or modify
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
#ifndef __SILKID_H
#define __SILKID_H

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
#define SILKID_NORMAL_PACKET_LENGTH                 (13)

/* ioctl command macro define
 */
#define IOCTL_SILKID_GET_COMPORT                    (IOCTL_USER_START + 0x00)
#define IOCTL_SILKID_SET_IRQ_HANDLER                (IOCTL_USER_START + 0x01)
#define IOCTL_SILKID_DIRECTION_CHOOSE               (IOCTL_USER_START + 0x02)
#define IOCTL_SILKID_GET_BAUDRATE                   (IOCTL_USER_START + 0x03)
#define IOCTL_SILKID_SET_BAUDRATE                   (IOCTL_USER_START + 0x04)
#define IOCTL_SILKID_CONNECT                        (IOCTL_USER_START + 0x05)
#define IOCTL_SILKID_DISCONNECT                     (IOCTL_USER_START + 0x06)
#define IOCTL_SILKID_DELETE_ALL_TEMPLATE            (IOCTL_USER_START + 0x07)
#define IOCTL_SILKID_DELETE_TEMPLATE                (IOCTL_USER_START + 0x08)
#define IOCTL_SILKID_DELETE_USER_TEMPLATE           (IOCTL_USER_START + 0x09)
#define IOCTL_SILKID_ENROLL_TEMPLATE                (IOCTL_USER_START + 0x0A)
#define IOCTL_SILKID_GET_ENROLL_NUMBERS             (IOCTL_USER_START + 0x0B)

/*---------- type define ----------*/
typedef enum {
    SILKID_MESSAGE_NONE,
    SILKID_MESSAGE_VERIFY_SUCCESS,
    SILKID_MESSAGE_VERIFY_FAILE,
    SILKID_MESSAGE_CLEAR_TEMPLATE_SUCCESS,
    SILKID_MESSAGE_CLEAR_TEMPLATE_FAILE
} silkid_message_en;

typedef struct {
    uint32_t id;
    silkid_message_en msg;
} silkid_irq_handler_args_t;

typedef enum {
    SILKID_ENROLL_RESULT_SUCCESS,
    SILKID_ENROLL_RESULT_FAILE,
    SILKID_ENROLL_RESULT_FULL,
    SILKID_ENROLL_RESULT_ID_REPEAT
} silkid_enroll_result_en;

typedef struct {
    uint32_t id;        /*<< fingerid << 16 | userid */
    void *template;
    uint32_t size;
    silkid_enroll_result_en result;
} silkid_enroll_template_t;

typedef struct {
    uint8_t buf[SILKID_NORMAL_PACKET_LENGTH];
    uint8_t offset;
    uint8_t status;
    bool iswrite;
} silkid_normal_recv_t;

typedef struct {
    serial_describe_t serial;
    silkid_normal_recv_t normal;
} silkid_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __SILKID_H */
