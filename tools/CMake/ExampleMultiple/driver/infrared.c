/**
 * @file driver\infrared.c
 *
 * Copyright (C) 2021
 *
 * infrared.c is free software: you can redistribute it and/or modify
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
#include "infrared.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t __open(driver_t **pdrv);
static void __close(driver_t **pdrv);
static int32_t __ioctl(driver_t **pdrv, uint32_t cmd, void *args);

static int32_t __ioctl_turn_on(infrared_describe_t *pdesc, void *args);
static int32_t __ioctl_turn_off(infrared_describe_t *pdesc, void *args);
static int32_t __ioctl_toggle(infrared_describe_t *pdesc, void *args);
static int32_t __ioctl_set_cycle(infrared_describe_t *pdesc, void *args);
static int32_t __ioctl_get_cycle(infrared_describe_t *pdesc, void *args);
static int32_t __ioctl_set_freq(infrared_describe_t *pdesc, void *args);
static int32_t __ioctl_get_freq(infrared_describe_t *pdesc, void *args);

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(infrared_describe_t *pdesc, void *args);
typedef struct {
    uint32_t ioctl_cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable ----------*/
DRIVER_DEFINED(infrared, __open, __close, NULL, NULL, __ioctl, NULL);

static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_INFRARED_TURN_ON, __ioctl_turn_on},
    {IOCTL_INFRARED_TURN_OFF, __ioctl_turn_off},
    {IOCTL_INFRARED_TOGGLE, __ioctl_toggle},
    {IOCTL_INFRARED_SET_CYCLE, __ioctl_set_cycle},
    {IOCTL_INFRARED_GET_CYCLE, __ioctl_get_cycle},
    {IOCTL_INFRARED_SET_FREQ, __ioctl_set_freq},
    {IOCTL_INFRARED_GET_FREQ, __ioctl_get_freq}
};

/*---------- function ----------*/
static int32_t __open(driver_t **pdrv)
{
    infrared_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Infrared device has not bind describe field\n");
            break;
        }
        retval = CY_EOK;
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                retval = CY_ERROR;
                __debug_warn("Infrared initialize failed\n");
            }
        }
    } while(0);

    return retval;
}

static void __close(driver_t **pdrv)
{
    infrared_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t __ioctl_turn_on(infrared_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;

    if(!pdesc->ops.ctrl) {
        __debug_warn("Infrared driver has no ctrl ops\n");
    } else {
        if(pdesc->ops.ctrl(true)) {
            retval = CY_EOK;
        }
    }

    return retval;
}

static int32_t __ioctl_turn_off(infrared_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;

    if(!pdesc->ops.ctrl) {
        __debug_warn("Infrared driver has no ctrl ops\n");
    } else {
        if(pdesc->ops.ctrl(false)) {
            pdesc->cycle.cycle_count = 0;
            pdesc->cycle.cycle_time = 0;
            retval = CY_EOK;
        }
    }

    return retval;
}

static int32_t __ioctl_toggle(infrared_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;

    if(!pdesc->ops.toggle) {
        __debug_error("Infrared driver has no toggle ops\n");
    } else {
        if(pdesc->ops.toggle()) {
            if(pdesc->cycle.cycle_count != 0 && pdesc->cycle.cycle_count != INFRARED_TOGGLE_REPEAT_MAX_COUNT) {
                pdesc->cycle.cycle_count--;
            }
            retval = CY_EOK;
        }
    }

    return retval;
}

static int32_t __ioctl_set_cycle(infrared_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    infrared_cycle_t *cycle = NULL;

    if(!args) {
        __debug_error("Args format error, can not update infrared cycle variables\n");
    } else {
        cycle = (infrared_cycle_t *)args;
        pdesc->cycle = *cycle;
        retval = CY_EOK;
    }

    return retval;
}

static int32_t __ioctl_get_cycle(infrared_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    infrared_cycle_t *cycle = NULL;
    
    if(!args) {
        __debug_error("Args format error, can not get infrared cycle variables\n");
    } else {
        cycle = (infrared_cycle_t *)args;
        *cycle = pdesc->cycle;
        retval = CY_EOK;
    }

    return retval;
}

static int32_t __ioctl_set_freq(infrared_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *freq = (uint32_t *)args;

    if(!args) {
        __debug_error("Args format error, can not set freq for infrared\n");
    } else {
        if(pdesc->freq != *freq) {
            pdesc->freq = *freq;
            if(pdesc->ops.deinit) {
                pdesc->ops.deinit();
            }
            if(pdesc->ops.init) {
                pdesc->ops.init();
            }
        }
        retval = CY_EOK;
    }

    return retval;
}

static int32_t __ioctl_get_freq(infrared_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *freq = (uint32_t *)args;

    if(!args) {
        __debug_error("Args format error, can not get infrared freq\n");
    } else {
        *freq = pdesc->freq;
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

static int32_t __ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    infrared_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Infrared driver has not bind describe field\n");
            break;
        }
        if(NULL == (cb = __ioctl_cb_func_find(cmd))) {
            __debug_error("Infrared driver not support cmd(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}
