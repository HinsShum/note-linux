/**
 * @file bsp/ds884/bsp_flash1.c
 *
 * Copyright (C) 2021
 *
 * bsp_flash1.c is free software: you can redistribute it and/or modify
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
#include "bsp_flash1.h"
#include "flash.h"
#include "gd32e23x.h"
#include "config/options.h"
#include <string.h>

/*---------- macro ----------*/
#define _IS_WRITE_ADDRESS_LEGAL(address)        ((address >= CONFIG_EMBED_FLASH_BASE) && \
                                                 (address < CONFIG_EMBED_FLASH_END) && \
                                                 ((address & 0x3) == 0))
#define _IS_READ_ADDRESS_LEGAL(address)         ((address >= CONFIG_EMBED_FLASH_BASE) && \
                                                 (address < CONFIG_EMBED_FLASH_END))

/* do callback
 */
#define _DO_CALLBACK(dev)                       do {                            \
                                                    if((&dev)->ops.cb) {        \
                                                        (&dev)->ops.cb();       \
                                                    }                           \
                                                } while(0)
#define _DO_LOCK(dev)                           do {                            \
                                                    if((&dev)->ops.lock) {      \
                                                        (&dev)->ops.lock();     \
                                                    }                           \
                                                } while(0)
#define _DO_UNLOCK(dev)                         do {                            \
                                                    if((&dev)->ops.unlock) {    \
                                                        (&dev)->ops.unlock();   \
                                                    }                           \
                                                } while(0)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static uint32_t bsp_write(const uint8_t *buf, uint32_t offset, uint32_t length);
static uint32_t bsp_read(uint8_t *buf, uint32_t offset, uint32_t length);
static uint32_t bsp_erase_block(uint32_t offset);
static bool bsp_block_start_check(uint32_t offset);

/*---------- type define ----------*/
/*---------- variable ----------*/
static flash_describe_t flash = {
    .start = CONFIG_EMBED_FLASH_BASE,
    .end = CONFIG_EMBED_FLASH_END,
    .block_size = CONFIG_EMBED_FLASH_BLOCK_SIZE,
    .ops.init = NULL,
    .ops.deinit = NULL,
    .ops.write = bsp_write,
    .ops.read = bsp_read,
    .ops.erase_block = bsp_erase_block,
    .ops.erase_chip = NULL,
    .ops.addr_is_block_start = bsp_block_start_check,
    .ops.cb = NULL
};
DEVICE_DEFINED(flash1, flash, &flash);

/*---------- function ----------*/
static void inline __fmc_clear_flag(void)
{
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);
}

static fmc_state_enum _fmc_erase_block(uint32_t address)
{
    fmc_state_enum retval = FMC_WPERR;

    fmc_unlock();
    __fmc_clear_flag();
    retval = fmc_page_erase(address);
    __fmc_clear_flag();
    fmc_lock();

    return retval;
}

static fmc_state_enum _fmc_word_program(uint32_t address, uint32_t data)
{
    fmc_state_enum retval = FMC_WPERR;

    retval = fmc_word_program(address, data);
    __fmc_clear_flag();

    return retval;
}

static uint32_t bsp_write(const uint8_t *buf, uint32_t offset, uint32_t length)
{
    uint32_t address = flash.start + offset;
    uint32_t act_len = 0;
    uint32_t data = 0;
    fmc_state_enum result = FMC_READY;
    uint8_t *p = (uint8_t *)buf;

    do {
        if(!buf) {
            break;
        }
        if(!_IS_WRITE_ADDRESS_LEGAL(address)) {
            break;
        }
        _DO_LOCK(flash);
        fmc_unlock();
        while(act_len < length) {
            _DO_CALLBACK(flash);
            memcpy(&data, p, sizeof(data));
            if(FMC_READY != _fmc_word_program(address, data)) {
                break;
            }
            act_len += sizeof(data);
            p += sizeof(data);
            address += sizeof(data);
        }
        if(act_len > length) {
            act_len = length;
        }
        fmc_lock();
        _DO_UNLOCK(flash);
    } while(0);

    return act_len;
}

static uint32_t bsp_read(uint8_t *buf, uint32_t offset, uint32_t length)
{
    uint32_t address = flash.start + offset;
    uint32_t act_len = 0;

    do {
        if(!buf) {
            break;
        }
        if(!_IS_READ_ADDRESS_LEGAL(address)) {
            break;
        }
        if((address + length) > flash.end) {
            act_len = flash.end - address;
        } else {
            act_len = length;
        }
        _DO_LOCK(flash);
        memcpy(buf, (void *)address, act_len);
        _DO_UNLOCK(flash);
        _DO_CALLBACK(flash);
    } while(0);

    return act_len;
}

static uint32_t bsp_erase_block(uint32_t offset)
{
    uint32_t address = flash.start + offset;
    uint32_t erased = 0;

    if(_IS_READ_ADDRESS_LEGAL(address)) {
        _DO_LOCK(flash);
        if(FMC_READY == _fmc_erase_block(address)) {
            erased = flash.block_size;
        }
        _DO_UNLOCK(flash);
    }
    _DO_CALLBACK(flash);

    return erased;
}

static bool bsp_block_start_check(uint32_t offset)
{
    uint32_t address = flash.start + offset;
    bool check = false;
    uint32_t blk_size = flash.block_size;

    if(_IS_READ_ADDRESS_LEGAL(address)) {
        check = ((address & (blk_size - 1)) == 0);
    }

    return check;
}
