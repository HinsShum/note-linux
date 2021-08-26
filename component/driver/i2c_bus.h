/**
 * @file driver\include\i2c_bus.h
 *
 * Copyright (C) 2021
 *
 * i2c_bus.h is free software: you can redistribute it and/or modify
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
#ifndef __I2C_BUS_H
#define __I2C_BUS_H

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
/**
 * @brief Set lock callback function to lock the i2c bus.
 * @param Args is a pointer of the callback function. The callback function
 *        type is `void (*)(void *)`.
 * @retval Always return CY_EOK.
 */
#define IOCTL_I2C_BUS_SET_LOCK_HANDLER                      (IOCTL_USER_START + 0x00)

/**
 * @brief Set unlock callback function to unlock the i2c bus.
 * @param Args is a pointer of the callback function. The callback function
 *        type is `void (*)(void *)`.
 * @return Always return CY_EOK.
 */
#define IOCTL_I2C_BUS_SET_UNLOCK_HANDER                     (IOCTL_USER_START + 0x01)

/**
 * @brief Set the user data.
 * @note The user data is the parameter of the lock or unlock callback function.
 * @param Args is a pointer of user data.
 * @retval Always return CY_EOK.
 */
#define IOCTL_I2C_BUS_SET_USER_DATA                         (IOCTL_USER_START + 0x02)

/**
 * @brief Lock the i2c bus.
 * @param Args is not useful, it can be NULL.
 * @retval Always return CY_EOK.
 */
#define IOCTL_I2C_BUS_LOCK                                  (IOCTL_USER_START + 0x03)

/**
 * @brief Unlock the i2c bus.
 * @param Args is not useful, it can be NULL.
 * @retval Always return CY_EOK.
 */
#define IOCTL_I2C_BUS_UNLOCK                                (IOCTL_USER_START + 0x04)

/*---------- type define ----------*/
typedef enum {
    I2C_BUS_TYPE_RANDOM_READ,
    I2C_BUS_TYPE_SEQUENTIAL_READ,
    I2C_BUS_TYPE_WRITE
} i2c_bus_type_t;

typedef struct {
    i2c_bus_type_t type;
    uint8_t dev_addr;
    uint8_t *mem_addr;
    uint8_t mem_addr_counts;
    uint8_t *buf;
    uint32_t len;
} i2c_bus_msg_t;

typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    void (*scl_set)(bool on);
    bool (*scl_get)(void);
    void (*sda_set)(bool on);
    bool (*sda_get)(void);
    void (*delay)(void);
    void (*lock)(void *data);
    void (*unlock)(void *data);
} i2c_bus_ops_t;

typedef struct {
    void *data;
    i2c_bus_ops_t ops;
} i2c_bus_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __I2C_BUS_H */
