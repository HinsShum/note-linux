/**
 * @file driver/flash.c
 *
 * Copyright (C) 2021
 *
 * flash.c is free software: you can redistribute it and/or modify
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
#include "flash.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t flash_open(driver_t **pdrv);
static void flash_close(driver_t **pdrv);
static int32_t flash_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static int32_t flash_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static int32_t flash_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(flash, flash_open, flash_close, flash_write, flash_read, flash_ioctl, NULL);

/*---------- function ----------*/
static int32_t flash_open(driver_t **pdrv)
{
    flash_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        retval = (pdesc->init() ? CY_EOK : CY_ERROR);
    }

    return retval;
}

static void flash_close(driver_t **pdrv)
{
    flash_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

static int32_t flash_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    flash_describe_t *pdesc = NULL;
    uint32_t actual_len = 0;

    assert(pdrv);
    assert(buf);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->write) {
        actual_len = pdesc->write(buf, addition, len);
    }

    return (int32_t)actual_len;
}

static int32_t flash_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    flash_describe_t *pdesc = NULL;
    uint32_t actual_len = 0;

    assert(pdrv);
    assert(buf);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->read) {
        actual_len = pdesc->read(buf, addition, len);
    }

    return (int32_t)actual_len;
}

static int32_t flash_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    flash_describe_t *pdesc = NULL;
    int32_t retval = CY_ERROR;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_FLASH_ERASE_MINIMUM_SIZE:
        case IOCTL_FLASH_ERASE_PAGE:
            if(pdesc && pdesc->erase_page) {
                retval = (pdesc->erase_page(*(uint32_t *)args) ? CY_EOK : CY_ERROR);
            }
            break;
        case IOCTL_FLASH_ERASE_SECTOR:
            if(pdesc && pdesc->erase_sector) {
                retval = (pdesc->erase_sector(*(uint32_t *)args) ? CY_EOK : CY_ERROR);
            }
        case IOCTL_FLASH_ERASE_CHIP:
            if(pdesc && pdesc->erase_chip) {
                retval = (pdesc->erase_chip() ? CY_EOK : CY_ERROR);
            }
            break;
        case IOCTL_FLASH_GET_INFO:
            if(pdesc) {
                *(flash_info_t *)args = pdesc->info;
                retval = CY_EOK;
            }
        default: 
            retval = CY_E_WRONG_ARGS;
            break;
    }

    return retval;
}
