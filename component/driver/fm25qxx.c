/**
 * @file driver/fm25qxx.c
 *
 * Copyright (C) 2021
 *
 * fm25qxx.c is free software: you can redistribute it and/or modify
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
#include "fm25qxx.h"
#include "driver.h"
#include "config/options.h"
#include "config/errorno.h"

/*---------- macro ----------*/
#define FM25QXX_MANUFACTURER_ID                 (0xA1)
#define FM25QXX_DEVICE_ID_02                    (0x11)
#define FM25QXX_DEVICE_ID_04                    (0x12)
#define FM25QXX_DEVICE_ID_08                    (0x13)
#define FM25QXX_DEVICE_ID_16                    (0x14)
#define FM25QXX_DEVICE_ID_32                    (0x15)
#define FM25QXX_DEVICE_ID_64                    (0x16)
#define FM25QXX_DEVICE_ID_128                   (0x17)

#define FM25QXX_READ_STATUS_WIP                 (1 << 0)
#define FM25QXX_READ_STATUS_WEL                 (1 << 1)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t fm25qxx_open(driver_t **pdrv);
static void fm25qxx_close(driver_t **pdrv);
static int32_t fm25qxx_write(driver_t **pdrv, void *buf, uint32_t addr, uint32_t len);
static int32_t fm25qxx_read(driver_t **pdrv, void *buf, uint32_t addr, uint32_t len);
static int32_t fm25qxx_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(fm25qxx, fm25qxx_open, fm25qxx_close, fm25qxx_write, fm25qxx_read, fm25qxx_ioctl, NULL);
static flash_info_t fm25qxx_serials[] = {
    [0] = {
        .start = 0,
        .end = 0x40000,
        .page_size = 0x100,
        .pages = 0x400,
        .sector_size = 0x1000,
        .sectors = 0x40,
        .minimum_erase_size = 0x1000
    },
    [1] = {
        .start = 0,
        .end = 0x80000,
        .page_size = 0x100,
        .pages = 0x800,
        .sector_size = 0x1000,
        .sectors = 0x80,
        .minimum_erase_size = 0x1000
    },
    [2] = {
        .start = 0,
        .end = 0x100000,
        .page_size = 0x100,
        .pages = 0x1000,
        .sector_size = 0x1000,
        .sectors = 0x100,
        .minimum_erase_size = 0x1000
    },
    [3] = {
        .start = 0,
        .end = 0x200000,
        .page_size = 0x100,
        .pages = 0x2000,
        .sector_size = 0x1000,
        .sectors = 0x200,
        .minimum_erase_size = 0x1000
    },
    [4] = {
        .start = 0,
        .end = 0x400000,
        .page_size = 0x100,
        .pages = 0x4000,
        .sector_size = 0x1000,
        .sectors = 0x400,
        .minimum_erase_size = 0x1000
    },
    [5] = {
        .start = 0,
        .end = 0x800000,
        .page_size = 0x100,
        .pages = 0x8000,
        .sector_size = 0x1000,
        .sectors = 0x800,
        .minimum_erase_size = 0x1000
    },
    [6] = {
        .start = 0,
        .end = 0x1000000,
        .page_size = 0x100,
        .pages = 0x10000,
        .sector_size = 0x1000,
        .sectors = 0x1000,
        .minimum_erase_size = 0x1000
    }
};

/*---------- function ----------*/
static void fm25qxx_getid(fm25qxx_describe_t *pdesc)
{
    pdesc->idcode = 0;
    pdesc->cs_ctrl(true);
    pdesc->xfer(0x90);
    pdesc->xfer(0x00);
    pdesc->xfer(0x00);
    pdesc->xfer(0x00);
    pdesc->idcode = ((uint16_t)pdesc->xfer(0x00)) << 8;
    pdesc->idcode |= pdesc->xfer(0x00);
    pdesc->cs_ctrl(false);
}

