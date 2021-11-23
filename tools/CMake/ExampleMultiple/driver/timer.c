/**
 * @file driver\timer.c
 *
 * Copyright (C) 2021
 *
 * timer.c is free software: you can redistribute it and/or modify
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
#include "timer.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t timer_open(driver_t **pdrv);
static void timer_close(driver_t **pdrv);
static int32_t timer_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t timer_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len);

static int32_t __ioctl_get_freq(timer_describe_t *pdesc, void *args);
static int32_t __ioctl_set_freq(timer_describe_t *pdesc, void *args);
static int32_t __ioctl_set_irq(timer_describe_t *pdesc, void *args);
static int32_t __ioctl_enable(timer_describe_t *pdesc, void *args);
static int32_t __ioctl_disable(timer_describe_t *pdesc, void *args);

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(timer_describe_t *pdesc, void *args);
typedef struct {
    uint32_t cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable ----------*/
DRIVER_DEFINED(timer, timer_open, timer_close, NULL, NULL, timer_ioctl, timer_irq_handler);
static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_TIMER_GET_FREQ, __ioctl_get_freq},
    {IOCTL_TIMER_SET_FREQ, __ioctl_set_freq},
    {IOCTL_TIMER_SET_IRQ_HANDLER, __ioctl_set_irq},
    {IOCTL_TIMER_ENABLE, __ioctl_enable},
    {IOCTL_TIMER_DISABLE, __ioctl_disable}
};

/*---------- function ----------*/
static int32_t timer_open(driver_t **pdrv)
{
    timer_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Timer device has not bind describe field\n");
            break;
        }
        retval = CY_EOK;
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                retval = CY_ERROR;
                __debug_warn("Timer initialize bsp failed\n");
            }
        }
    } while(0);

    return retval;
}

static void timer_close(driver_t **pdrv)
{
    timer_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t __ioctl_get_freq(timer_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *freq = (uint32_t *)args;

    if(!args) {
        __debug_error("Args format error, can not get timer freq\n");
    } else {
        *freq = pdesc->freq;
        retval = CY_EOK;
    }

    return retval;
}

static int32_t __ioctl_set_freq(timer_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *freq = (uint32_t *)args;

    if(!args) {
        __debug_error("Args format error, can not set timer freq\n");
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

static int32_t __ioctl_set_irq(timer_describe_t *pdesc, void *args)
{
    timer_irq_handler_fn irq = (timer_irq_handler_fn)args;

    pdesc->ops.irq_handler = irq;

    return CY_EOK;
}

static int32_t __ioctl_enable(timer_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;

    if(pdesc->ops.enable) {
        if(pdesc->ops.enable(true)) {
            retval = CY_EOK;
        }
    }

    return retval;
}

static int32_t __ioctl_disable(timer_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;

    if(pdesc->ops.enable) {
        if(pdesc->ops.enable(false)) {
            retval = CY_EOK;
        }
    }

    return retval;
}

static ioctl_cb_func_t __ioctl_cb_func_find(uint32_t ioctl_cmd)
{
    ioctl_cb_func_t cb = NULL;

    for(uint32_t i = 0; i < ARRAY_SIZE(ioctl_cb_array); ++i) {
        if(ioctl_cb_array[i].cmd == ioctl_cmd) {
            cb = ioctl_cb_array[i].cb;
            break;
        }
    }

    return cb;
}

static int32_t timer_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    timer_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Timer driver has not bind describe field\n");
            break;
        }
        if(NULL == (cb = __ioctl_cb_func_find(cmd))) {
            __debug_error("Timer driver not support cmd(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}

static int32_t timer_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len)
{
    timer_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.irq_handler) {
        retval = pdesc->ops.irq_handler(irq_handler, args, len);
    }

    return retval;
}
