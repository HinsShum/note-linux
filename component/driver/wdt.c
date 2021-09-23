/**
 * @file driver/wdt.c
 *
 * Copyright (C) 2021
 *
 * wdt.c is free software: you can redistribute it and/or modify
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
#include "wdt.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t wdt_open(driver_t **pdrv);
static void wdt_close(driver_t **pdrv);
static int32_t wdt_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(wdt, wdt_open, wdt_close, NULL, NULL, wdt_ioctl, NULL);

/*---------- function ----------*/
static int32_t wdt_open(driver_t **pdrv)
{
    wdt_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        retval = (pdesc->init() ? CY_EOK : CY_ERROR);
    }

    return retval;
}

static void wdt_close(driver_t **pdrv)
{
    wdt_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

static int32_t wdt_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    wdt_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_WDT_FEED:
            if(pdesc && pdesc->feed) {
                retval = (pdesc->feed() ? CY_EOK : CY_ERROR);
            } 
            break;
        default:
            break;
    }

    return retval;
}
