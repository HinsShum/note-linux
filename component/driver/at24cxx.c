/**
 * @file driver\at24cxx.c
 *
 * Copyright (C) 2021
 *
 * at24cxx.c is free software: you can redistribute it and/or modify
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
#include "at24cxx.h"
#include "i2c_bus.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"
#include <string.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t at24cxx_open(driver_t **pdrv);
static void at24cxx_close(driver_t **pdrv);
static int32_t at24cxx_write(driver_t **pdrv, void *buf, uint32_t offset, uint32_t length);
static int32_t at24cxx_read(driver_t **pdrv, void *buf, uint32_t offset, uint32_t length);
static int32_t at24cxx_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/* private ioctl functions
 */
static int32_t _ioctl_erase_block(at24cxx_describe_t *pdesc, void *args);
static int32_t _ioctl_erase_chip(at24cxx_describe_t *pdesc, void *args);
static int32_t _ioctl_check_addr_is_block_start(at24cxx_describe_t *pdesc, void *args);
static int32_t _ioctl_get_info(at24cxx_describe_t *pdesc, void *args);
static int32_t _ioctl_set_callback(at24cxx_describe_t *pdesc, void *args);

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(at24cxx_describe_t *pdesc, void *args);
typedef struct {
    uint32_t ioctl_cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable ----------*/
DRIVER_DEFINED(at24cxx, at24cxx_open, at24cxx_close, at24cxx_write, at24cxx_read, at24cxx_ioctl, NULL);

static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_FLASH_ERASE_BLOCK, _ioctl_erase_block},
    {IOCTL_FLASH_ERASE_CHIP, _ioctl_erase_chip},
    {IOCTL_FLASH_CHECK_ADDR_IS_BLOCK_START, _ioctl_check_addr_is_block_start},
    {IOCTL_FLASH_GET_INFO, _ioctl_get_info},
    {IOCTL_FLASH_SET_CALLBACK, _ioctl_set_callback}
};

/*---------- function ----------*/
static int32_t at24cxx_open(driver_t **pdrv)
{
    at24cxx_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    void *bus = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("AT24CXX driver has no describe field\n");
            break;
        }
        retval = CY_EOK;
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                __debug_error("AT24CXX initialize failed\n");
                retval = CY_ERROR;
                break;
            }
        }
        /* bind to i2c bus */
        if(NULL == (bus = device_open(pdesc->bus_name))) {
            __debug_error("AT24CXX bind i2c bus failed\n");
            if(pdesc->ops.deinit) {
                pdesc->ops.deinit();
            }
            retval = CY_ERROR;
            break;
        }
        pdesc->bus = bus;
    } while(0);

    return retval;
}

static void at24cxx_close(driver_t **pdrv)
{
    at24cxx_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc) {
        if(pdesc->bus) {
            device_close(pdesc->bus);
            pdesc->bus = NULL;
        }
        if(pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }
    }
}

static void inline _do_callback(at24cxx_describe_t *pdesc)
{
    if(pdesc->ops.cb) {
        pdesc->ops.cb();
    }
}

