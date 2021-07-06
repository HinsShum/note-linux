/**
 * @file /driver/led.c
 *
 * Copyright (C) 2020
 *
 * led.c is free software: you can redistribute it and/or modify
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

/*---------- includes ----------*/
#include "led.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t led_open(driver_t **pdrv);
static void led_close(driver_t **pdrv);
static int32_t led_ioctl(driver_t **pdrv, uint32_t cmd, void *arg);

/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(led, led_open, led_close, NULL, NULL, led_ioctl, NULL);

/*---------- function ----------*/
static int32_t led_open(driver_t **pdrv)
{
    led_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        pdesc->init();
    }
    if(pdesc && pdesc->ctrl) {
        pdesc->ctrl(false);
    }

    return retval;
}

static void led_close(driver_t **pdrv)
{
    led_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
    if(pdesc && pdesc->ctrl) {
        pdesc->ctrl(false);
    }
}

static int32_t led_ioctl(driver_t **pdrv, uint32_t cmd, void *arg)
{
    led_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;
    led_cycle_t *cycle = NULL;

    assert(pdrv);

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_LED_ON:
            if(pdesc->ctrl) {
                pdesc->ctrl(true);
            }
            break;
        case IOCTL_LED_OFF:
            if(pdesc->ctrl) {
                pdesc->ctrl(false);
                pdesc->cycle_count = 0;
                pdesc->cycle_time = 0;
            }
            break;
        case IOCTL_LED_TOGGLE:
            if(pdesc->toggle) {
                pdesc->toggle();
                if(pdesc->cycle_count != 0 && pdesc->cycle_count != LED_CYCLE_COUNT_MAX) {
                    pdesc->cycle_count--;
                }
            }
            break;
        case IOCTL_LED_SET_CYCLE:
            if(arg) {
                cycle = (led_cycle_t *)arg;
                pdesc->cycle_time = cycle->cycle_time;
                pdesc->cycle_count = cycle->cycle_count;
            }
            break;
        case IOCTL_LED_GET_CYCLE:
            if(arg) {
                cycle = (led_cycle_t *)arg;
                cycle->cycle_time = pdesc->cycle_time;
                cycle->cycle_count = pdesc->cycle_count;
            }
            break;
        case IOCTL_LED_GET_STATUS:
            if(arg && pdesc && pdesc->get) {
                *(bool *)arg = pdesc->get();
            }
            break;
        default:
            retval = CY_E_WRONG_ARGS;
            break;
    }

    return retval;
}
