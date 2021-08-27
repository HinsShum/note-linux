/**
 * @file /driver/inc/led.h
 *
 * Copyright (C) 2020
 *
 * led.h is free software: you can redistribute it and/or modify
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
 */
#ifndef __LED_H
#define __LED_H

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
 * @brief Turn on the led.
 * @param Args is not useful, it can be NULL.
 * @retval If the led driver has no turn on functions, the interface
 *         will return CY_E_WRONG_ARGS.
 *         If the led driver turn on the led failed, the interface
 *         will return CY_ERROR.
 *         If the led driver turn on the led ok, the interface will
 *         return CY_EOK.
 */
#define IOCTL_LED_ON                        (IOCTL_USER_START + 0x00)

/**
 * @brief Turn off the led.
 * @param Args is not useful, it can be NULL.
 * @retval If the led driver has no turn off functions, the interface
 *         will return CY_E_WRONG_ARGS.
 *         If the led driver turn off the led failed, the interface
 *         will return CY_ERROR.
 *         If the led driver turn off the led ok, the interface will
 *         return CY_EOK.
 */
#define IOCTL_LED_OFF                       (IOCTL_USER_START + 0x01)

/**
 * @brief Toggle the led.
 * @param Args is not useful, it can be NULL.
 * @retval If the led driver has no toggle functions, the interface
 *         will return CY_E_WRONG_ARGS.
 *         If the led driver toggle failed, the interface will return
 *         CY_ERROR.
 *         If the led driver toggle ok, the interface will return CY_EOK.
 */
#define IOCTL_LED_TOGGLE                    (IOCTL_USER_START + 0x02)

/**
 * @brief Set the cycle of the led.
 * @param Args is a pointer of the cycle information, the tyoe is `led_cycle_t`.
 * @retval If the args is null, the interface will return CY_E_WRONG_ARGS,
 *         otherwise, return CY_EOK.
 */
#define IOCTL_LED_SET_CYCLE                 (IOCTL_USER_START + 0x03)

/**
 * @brief Get the cycle of the led.
 * @param Args is a pointer to store the cycle information, the type is 
 *        `led_cycle_t`.
 * @retval If the args is null, the interface will return CY_E_WRONG_ARGS,
 *         otherwise, return CY_EOK.
 */
#define IOCTL_LED_GET_CYCLE                 (IOCTL_USER_START + 0x04)

/**
 * @brief Get the led status.
 * @param Args is a pointer to store the led status, the type is `bool`.
 * @retval If the args is null, the interface will return CY_E_WRONG_ARGS,
 *         otherwise, return CY_EOK.
 */
#define IOCTL_LED_GET_STATUS                (IOCTL_USER_START + 0x05)

#define LED_CYCLE_COUNT_MAX                 (0xFFFFFFFF)

/*---------- type define ----------*/
typedef struct {
    uint32_t cycle_time;
    uint32_t cycle_count;
} led_cycle_t;

typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    bool (*ctrl)(bool on);
    bool (*toggle)(void);
    bool (*get)(void);
} led_ops_t;

typedef struct {
    led_cycle_t cycle;
    led_ops_t ops;
} led_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __LED_H */