static int32_t at24cxx_write(driver_t **pdrv, void *buf, uint32_t offset, uint32_t length)
{
    i2c_bus_msg_t msg = {0};
    uint32_t actual_len = 0;
    at24cxx_describe_t *pdesc = NULL;
    int32_t result = CY_EOK;
    uint8_t memory_addr[2] = {0};
    uint32_t address = 0;

    assert(pdrv);
    assert(buf);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("AT24CXX driver has no describe field\n");
            break;
        }
        if(!pdesc->bus) {
            __debug_error("AT24CXX not bind to i2c bus\n");
            break;
        }
        msg.type = I2C_BUS_TYPE_WRITE;
        msg.dev_addr = pdesc->address;
        msg.mem_addr = memory_addr;
        msg.mem_addr_counts = pdesc->mem_addr_counts;
        if(pdesc->ops.write_protect_set) {
            pdesc->ops.write_protect_set(false);
        }
        address = pdesc->info.start + offset;
        if(address >= pdesc->info.end) {
            __debug_error("AT24CXX write address is overflow\n");
            length = 0;
        } else if((address + length) > pdesc->info.end) {
            length = pdesc->info.end - address;
            __debug_warn("AT24CXX write address plus length is overflow, it only can write %d bytes\n", length);
        }
        while(actual_len < length) {
            if(pdesc->mem_addr_counts == 1) {
                memory_addr[0] = address & 0xFF;
            } else {
                memory_addr[0] = (address >> 8) & 0xFF;
                memory_addr[1] = address & 0xFF;
            }
            msg.buf = (buf + actual_len);
            if(((address + (length - actual_len)) & (~(pdesc->info.block_size - 1))) !=
               (address & (~(pdesc->info.block_size - 1)))) {
                msg.len = ((address + pdesc->info.block_size) & (~(pdesc->info.block_size - 1))) - address;
            } else {
                msg.len = length - actual_len;
            }
            device_ioctl(pdesc->bus, IOCTL_I2C_BUS_LOCK, NULL);
            result = device_write(pdesc->bus, &msg, 0, sizeof(msg));
            device_ioctl(pdesc->bus, IOCTL_I2C_BUS_UNLOCK, NULL);
            _do_callback(pdesc);
            if(CY_EOK != result) {
                __debug_error("AT24CXX write failed\n");
                break;
            }
            actual_len += msg.len;
            address += msg.len;
            __delay_ms(10);
        }
    } while(0);
    if(pdesc->ops.write_protect_set) {
        pdesc->ops.write_protect_set(true);
    }

    return (int32_t)actual_len;
}

static int32_t at24cxx_read(driver_t **pdrv, void *buf, uint32_t offset, uint32_t length)
{
    at24cxx_describe_t *pdesc = NULL;
    int32_t result = CY_EOK;
    uint32_t actual_len = 0;
    i2c_bus_msg_t msg = {0};
    uint8_t memory_addr[2] = {0};
    uint32_t address = 0;

    assert(pdrv);
    assert(buf);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("AT24CXX driver has no describe field\n");
            break;
        }
        if(!pdesc->bus) {
            __debug_error("AT24CXX not bind to i2c bus\n");
            break;
        }
        msg.type = I2C_BUS_TYPE_RANDOM_READ;
        msg.dev_addr = pdesc->address;
        msg.mem_addr = memory_addr;
        msg.mem_addr_counts = pdesc->mem_addr_counts;
        address = pdesc->info.start + offset;
        if(address >= pdesc->info.end) {
            __debug_error("AT24CXX read address is overflow\n");
            length = 0;
            break;
        } else if((address + length) > pdesc->info.end) {
            length = pdesc->info.end - address;
            __debug_warn("AT24CXX read address plus length is overflow, it only can read %d bytes\n", length);
        }
        if(pdesc->mem_addr_counts == 1) {
            memory_addr[0] = address & 0xFF;
        } else {
            memory_addr[0] = (address >> 8) & 0xFF;
            memory_addr[1] = address & 0xFF;
        }
        msg.buf = buf;
        msg.len = length;
        _do_callback(pdesc);
        device_ioctl(pdesc->bus, IOCTL_I2C_BUS_LOCK, NULL);
        if(CY_EOK == device_read(pdesc->bus, &msg, 0, sizeof(msg))) {
            actual_len = length;
        }
        device_ioctl(pdesc->bus, IOCTL_I2C_BUS_UNLOCK, NULL);
    } while(0);

    return (int32_t)actual_len;
}

