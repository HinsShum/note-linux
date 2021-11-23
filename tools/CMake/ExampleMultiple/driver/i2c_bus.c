/**
 * @file driver\i2c_bus.c
 *
 * Copyright (C) 2021
 *
 * i2c_bus.c is free software: you can redistribute it and/or modify
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
#include "i2c_bus.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t i2c_bus_open(driver_t **pdrv);
static void i2c_bus_close(driver_t **pdrv);
static int32_t i2c_bus_write_bytes(driver_t **pdrv, void *pbuf, uint32_t addition, uint32_t len);
static int32_t i2c_bus_read_bytes(driver_t **pdrv, void *pbuf, uint32_t addition, uint32_t len);
static int32_t i2c_bus_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/* private ioctl functions
 */
static int32_t _ioctl_set_lock_handler(i2c_bus_describe_t *pdesc, void *args);
static int32_t _ioctl_set_unlock_handler(i2c_bus_describe_t *pdesc, void *args);
static int32_t _ioctl_set_user_data(i2c_bus_describe_t *pdesc, void *args);
static int32_t _ioctl_lock(i2c_bus_describe_t *pdesc, void *args);
static int32_t _ioctl_unlock(i2c_bus_describe_t *pdesc, void *args);

static int32_t _i2c_bus_random_read_bytes(i2c_bus_describe_t *pdesc, i2c_bus_msg_t *pmsg);
static int32_t _i2c_bus_sequential_read_bytes(i2c_bus_describe_t *pdesc, i2c_bus_msg_t *pmsg);
static int32_t _i2c_bus_write_bytes(i2c_bus_describe_t *pdesc, i2c_bus_msg_t *pmsg);

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(i2c_bus_describe_t *pdesc, void *args);
typedef struct {
    uint32_t ioctl_cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

typedef int32_t (*type_cb_func_t)(i2c_bus_describe_t *pdesc, i2c_bus_msg_t *pmsg);
typedef struct {
    i2c_bus_type_t type;
    type_cb_func_t cb;
} type_cb_t;

/*---------- variable ----------*/
DRIVER_DEFINED(i2c_bus, i2c_bus_open, i2c_bus_close, i2c_bus_write_bytes, i2c_bus_read_bytes, i2c_bus_ioctl, NULL);

static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_I2C_BUS_SET_LOCK_HANDLER, _ioctl_set_lock_handler},
    {IOCTL_I2C_BUS_SET_UNLOCK_HANDER, _ioctl_set_unlock_handler},
    {IOCTL_I2C_BUS_SET_USER_DATA, _ioctl_set_user_data},
    {IOCTL_I2C_BUS_LOCK, _ioctl_lock},
    {IOCTL_I2C_BUS_UNLOCK, _ioctl_unlock}
};

static type_cb_t type_cb_array[] = {
    {I2C_BUS_TYPE_RANDOM_READ, _i2c_bus_random_read_bytes},
    {I2C_BUS_TYPE_SEQUENTIAL_READ, _i2c_bus_sequential_read_bytes},
    {I2C_BUS_TYPE_WRITE, _i2c_bus_write_bytes}
};

/*---------- function ----------*/
static bool _i2c_bus_start(i2c_bus_describe_t *pdesc)
{
    bool retval = false;

    if(pdesc->ops.sda_get()) {
        pdesc->ops.sda_set(false);
        pdesc->ops.delay();
        pdesc->ops.scl_set(false);
        retval = true;
    } else {
        __debug_error("I2C bus detect the SDA signal is not high when i2c bus start\n");
    }

    return retval;
}

static void _i2c_bus_stop(i2c_bus_describe_t *pdesc)
{
    pdesc->ops.scl_set(false);
    pdesc->ops.delay();
    pdesc->ops.sda_set(false);
    pdesc->ops.delay();
    pdesc->ops.scl_set(true);
    pdesc->ops.delay();
    pdesc->ops.sda_set(true);
    pdesc->ops.delay();
}

static inline bool _i2c_wait_ack(i2c_bus_describe_t *pdesc)
{
    bool ack = false;

    pdesc->ops.sda_set(true);
    pdesc->ops.delay();
    pdesc->ops.scl_set(true);
    pdesc->ops.delay();
    ack = !pdesc->ops.sda_get();
    pdesc->ops.scl_set(false);

    return ack;
}

