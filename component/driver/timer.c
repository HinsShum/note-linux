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

/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(timer, timer_open, timer_close, NULL, NULL, timer_ioctl, timer_irq_handler);

/*---------- function ----------*/
static int32_t timer_open(driver_t **pdrv)
{
    timer_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        retval = (pdesc->init() ? CY_EOK : CY_ERROR);
    }

    return retval;
}

static void timer_close(driver_t **pdrv)
{
    timer_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

static int32_t timer_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    timer_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_TIMER_GET_INTERVAL:
            if(pdesc && args) {
                *(uint32_t *)args = pdesc->interval_ms;
                retval = CY_EOK;
            }
            break;
        case IOCTL_TIMER_SET_INTERVAL:
            if(pdesc && pdesc->init && args) {
                uint32_t interval = *(uint32_t *)args;
                if(pdesc->interval_ms != interval) {
                    pdesc->interval_ms = interval;
                    pdesc->init();
                }
                retval = CY_EOK;
            }
            break;
        case IOCTL_TIMER_SET_IRQ_HANDLER:
            if(pdesc && args) {
                pdesc->irq_handler = (timer_irq_handler_fn)args;
                retval = CY_EOK;
            }
            break;
        case IOCTL_TIMER_ENABLE:
            if(pdesc && pdesc->enable) {
                retval = (pdesc->enable(true) ? CY_EOK : CY_ERROR);
            }
            break;
        case IOCTL_TIMER_DISABLE:
            if(pdesc && pdesc->enable) {
                retval = (pdesc->enable(false) ? CY_EOK : CY_ERROR);
            }
            break;
        default:
            break;
    }

    return retval;
}

static int32_t timer_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len)
{
    timer_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->irq_handler) {
        retval = pdesc->irq_handler(irq_handler, args, len);
    }

    return retval;
}