static int32_t _ioctl_erase_block(at24cxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *poffset = (uint32_t *)args;
    uint32_t addr = 0;
    uint8_t buf[64] = {0};
    i2c_bus_msg_t msg = {0};
    uint8_t memory_addr[2] = {0};

    do {
        if(!args) {
            __debug_error("Args is NULL, erase block function must specify the erase address\n");
            break;
        }
        memset(buf, 0xFF, sizeof(buf));
        addr = ((*poffset + pdesc->info.start) / pdesc->info.block_size) * pdesc->info.block_size;
        if(pdesc->mem_addr_counts == 1) {
            memory_addr[0] = addr & 0xFF;
        } else {
            memory_addr[0] = (addr >> 8) & 0xFF;
            memory_addr[1] = addr & 0xFF;
        }
        msg.type = I2C_BUS_TYPE_WRITE;
        msg.dev_addr = pdesc->address;
        msg.mem_addr = memory_addr;
        msg.mem_addr_counts = pdesc->mem_addr_counts;
        msg.buf = buf;
        msg.len = pdesc->info.block_size;
        if(pdesc->ops.write_protect_set) {
            pdesc->ops.write_protect_set(false);
        }
        device_ioctl(pdesc->bus, IOCTL_I2C_BUS_LOCK, NULL);
        retval = device_write(pdesc->bus, &msg, 0, sizeof(msg));
        device_ioctl(pdesc->bus, IOCTL_I2C_BUS_UNLOCK, NULL);
        _do_callback(pdesc);
        if(CY_EOK != retval) {
            __debug_error("AT24CXX erase block failed\n");
            break;
        }
        retval = (int32_t)pdesc->info.block_size;
        __debug_info("Erase address(%08X) block size: %dbytes\n", addr, pdesc->info.block_size);
    } while(0);
    if(pdesc->ops.write_protect_set) {
        pdesc->ops.write_protect_set(true);
    }

    return retval;
}

static int32_t _ioctl_erase_chip(at24cxx_describe_t *pdesc, void *args)
{
    uint32_t addr = pdesc->info.start;
    int32_t retval = CY_ERROR;
    uint8_t buf[64] = {0};
    i2c_bus_msg_t msg = {0};
    uint8_t memory_addr[2] = {0};

    if(pdesc->ops.write_protect_set) {
        pdesc->ops.write_protect_set(false);
    }
    memset(buf, 0xFF, ARRAY_SIZE(buf));
    while(addr < pdesc->info.end) {
        if(pdesc->mem_addr_counts == 1) {
            memory_addr[0] = addr & 0xFF;
        } else {
            memory_addr[0] = (addr >> 8) & 0xFF;
            memory_addr[1] = addr & 0xFF;
        }
        msg.type = I2C_BUS_TYPE_WRITE;
        msg.dev_addr = pdesc->address;
        msg.mem_addr = memory_addr;
        msg.mem_addr_counts = pdesc->mem_addr_counts;
        msg.buf = buf;
        msg.len = pdesc->info.block_size;
        device_ioctl(pdesc->bus, IOCTL_I2C_BUS_LOCK, NULL);
        retval = device_write(pdesc->bus, &msg, 0, sizeof(msg));
        device_ioctl(pdesc->bus, IOCTL_I2C_BUS_UNLOCK, NULL);
        _do_callback(pdesc);
        if(CY_EOK != retval) {
            __debug_error("AT24CXX erase chip failed, %08X address occur error\n", addr);
            break;
        }
        __debug_info("Erase chip, current address: %08X\n", addr);
        addr += msg.len;
    }
    if(pdesc->ops.write_protect_set) {
        pdesc->ops.write_protect_set(true);
    }
    if(addr >= pdesc->info.end) {
        retval = (int32_t)(pdesc->info.end - pdesc->info.start);
    }

    return retval;
}

static int32_t _ioctl_check_addr_is_block_start(at24cxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *poffset = (uint32_t *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, can not check the addr\n");
            break;
        }
        if(((*poffset + pdesc->info.start) % pdesc->info.block_size) == 0) {
            retval = CY_EOK;
        } else {
            retval = CY_ERROR;
        }
    } while(0);

    return retval;
}

static int32_t _ioctl_get_info(at24cxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    flash_info_t *pinfo = (flash_info_t *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, no memory to store at24cxx information\n");
            break;
        }
        pinfo->start = pdesc->info.start;
        pinfo->end = pdesc->info.end;
        pinfo->block_size = pdesc->info.block_size;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_set_callback(at24cxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    void (*cb)(void) = (void (*)(void))args;

    do {
        if(!args) {
            __debug_error("Args is NULL, no callback to bind the at24cxx device\n");
            break;
        }
        pdesc->ops.cb = cb;
        retval = CY_EOK;
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

static int32_t at24cxx_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    at24cxx_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("AT24CXX has no describe field\n");
            break;
        }
        if(NULL == (cb = _ioctl_cb_func_find(cmd))) {
            __debug_error("AT24CXX not support this command(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}
