/**
 * @file APP\driver\buzzer.c
 *
 * Copyright (C) 2021
 *
 * buzzer.c is free software: you can redistribute it and/or modify
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

/*---------- includes ----------*/
#include "buzzer.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t buzzer_open(driver_t **pdrv);
static void buzzer_close(driver_t **pdrv);
static int32_t buzzer_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

static int32_t __ioctl_turn_on(buzzer_describe_t *pdesc, void *args);
static int32_t __ioctl_turn_off(buzzer_describe_t *pdesc, void *args);
static int32_t __ioctl_toggle(buzzer_describe_t *pdesc, void *args);
static int32_t __ioctl_set_cycle(buzzer_describe_t *pdesc, void *args);
static int32_t __ioctl_get_cycle(buzzer_describe_t *pdesc, void *args);

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(buzzer_describe_t *pdesc, void *args);
typedef struct {
    uint32_t ioctl_cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable ----------*/
DRIVER_DEFINED(buzzer, buzzer_open, buzzer_close, NULL, NULL, buzzer_ioctl, NULL);

static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_BUZZER_ON, __ioctl_turn_on},
    {IOCTL_BUZZER_OFF, __ioctl_turn_off},
    {IOCTL_BUZZER_TOGGLE, __ioctl_toggle},
    {IOCTL_BUZZER_SET_CYCLE, __ioctl_set_cycle},
    {IOCTL_BUZZER_GET_CYCLE, __ioctl_get_cycle}
};

/*---------- function ----------*/
static int32_t buzzer_open(driver_t **pdrv)
{
    buzzer_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Buzzer device has not bind describe field\n");
            break;
        }
        retval = CY_EOK;
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                retval = CY_ERROR;
                __debug_warn("Buzzer initialize failed\n");
            }
        }
    } while(0);

    return retval;
}

static void buzzer_close(driver_t **pdrv)
{
    buzzer_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t __ioctl_turn_on(buzzer_describe_t *pdesc, void *args)
{
    if(pdesc->ops.ctrl) {
        pdesc->ops.ctrl(true);
    }

    return CY_EOK;
}

static int32_t __ioctl_turn_off(buzzer_describe_t *pdesc, void *args)
{
    if(pdesc->ops.ctrl) {
        pdesc->ops.ctrl(false);
        pdesc->cycle.cycle_count = 0;
        pdesc->cycle.cycle_time = 0;
    }

    return CY_EOK;
}

static int32_t __ioctl_toggle(buzzer_describe_t *pdesc, void *args)
{
    if(pdesc->ops.toggle) {
        pdesc->ops.toggle();
        if(pdesc->cycle.cycle_count != 0 && pdesc->cycle.cycle_count != BUZZER_CYCLE_COUNT_MAX) {
            pdesc->cycle.cycle_count--;
        }
    }

    return CY_EOK;
}

static int32_t __ioctl_set_cycle(buzzer_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    buzzer_cycle_t *cycle = (buzzer_cycle_t *)args;

    if(!args) {
        __debug_error("Args format error, can not set buzzer cycle\n");
    } else {
        pdesc->cycle.cycle_count = cycle->cycle_count;
        pdesc->cycle.cycle_time = cycle->cycle_time;
        retval = CY_EOK;
    }

    return retval;
}

static int32_t __ioctl_get_cycle(buzzer_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    buzzer_cycle_t *cycle = (buzzer_cycle_t *)args;

    if(!args) {
        __debug_error("Args format error, can not get buzzer cycle\n");
    } else {
        cycle->cycle_count = pdesc->cycle.cycle_count;
        cycle->cycle_time = pdesc->cycle.cycle_time;
        retval = CY_EOK;
    }

    return retval;
}

static ioctl_cb_func_t __ioctl_cb_func_find(uint32_t ioctl_cmd)
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

static int32_t buzzer_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    buzzer_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Buzzer driver has not bind describe field\n");
            break;
        }
        if(NULL == (cb = __ioctl_cb_func_find(cmd))) {
            __debug_error("Buzzer driver not support command(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}
