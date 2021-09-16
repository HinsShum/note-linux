/**
 * @file driver\ms8837.c
 *
 * Copyright (C) 2021
 *
 * ms8837.c is free software: you can redistribute it and/or modify
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
#include "ms8837.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t ms8837_open(driver_t **pdrv);
static void ms8837_close(driver_t **pdrv);
static int32_t ms8837_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(ms8837, ms8837_open, ms8837_close, NULL, NULL, ms8837_ioctl, NULL);

/*---------- function ----------*/
static int32_t ms8837_open(driver_t **pdrv)
{
    ms8837_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        retval = (pdesc->init() ? CY_EOK : CY_ERROR);
    }

    return retval;
}

static void ms8837_close(driver_t **pdrv)
{
    ms8837_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

static int32_t ms8837_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    ms8837_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_MS8837_TURN_ON_FORWARD:
            if(pdesc && pdesc->ctrl) {
                pdesc->ctrl(CTRL_FREE);
                __delay_us(1);
                retval = (pdesc->ctrl(CTRL_FORWARD) ? CY_EOK : CY_ERROR);
            }
            break;
        case IOCTL_MS8837_TURN_ON_BACKWARD:
            if(pdesc && pdesc->ctrl) {
                pdesc->ctrl(CTRL_FREE);
                __delay_us(1);
                retval = (pdesc->ctrl(CTRL_BACKWARD) ? CY_EOK : CY_ERROR);
            }
            break;
        case IOCTL_MS8837_TURN_OFF:
            if(pdesc && pdesc->ctrl) {
                retval = (pdesc->ctrl(CTRL_STOP) ? CY_EOK : CY_ERROR);
            }
            break;
        case IOCTL_MS8837_TURN_ON_FREE:
            if(pdesc && pdesc->ctrl) {
                retval = (pdesc->ctrl(CTRL_FREE) ? CY_EOK : CY_ERROR);
            }
            break;
        case IOCTL_MS8837_SLEEP_ENABLE:
            if(pdesc && pdesc->sleep) {
                retval = (pdesc->sleep(true) ? CY_EOK : CY_ERROR);
            }
            break;
        case IOCTL_MS8837_SLEEP_DISABLE:
            if(pdesc && pdesc->sleep) {
                retval = (pdesc->sleep(false) ? CY_EOK : CY_ERROR);
            }
            break;
        case IOCTL_DEVICE_POWER_ON:
            if(pdesc && pdesc->power) {
                retval = (pdesc->power(true) ? CY_EOK : CY_ERROR);
            }
            break;
        case IOCTL_DEVICE_POWER_OFF:
            if(pdesc && pdesc->power) {
                retval = (pdesc->power(false) ? CY_EOK : CY_ERROR);
            }
            break;
        default:
            break;
    }

    return retval;
}
