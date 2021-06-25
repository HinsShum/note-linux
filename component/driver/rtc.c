/**
 * @file driver\rtc.c
 *
 * Copyright (C) 2021
 *
 * rtc.c is free software: you can redistribute it and/or modify
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
#include "rtc.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"
#include <string.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t rtc_open(driver_t **pdrv);
static void rtc_close(driver_t **pdrv);
static int32_t rtc_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static int32_t rtc_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(rtc, rtc_open, rtc_close, rtc_write, rtc_read, NULL, NULL);

/*---------- function ----------*/
static int32_t rtc_open(driver_t **pdrv)
{
    rtc_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        retval = (pdesc->init() ? CY_EOK : CY_ERROR);
    }

    return retval;
}

static void rtc_close(driver_t **pdrv)
{
    rtc_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

static int32_t rtc_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    rtc_describe_t *pdesc = NULL;
    int32_t retval = CY_ERROR;
    int32_t utc = -1;

    assert(pdrv && buf);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(len > sizeof(utc) || len == 0) {
            break;
        }
        memcpy(&utc, buf, len);
        if(pdesc && pdesc->set_utc) {
            retval = (pdesc->set_utc(utc) ? CY_EOK : CY_ERROR);
        }
    } while(0);

    return retval;
}

static int32_t rtc_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    rtc_describe_t *pdesc = NULL;
    int32_t retval = CY_ERROR;
    int32_t utc = -1;

    assert(pdrv && buf);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(len > sizeof(utc) || len == 0) {
            break;
        }
        if(pdesc && pdesc->get_utc) {
            utc = pdesc->get_utc();
        }
        if(utc < 0) {
            break;
        }
        memcpy(buf, &utc, len);
        retval = CY_EOK;
    } while(0);

    return retval;
}