static bool _i2c_bus_write_byte(i2c_bus_describe_t *pdesc, uint8_t byte)
{
    for(int8_t i = 7; i >= 0; --i) {
        pdesc->ops.scl_set(false);
        pdesc->ops.sda_set(!!((byte >> i) & 0x01));
        pdesc->ops.delay();
        pdesc->ops.scl_set(true);
        pdesc->ops.delay();
    }
    pdesc->ops.scl_set(false);

    return _i2c_wait_ack(pdesc);
}

static uint8_t _i2c_bus_read_byte(i2c_bus_describe_t *pdesc)
{
    uint8_t data = 0;

    pdesc->ops.sda_set(true);
    pdesc->ops.delay();
    for(uint8_t i = 0; i < 8; ++i) {
        data <<= 1;
        pdesc->ops.scl_set(true);
        pdesc->ops.delay();
        data |= pdesc->ops.sda_get();
        pdesc->ops.scl_set(false);
        pdesc->ops.delay();
    }

    return data;
}

static bool _i2c_bus_send_ack(i2c_bus_describe_t *pdesc, bool ack)
{
    pdesc->ops.sda_set(!ack);
    pdesc->ops.delay();
    pdesc->ops.scl_set(true);
    pdesc->ops.delay();
    pdesc->ops.scl_set(false);

    return true;
}

static bool _i2c_bus_send_address(i2c_bus_describe_t *pdesc, uint8_t address)
{
    bool retval = false;

    if(true == _i2c_bus_start(pdesc)) {
        retval = _i2c_bus_write_byte(pdesc, address);
    }

    return retval;
}

static int32_t _i2c_bus_ops_check(i2c_bus_describe_t *pdesc)
{
    int32_t retval = CY_EOK;

    if(!pdesc->ops.scl_set || !pdesc->ops.scl_get || !pdesc->ops.sda_set || !pdesc->ops.sda_set || !pdesc->ops.delay) {
        retval = CY_E_POINT_NONE;
        __debug_error("I2C bus ops not complete\n");
    }

    return retval;
}

static int32_t i2c_bus_open(driver_t **pdrv)
{
    i2c_bus_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("I2C bus has no describe field\n");
            break;
        }
        if(CY_EOK != (retval = _i2c_bus_ops_check(pdesc))) {
            break;
        }
        retval = CY_EOK;
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                retval = CY_ERROR;
                break;
            }
        }
        pdesc->ops.scl_set(true);
        pdesc->ops.sda_set(true);
    } while(0);

    return retval;
}