static uint8_t fm25qxx_get_read_status(fm25qxx_describe_t *pdesc, uint8_t reg_addr)
{
    uint8_t status = 0;

    pdesc->cs_ctrl(true);
    pdesc->xfer(reg_addr);
    status = pdesc->xfer(0x00);
    pdesc->cs_ctrl(false);

    return status;
}

static int32_t fm25qxx_write_enable(fm25qxx_describe_t *pdesc)
{
    int32_t timeout = 100;
    uint8_t status = 0;
    int32_t retval = CY_E_TIME_OUT;

    pdesc->cs_ctrl(true);
    pdesc->xfer(0x06);
    pdesc->cs_ctrl(false);

    do {
        status = fm25qxx_get_read_status(pdesc, 0x05);
        if(status & FM25QXX_READ_STATUS_WEL) {
            retval = CY_EOK;
            break;
        }
        __delay_ms(1);
    } while(--timeout > 0);

    return retval;
}

static int32_t fm25qxx_erase_sector(fm25qxx_describe_t *pdesc, uint32_t addr)
{
    int32_t timeout = 400;
    int32_t retval = CY_E_TIME_OUT;
    uint8_t status = 0;

    do {
        if(CY_EOK != fm25qxx_write_enable(pdesc)) {
            break;
        }
        pdesc->cs_ctrl(true);
        pdesc->xfer(0x20);
        pdesc->xfer((uint8_t)((addr >> 16) & 0xFF));
        pdesc->xfer((uint8_t)((addr >> 8) & 0xFF));
        pdesc->xfer((uint8_t)((addr >> 0) & 0xFF));
        pdesc->cs_ctrl(false);
        do {
            status = fm25qxx_get_read_status(pdesc, 0x05);
            if(0 == (status & FM25QXX_READ_STATUS_WIP)) {
                retval = CY_EOK;
                break;
            }
            __delay_ms(1);
        } while(--timeout > 0);
    } while(0);

    return retval;
}

static int32_t fm25qxx_erase_chip(fm25qxx_describe_t *pdesc)
{
    uint8_t status = 0;
    int32_t retval = CY_E_TIME_OUT;
    int32_t timeout = 3000;

    do {
        if(CY_EOK != fm25qxx_write_enable(pdesc)) {
            break;
        }
        pdesc->cs_ctrl(true);
        pdesc->xfer(0xC7);
        pdesc->cs_ctrl(false);
        do {
            status = fm25qxx_get_read_status(pdesc, 0x05);
            if(0 == (status & FM25QXX_READ_STATUS_WIP)) {
                retval = CY_EOK;
                break;
            }
            __delay_ms(1);
        } while(--timeout > 0);
    } while(0);

    return retval;
}

static int32_t fm25qxx_write_page(fm25qxx_describe_t *pdesc, uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint16_t page_left = 0x00;
    uint8_t status = 0;
    int32_t timeout = 50;
    uint32_t length = 0;

    do {
        if((addr + len) > pdesc->flash.info.end) {
            break;
        }
        page_left = pdesc->flash.info.page_size - (addr % pdesc->flash.info.page_size);
        if(page_left < len) {
            len = page_left;
        }
        if(CY_EOK != fm25qxx_write_enable(pdesc)) {
            break;
        }
        pdesc->cs_ctrl(true);
        pdesc->xfer(0x02);
        pdesc->xfer((uint8_t)((addr >> 16) & 0xFF));
        pdesc->xfer((uint8_t)((addr >> 8) & 0xFF));
        pdesc->xfer((uint8_t)((addr >> 0) & 0xFF));
        for(uint32_t i = 0; i < len; ++i) {
            pdesc->xfer(buf[i]);
        }
        pdesc->cs_ctrl(false);
        do {
            status = fm25qxx_get_read_status(pdesc, 0x05);
            if(0 == (status & FM25QXX_READ_STATUS_WIP)) {
                length = len;
                break;
            }
            __delay_ms(1);
        } while(--timeout > 0);
    } while(0);

    return (int32_t)length;
}

