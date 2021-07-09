/**
 * @file driver/include/serial.h
 *
 * Copyright (C) 2021
 *
 * serial.h is free software: you can redistribute it and/or modify
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
#ifndef __SERIAL_H
#define __SERIAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <device.h>

/*---------- macro ----------*/
#define IOCTL_SERIAL_GET_COMPORT                    (IOCTL_USER_START + 0x00)
#define IOCTL_SERIAL_SET_IRQ_HANDLER                (IOCTL_USER_START + 0x01)
#define IOCTL_SERIAL_DIRECTION_CHOOSE               (IOCTL_USER_START + 0x02)
#define IOCTL_SERIAL_GET_BAUDRATE                   (IOCTL_USER_START + 0x03)
#define IOCTL_SERIAL_SET_BAUDRATE                   (IOCTL_USER_START + 0x04)

/* Write automatically change dir flag 
 */
#define SERIAL_WIRTE_CHANGE_DIR_AUTOMATICALLY       (0x00)
#define SERIAL_WIRTE_CHANGE_DIR_MANUAL              (0x01)

/*---------- type define ----------*/
typedef enum {
    SERIAL_DIRECTION_TX,
    SERIAL_DIRECTION_RX,
    SERIAL_DIRECTION_NRX_NTX
} serial_direction_en;

typedef struct {
    uint8_t comport;
    uint32_t baudrate;
    bool (*init)(void);
    void (*deinit)(void);
    void (*dir_change)(serial_direction_en dir);
    uint16_t (*write)(uint8_t *pbuf, uint16_t len);
    int32_t (*irq_handler)(uint32_t irq_handler, void *args, uint32_t len);
} serial_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __SERIAL_H */
