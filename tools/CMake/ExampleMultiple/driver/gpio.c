/**
 * @file APP/driver/gpio.c
 *
 * Copyright (C) 2021
 *
 * gpio.c is free software: you can redistribute it and/or modify
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
#include "gpio.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
static int32_t gpio_open(driver_t **pdrv);
static void gpio_close(driver_t **pdrv);
static int32_t gpio_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t gpio_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len);

static int32_t __ioctl_get(gpio_describe_t *pdesc, void *args);
static int32_t __ioctl_set(gpio_describe_t *pdesc, void *args);
static int32_t __ioctl_toggle(gpio_describe_t *pdesc, void *args);
static int32_t __ioctl_set_irq(gpio_describe_t *pdesc, void *args);

/*---------- function prototype ----------*/
/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(gpio_describe_t *pdesc, void *args);
typedef struct {
    uint32_t ioctl_cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable ----------*/
DRIVER_DEFINED(gpio, gpio_open, gpio_close, NULL, NULL, gpio_ioctl, gpio_irq_handler);

static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_GPIO_GET, __ioctl_get},
    {IOCTL_GPIO_SET, __ioctl_set},
    {IOCTL_GPIO_TOGGLE, __ioctl_toggle},
    {IOCTL_GPIO_SET_IRQ_HANDLER, __ioctl_set_irq}
};

/*---------- function ----------*/
static int32_t gpio_open(driver_t **pdrv)
{
    gpio_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("GPIO device has not bind describe field\n");
            break;
        }
        retval = CY_EOK;
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                retval = CY_ERROR;
                __debug_warn("GPIO initialize failed\n");
            }
        }
    } while(0);

    return retval;
}

static void gpio_close(driver_t **pdrv)
{
    gpio_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t __ioctl_get(gpio_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    bool *val = (bool *)args;

    if(!args) {
        __debug_error("Args format error, can not get gpio value\n");
    } else {
        if(pdesc->ops.get) {
            *val = pdesc->ops.get();
            retval = CY_EOK;
        } else {
            retval = CY_ERROR;
        }
    }

    return retval;
}

static int32_t __ioctl_set(gpio_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    bool *val = (bool *)args;

    if(!args) {
        __debug_error("Args format error, can not set gpio value\n");
    } else {
        if(pdesc->ops.set) {
            pdesc->ops.set(*val);
            retval = CY_EOK;
        } else {
            retval = CY_ERROR;
        }
    }

    return retval;
}

static int32_t __ioctl_toggle(gpio_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;

    if(pdesc->ops.toggle) {
        pdesc->ops.toggle();
        retval = CY_EOK;
    }

    return retval;
}

static int32_t __ioctl_set_irq(gpio_describe_t *pdesc, void *args)
{
    pdesc->ops.irq_handler = (gpio_irq_handler_fn)args;

    return CY_EOK;
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

static int32_t gpio_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    gpio_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("GPIO driver has not bind describe field\n");
            break;
        }
        if(NULL == (cb = (__ioctl_cb_func_find(cmd)))) {
            __debug_error("GPIO driver not support cmd(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}

static int32_t gpio_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len)
{
    gpio_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.irq_handler) {
        retval = pdesc->ops.irq_handler(irq_handler, args, len);
    }

    return retval;
}