static void i2c_bus_close(driver_t **pdrv)
{
    i2c_bus_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t __i2c_bus_write_memory_address(i2c_bus_describe_t *pdesc, i2c_bus_msg_t *pmsg)
{
    int32_t retval = CY_ERROR;

    do {
        if(true != _i2c_bus_send_address(pdesc, pmsg->dev_addr & 0xFE)) {
            __debug_error("Send i2c bus address(%02X) failed\n", pmsg->dev_addr);
            break;
        }
        retval = CY_EOK;
        if(!pmsg->mem_addr) {
            break;
        }
        /* send memory address */
        for(uint32_t i = 0; i < pmsg->mem_addr_counts; ++i) {
            if(true != _i2c_bus_write_byte(pdesc, pmsg->mem_addr[i])) {
                __debug_error("I2C bus write memory address failed\n");
                retval = CY_ERROR;
                break;
            }
        }
    } while(0);

    return retval;
}

static int32_t _i2c_bus_random_read_bytes(i2c_bus_describe_t *pdesc, i2c_bus_msg_t *pmsg)
{
    int32_t retval = CY_ERROR;
    uint32_t length = pmsg->len;
    uint8_t *pbuf = pmsg->buf;

    do {
        if(CY_EOK != (retval = __i2c_bus_write_memory_address(pdesc, pmsg))) {
            __debug_error("I2C bus random read bytes failed\n");
            break;
        }
        pdesc->ops.scl_set(true);
        pdesc->ops.delay();
        if(true != _i2c_bus_send_address(pdesc, pmsg->dev_addr | 0x01)) {
            __debug_error("Send i2c bus address(%02X) failed\n", pmsg->dev_addr);
            retval = CY_ERROR;
            break;
        }
        while(length > 0) {
            --length;
            *pbuf++ = _i2c_bus_read_byte(pdesc);
            _i2c_bus_send_ack(pdesc, length != 0);
        }
    } while(0);
    _i2c_bus_stop(pdesc);

    return retval;
}

static int32_t _i2c_bus_sequential_read_bytes(i2c_bus_describe_t *pdesc, i2c_bus_msg_t *pmsg)
{
    int32_t retval = CY_ERROR;
    uint32_t length = pmsg->len;
    uint8_t *pbuf = pmsg->buf;

    do {
        if(true != _i2c_bus_send_address(pdesc, pmsg->dev_addr | 0x01)) {
            __debug_error("Send i2c bus address(%02X) failed\n", pmsg->dev_addr);
            break;
        }
        retval = CY_EOK;
        while(length > 0) {
            --length;
            *pbuf++ = _i2c_bus_read_byte(pdesc);
            _i2c_bus_send_ack(pdesc, length != 0);
        }
    } while(0);
    _i2c_bus_stop(pdesc);

    return retval;
}

static int32_t _i2c_bus_write_bytes(i2c_bus_describe_t *pdesc, i2c_bus_msg_t *pmsg)
{
    int32_t retval = CY_ERROR;
    uint32_t length = pmsg->len;
    uint8_t *pbuf = pmsg->buf;

    do {
        if(CY_EOK != (retval = __i2c_bus_write_memory_address(pdesc, pmsg))) {
            __debug_error("I2C bus write bytes failed\n");
            break;
        }
        retval = CY_ERROR;
        while(length > 0) {
            if(true != _i2c_bus_write_byte(pdesc, *pbuf++)) {
                __debug_error("I2C bus write data failed, not get the right ack\n");
                break;
            }
            --length;
        }
        if(!length) {
            retval = CY_EOK;
        }
    } while(0);
    _i2c_bus_stop(pdesc);

    return retval;
}

static type_cb_func_t _type_cb_func_find(i2c_bus_type_t type)
{
    type_cb_func_t cb = NULL;

    for(uint32_t i = 0; i < ARRAY_SIZE(type_cb_array); ++i) {
        if(type_cb_array[i].type == type) {
            cb = type_cb_array[i].cb;
            break;
        }
    }

    return cb;
}

static int32_t i2c_bus_read_bytes(driver_t **pdrv, void *pbuf, uint32_t addition, uint32_t len)
{
    i2c_bus_msg_t *pmsg = (i2c_bus_msg_t *)pbuf;
    i2c_bus_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    type_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("I2C bus has no describe field\n");
            break;
        }
        if(len != sizeof(i2c_bus_msg_t) || !pmsg || !pmsg->buf) {
            __debug_error("I2C bus read bytes parameters error\n");
            break;
        }
        if(NULL == (cb = _type_cb_func_find(pmsg->type))) {
            __debug_error("I2C bus not support this type(%08X)\n", pmsg->type);
            break;
        }
        retval = cb(pdesc, pmsg);
    } while(0);

    return retval;
}

static int32_t i2c_bus_write_bytes(driver_t **pdrv, void *pbuf, uint32_t addition, uint32_t len)
{
    i2c_bus_msg_t *pmsg = (i2c_bus_msg_t *)pbuf;
    i2c_bus_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    type_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("I2C bus has no describe field\n");
            break;
        }
        if(len != sizeof(i2c_bus_msg_t) || !pmsg || !pmsg->buf) {
            __debug_error("I2C bus write bytes parameters error\n");
            break;
        }
        if(NULL == (cb = _type_cb_func_find(pmsg->type))) {
            __debug_error("I2C bus not support this type(%08X)\n", pmsg->type);
            break;
        }
        retval = cb(pdesc, pmsg);
    } while(0);

    return retval;
}

static int32_t _ioctl_set_lock_handler(i2c_bus_describe_t *pdesc, void *args)
{
    pdesc->ops.lock = (void (*)(void *))args;

    return CY_EOK;
}

static int32_t _ioctl_set_unlock_handler(i2c_bus_describe_t *pdesc, void *args)
{
    pdesc->ops.unlock = (void (*)(void *))args;

    return CY_EOK;
}

static int32_t _ioctl_set_user_data(i2c_bus_describe_t *pdesc, void *args)
{
    pdesc->data = args;

    return CY_EOK;
}

static int32_t _ioctl_lock(i2c_bus_describe_t *pdesc, void *args)
{
    if(pdesc->ops.lock) {
        pdesc->ops.lock(pdesc->data);
    }

    return CY_EOK;
}

static int32_t _ioctl_unlock(i2c_bus_describe_t *pdesc, void *args)
{
    if(pdesc->ops.unlock) {
        pdesc->ops.unlock(pdesc->data);
    }

    return CY_EOK;
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

static int32_t i2c_bus_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    i2c_bus_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("I2C bus has no describe field\n");
            break;
        }
        if(NULL == (cb = _ioctl_cb_func_find(cmd))) {
            __debug_error("I2C bus not support this command(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}
