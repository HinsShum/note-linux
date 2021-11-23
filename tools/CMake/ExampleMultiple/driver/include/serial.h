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
/**
 * @brief Get the serial comport.
 * @note The comport is useful when program running under the windows
 *       or linux platforms. But under the embeded platform, the comport
 *       is not necessary.
 * @para Args is a pointer of the buffer to store the comport information,
 *       the buffer type is `uint8_t`.
 * @retval If the args is is NULL, the interface will return CY_E_WRONG_ARGS,
 *         otherwise, return CY_EOK.
 */
#define IOCTL_SERIAL_GET_COMPORT                    (IOCTL_USER_START + 0x00)

/**
 * @brief Set the irq server callback function.
 * @note If enable the serial interrupt function, when occur once interrupt,
 *       the callback funtion will be called once.
 * @param Args is the pointer of the callback function.
 *        The type is `int32_t (*)(uint32_t irq_handler. void *args, uint32_t length)`.
 * @retval The interface always return CY_EOK.
 */
#define IOCTL_SERIAL_SET_IRQ_HANDLER                (IOCTL_USER_START + 0x01)

/**
 * @brief Set the direction about the serial device.
 * @param Args is the pointer of the direct variables. The type of the direct
 *        variable is `serial_direction_en`.
 * @retval If the args is NULL or invalid value, the interface will return
 *         CY_E_WRONG_ARGS.
 *         If the serial device not support set direction, the interface
 *         will return CY_E_WRONG_ARGS.
 *         If set direction ok, the interface will return CY_EOK.
 */
#define IOCTL_SERIAL_SET_DIRECTION                  (IOCTL_USER_START + 0x02)

/**
 * @brief Get the baudrate of the serial device.
 * @param Args is a pointer of the buffer to store the badurate information.
 * @retval If the args is NULL, the interface will return CY_E_WRONG_ARGS,
 *         otherwise, return CY_EOK.
 */
#define IOCTL_SERIAL_GET_BAUDRATE                   (IOCTL_USER_START + 0x03)

/**
 * @brief Set the baudrate of the serial device.
 * @param Args is a ponter of the baudrate variable address.
 * @retval If the args is NULL, the interface will return CY_E_WRONG_ARGS.
 *         If serial re-initialze failed, the interface will return
 *         CY_ERROR, otherwise, return CY_EOK.
 */
#define IOCTL_SERIAL_SET_BAUDRATE                   (IOCTL_USER_START + 0x04)

/**
 * @brief The IOCTL_SERIAL_INHERIT_START is used when other drivers to inherit
 *        the serial driver.
 */
#define IOCTL_SERIAL_INHERIT_START                  (IOCTL_USER_START + 0x05)

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
    bool (*init)(void);
    void (*deinit)(void);
    void (*dir_change)(serial_direction_en dir);
    uint16_t (*write)(uint8_t *pbuf, uint16_t len);
    int32_t (*irq_handler)(uint32_t irq_handler, void *args, uint32_t len);
} serial_ops_t;

typedef struct {
    uint8_t comport;
    uint32_t baudrate;
    serial_ops_t ops;
} serial_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __SERIAL_H */
