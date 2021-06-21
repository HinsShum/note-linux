/**
 * @file driver/serial.c
 *
 * Copyright (C) 2021
 *
 * serial.c is free software: you can redistribute it and/or modify
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
#include "serial.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t serial_open(driver_t **pdrv);
static void serial_close(driver_t **pdrv);
static int32_t serial_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static int32_t serial_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t serial_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len);

/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(serial, serial_open, serial_close, serial_write, NULL, serial_ioctl, serial_irq_handler);

/*---------- function ----------*/
static int32_t serial_open(driver_t **pdrv)
{
    serial_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        retval = (pdesc->init() ? CY_EOK : CY_ERROR);
    }

    return retval;
}

static void serial_close(driver_t **pdrv)
{
    serial_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

static int32_t serial_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    serial_describe_t *pdesc = NULL;
    uint16_t actual_len = 0;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->dir_change && addition == SERIAL_WIRTE_CHANGE_DIR_AUTOMATICALLY) {
        pdesc->dir_change(SERIAL_DIRECTION_TX);
    }
    if(pdesc && pdesc->write) {
        actual_len = pdesc->write((uint8_t *)buf, (uint16_t)len);
    }
    if(pdesc && pdesc->dir_change && addition == SERIAL_WIRTE_CHANGE_DIR_AUTOMATICALLY) {
        pdesc->dir_change(SERIAL_DIRECTION_RX);
    }

    return (int32_t)actual_len;
}

static int32_t serial_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    serial_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_SERIAL_GET_COMPORT:
            if(pdesc) {
                *(uint8_t *)args = pdesc->comport;
            }
            break;
        case IOCTL_SERIAL_SET_IRQ_HANDLER:
            if(pdesc) {
                pdesc->irq_handler = (int32_t (*)(uint32_t, void *, uint32_t))args;
            }
            break;
        case IOCTL_SERIAL_DIRECTION_CHOOSE:
            if(pdesc && pdesc->dir_change && args) {
                serial_direction_en dir = *(serial_direction_en *)args;
                if(dir > SERIAL_DIRECTION_NRX_NTX) {
                    retval = CY_E_WRONG_ARGS;
                } else {
                    pdesc->dir_change(dir);
                }
            }
            break;
        case IOCTL_SERIAL_GET_BAUDRATE:
            if(pdesc && args) {
                *(uint32_t *)args = pdesc->baudrate;
            }
            break;
        case IOCTL_SERIAL_SET_BAUDRATE:
            if(pdesc && args) {
                uint32_t baudrate = *(uint32_t *)args;
                if(pdesc->baudrate != baudrate) {
                    pdesc->baudrate = baudrate;
                    if(pdesc->deinit) {
                        pdesc->deinit();
                    }
                    if(pdesc->init) {
                        retval = (pdesc->init() ? CY_EOK : CY_ERROR);
                    }
                }
            }
            break;
        default:
            retval = CY_E_WRONG_ARGS;
            break;
    }

    return retval;
}

static int32_t serial_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len)
{
    serial_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->irq_handler) {
        retval = pdesc->irq_handler(irq_handler, args, len);
    }

    return retval;
}