static int32_t fm25qxx_open(driver_t **pdrv)
{
    fm25qxx_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->flash.init) {
        retval = (pdesc->flash.init() ? CY_EOK : CY_ERROR);
        if(CY_EOK == retval) {
            fm25qxx_getid(pdesc);
            do {
                uint8_t device_id = (pdesc->idcode & 0xFF);
                if(((pdesc->idcode >> 8) & 0xFF) != FM25QXX_MANUFACTURER_ID) {
                    retval = CY_ERROR;
                    break;
                }
                if(device_id < FM25QXX_DEVICE_ID_02 || device_id > FM25QXX_DEVICE_ID_128) {
                    retval = CY_ERROR;
                    break;
                }
                pdesc->flash.info = fm25qxx_serials[device_id - FM25QXX_DEVICE_ID_02];
            } while(0);
            if(CY_EOK != retval) {
                pdesc->flash.deinit();
            }
        }
    }

    return retval;
}

static void fm25qxx_close(driver_t **pdrv)
{
    fm25qxx_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->flash.deinit) {
        pdesc->flash.deinit();
    }
}

static int32_t fm25qxx_write(driver_t **pdrv, void *buf, uint32_t addr, uint32_t len)
{
    uint8_t *pdata = (uint8_t *)buf;
    fm25qxx_describe_t *pdesc = NULL;
    uint32_t length = 0;

    assert(pdrv);
    assert(buf);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            break;
        }
        if((addr + len) > pdesc->flash.info.end) {
            break;
        }
        do {
            int32_t result = fm25qxx_write_page(pdesc, addr + length, &pdata[length], (len - length));
            if(0 == result) {
                break;
            }
            length += result;
        } while(length < len);
    } while(0);

    return (int32_t)length;
}

static int32_t fm25qxx_read(driver_t **pdrv, void *buf, uint32_t addr, uint32_t len)
{
    uint32_t length = 0;
    fm25qxx_describe_t *pdesc = NULL;
    uint8_t *pdata = (uint8_t *)buf;

    assert(pdrv);
    assert(buf);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            break;
        }
        if(addr > pdesc->flash.info.end) {
            break;
        }
        if((addr + len) > pdesc->flash.info.end) {
            length = pdesc->flash.info.end - addr;
        } else {
            length = len;
        }
        pdesc->cs_ctrl(true);
        pdesc->xfer(0x03);
        pdesc->xfer((uint8_t)((addr >> 16) & 0xFF));
        pdesc->xfer((uint8_t)((addr >> 8) & 0xFF));
        pdesc->xfer((uint8_t)((addr >> 0) & 0xFF));
        for(uint32_t i = 0; i < length; ++i) {
            pdata[i] = pdesc->xfer(0x00);
        }
        pdesc->cs_ctrl(false);
    } while(0);

    return (int32_t)length;
}

static int32_t fm25qxx_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    fm25qxx_describe_t *pdesc = NULL;
    int32_t retval = CY_ERROR;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_FLASH_ERASE_PAGE:
            retval = CY_E_WRONG_ARGS;
            break;
        case IOCTL_FLASH_ERASE_MINIMUM_SIZE:
        case IOCTL_FLASH_ERASE_SECTOR:
            if(pdesc) {
                retval = fm25qxx_erase_sector(pdesc, *(uint32_t *)args);
            }
            break;
        case IOCTL_FLASH_ERASE_CHIP:
            if(pdesc) {
                retval = fm25qxx_erase_chip(pdesc);
            }
            break;
        case IOCTL_FLASH_GET_INFO:
            if(pdesc) {
                *(flash_info_t *)args = pdesc->flash.info;
                retval = CY_EOK;
            }
            break;
        case IOCTL_FM25QXX_GET_IDCODE:
            if(pdesc) {
                *(uint16_t *)args = pdesc->idcode;
                retval = CY_EOK;
            }
            break;
        default:
            retval = CY_E_WRONG_ARGS;
            break;
    }

    return retval;
}
