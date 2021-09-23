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

/* private ioctl functions 
 */
static int32_t _ioctl_led_turn_on(led_describe_t *pdesc, void *args);
static int32_t _ioctl_led_turn_off(led_describe_t *pdesc, void *args);
static int32_t _ioctl_led_toggle(led_describe_t *pdesc, void *args);
static int32_t _ioctl_led_set_cycle(led_describe_t *pdesc, void *args);
static int32_t _ioctl_led_get_cycle(led_describe_t *pdesc, void *args);
static int32_t _ioctl_led_get_status(led_describe_t *pdesc, void *args);

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(led_describe_t *pdesc, void *args);
typedef struct {
    uint32_t ioctl_cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable ----------*/
DRIVER_DEFINED(led, led_open, led_close, NULL, NULL, led_ioctl, NULL);

/* define led ioctl functions
 */
static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_LED_ON, _ioctl_led_turn_on},
    {IOCTL_LED_OFF, _ioctl_led_turn_off},
    {IOCTL_LED_TOGGLE, _ioctl_led_toggle},
    {IOCTL_LED_SET_CYCLE, _ioctl_led_set_cycle},
    {IOCTL_LED_GET_CYCLE, _ioctl_led_get_cycle},
    {IOCTL_LED_GET_STATUS, _ioctl_led_get_status}
};

/*---------- function ----------*/
static int32_t led_open(driver_t **pdrv)
{
    led_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Led device has no describe field\n");
            break;
        }
        retval = CY_EOK;
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                __debug_error("Led device initialize failed\n");
                retval = CY_ERROR;
                break;
            }
        }
        if(pdesc->ops.ctrl) {
            pdesc->ops.ctrl(false);
        }
    } while(0);

    return retval;
}

static void led_close(driver_t **pdrv)
{
    led_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Led device has no describe field\n");
            break;
        }
        if(pdesc->ops.ctrl) {
            pdesc->ops.ctrl(false);
        }
        if(pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }
    } while(0);
}

static int32_t _ioctl_led_turn_on(led_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;

    do {
        if(!pdesc->ops.ctrl) {
            __debug_error("Led driver has no turn on ops\n");
            break;
        }
        retval = CY_EOK;
        if(!pdesc->ops.ctrl(true)) {
            retval = CY_ERROR;
            __debug_error("Led driver try to turn on the led failed\n");
        }
    } while(0);

    return retval;
}

static int32_t _ioctl_led_turn_off(led_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;

    do {
        if(!pdesc->ops.ctrl) {
            __debug_error("Led driver has no turn off ops\n");
            break;
        }
        retval = CY_EOK;
        if(!pdesc->ops.ctrl(false)) {
            retval = CY_ERROR;
            __debug_error("Led driver try to turn off the led failed\n");
            break;
        }
        pdesc->cycle.cycle_count = 0;
        pdesc->cycle.cycle_time = 0;
    } while(0);

    return retval;
}

static int32_t _ioctl_led_toggle(led_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;

    do {
        if(!pdesc->ops.toggle) {
            __debug_error("Led driver has no toggle ops\n");
            break;
        }
        retval = CY_EOK;
        if(!pdesc->ops.toggle()) {
            retval = CY_ERROR;
            __debug_error("Led driver try to toggle the led failed\n");
            break;
        }
        if(pdesc->cycle.cycle_count != 0 && pdesc->cycle.cycle_count != LED_CYCLE_COUNT_MAX) {
            pdesc->cycle.cycle_count--;
        }
    } while(0);

    return retval;
}

static int32_t _ioctl_led_set_cycle(led_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    led_cycle_t *pcycle = (led_cycle_t *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, can not set the led cycle\n");
            break;
        }
        pdesc->cycle.cycle_count = pcycle->cycle_count;
        pdesc->cycle.cycle_time = pcycle->cycle_time;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_led_get_cycle(led_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    led_cycle_t *pcycle = (led_cycle_t *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, no memory to store the cycle information\n");
            break;
        }
        pcycle->cycle_count = pdesc->cycle.cycle_count;
        pcycle->cycle_time = pdesc->cycle.cycle_time;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_led_get_status(led_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    bool *pstatus = (bool *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, no memory to store the led status\n");
            break;
        }
        if(!pdesc->ops.get) {
            __debug_error("Led driver has no get ops\n");
            break;
        }
        *pstatus = pdesc->ops.get();
        retval = CY_EOK;
    } while(0);

    return retval;
}

static ioctl_cb_func_t _ioctl_cb_func_find(uint32_t ioctl_cmd)
{
    ioctl_cb_func_t cb = NULL;

    for(uint32_t i = 0; i < ARRAY_SIZE(ioctl_cb_array); ++i) {
        if(ioctl_cb_array[i].ioctl_cmd == ioctl_cmd) {
            cb = ioctl_cb_array[i].cb;
            break;
        }
    }

    return cb;
}

static int32_t led_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    led_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Led device has no describe field\n");
            break;
        }
        if(NULL == (cb = _ioctl_cb_func_find(cmd))) {
            __debug_error("Led driver not support this command(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}
