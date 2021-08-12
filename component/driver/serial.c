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

/* private ioctl functions
 */
static int32_t _ioctl_serial_get_comport(serial_describe_t *pdesc, void *args);
static int32_t _ioctl_serial_set_irq_handler(serial_describe_t *pdesc, void *args);
static int32_t _ioctl_serial_set_direction(serial_describe_t *pdesc, void *args);
static int32_t _ioctl_serial_get_baudrate(serial_describe_t *pdesc, void *args);
static int32_t _ioctl_serial_set_baudrate(serial_describe_t *pdesc, void *args);

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(serial_describe_t *pdesc, void *args);
typedef struct {
    uint32_t ioctl_cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable ----------*/
DRIVER_DEFINED(serial, serial_open, serial_close, serial_write, NULL, serial_ioctl, serial_irq_handler);

/* define serial ioctl functions
 */
static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_SERIAL_GET_COMPORT, _ioctl_serial_get_comport},
    {IOCTL_SERIAL_SET_IRQ_HANDLER, _ioctl_serial_set_irq_handler},
    {IOCTL_SERIAL_SET_DIRECTION, _ioctl_serial_set_direction},
    {IOCTL_SERIAL_GET_BAUDRATE, _ioctl_serial_get_baudrate},
    {IOCTL_SERIAL_SET_BAUDRATE, _ioctl_serial_set_baudrate}
};

/*---------- function ----------*/
static int32_t serial_open(driver_t **pdrv)
{
    serial_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Serial device has no describe field\n");
            break;
        }
        retval = CY_EOK;
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                retval = CY_ERROR;
            }
        }
    } while(0);

    return retval;
}

static void serial_close(driver_t **pdrv)
{
    serial_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t serial_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    serial_describe_t *pdesc = NULL;
    uint16_t actual_len = 0;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.dir_change && addition == SERIAL_WIRTE_CHANGE_DIR_AUTOMATICALLY) {
        pdesc->ops.dir_change(SERIAL_DIRECTION_TX);
    }
    if(pdesc && pdesc->ops.write) {
        actual_len = pdesc->ops.write((uint8_t *)buf, (uint16_t)len);
    }
    if(pdesc && pdesc->ops.dir_change && addition == SERIAL_WIRTE_CHANGE_DIR_AUTOMATICALLY) {
        pdesc->ops.dir_change(SERIAL_DIRECTION_RX);
    }

    return (int32_t)actual_len;
}

static int32_t _ioctl_serial_get_comport(serial_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint8_t *comport = (uint8_t *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, no memory to store the comport\n");
            break;
        }
        *comport = pdesc->comport;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_serial_set_irq_handler(serial_describe_t *pdesc, void *args)
{
    int32_t (*irq)(uint32_t, void *, uint32_t) = (int32_t (*)(uint32_t, void *, uint32_t))args;

    pdesc->ops.irq_handler = irq;

    return CY_EOK;
}

static int32_t _ioctl_serial_set_direction(serial_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    serial_direction_en *pdir = (serial_direction_en *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, can not set direction\n");
            break;
        }
        if(!pdesc->ops.dir_change) {
            __debug_error("Serial device has no set direction ops\n");
            break;
        }
        if(*pdir > SERIAL_DIRECTION_NRX_NTX) {
            __debug_error("Serial direction set command(%08X) not support\n", *pdir);
            break;
        }
        pdesc->ops.dir_change(*pdir);
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_serial_get_baudrate(serial_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *pbaudrate = (uint32_t *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, no memory to store the baudrate\n");
            break;
        }
        *pbaudrate = pdesc->baudrate;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_serial_set_baudrate(serial_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *pbaudrate = (uint32_t *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, can not set the baudrate\n");
            break;
        }
        retval = CY_EOK;
        if(*pbaudrate == pdesc->baudrate) {
            __debug_message("Baudrate no need to change\n");
            break;
        }
        pdesc->baudrate = *pbaudrate;
        if(pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                retval = CY_ERROR;
            }
        }
    } while(0);

    return retval;
}

static ioctl_cb_func_t _ioctl_cb_func_find(uint32_t ioctl_cmd)
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

static int32_t serial_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    serial_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("Serial device has no describe field\n");
            break;
        }
        if(NULL == (cb = _ioctl_cb_func_find(cmd))) {
            __debug_error("Serial dirver not support this command(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}

static int32_t serial_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len)
{
    serial_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.irq_handler) {
        retval = pdesc->ops.irq_handler(irq_handler, args, len);
    }

    return retval;
}
