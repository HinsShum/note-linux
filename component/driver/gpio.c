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

/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(gpio, gpio_open, gpio_close, NULL, NULL, gpio_ioctl, gpio_irq_handler);

/*---------- function ----------*/
static int32_t gpio_open(driver_t **pdrv)
{
    gpio_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        retval = (pdesc->init() ? CY_EOK : CY_ERROR);
    }

    return retval;
}

static void gpio_close(driver_t **pdrv)
{
    gpio_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

static int32_t gpio_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    gpio_describe_t *pdesc = NULL;
    int32_t retval = CY_ERROR;
    bool *res = (bool *)args;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_GPIO_GET:
            if(pdesc && pdesc->get && res) {
                *res = pdesc->get();
                retval = CY_EOK;
            }
            break;
        case IOCTL_GPIO_SET:
            if(pdesc && pdesc->set && res) {
                pdesc->set(*res);
                retval = CY_EOK;
            }
            break;
        case IOCTL_GPIO_TOGGLE:
            if(pdesc && pdesc->toggle) {
                pdesc->toggle();
                retval = CY_EOK;
            }
            break;
        case IOCTL_GPIO_SET_IRQ_HANDLER:
            if(pdesc) {
                pdesc->irq_handler = (gpio_irq_handler_fn)args;
            }
            break;
        default:
            retval = CY_E_WRONG_ARGS;
            break;
    }

    return retval;
}

static int32_t gpio_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len)
{
    gpio_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->irq_handler) {
        retval = pdesc->irq_handler(irq_handler, args, len);
    }

    return retval;
}
