/**
 * @file bsp/cy001/bsp_embed_flash.c
 *
 * Copyright (C) 2021
 *
 * bsp_embed_flash.c is free software: you can redistribute it and/or modify
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
#include "bsp_embed_flash.h"
#include "flash.h"
#include "stm32f1xx.h"
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
static uint32_t bsp_write(const uint8_t *buf, uint32_t offset, uint32_t length)
{
    uint32_t addr = flash.start + offset;
    uint32_t act_len = 0;
    uint32_t data = 0;
    uint8_t *p = (uint8_t *)buf;

    do {
        if(!buf) {
            break;
        }
        if(!_IS_WRITE_ADDRESS_LEGAL(addr)) {
            break;
        }
        _DO_LOCK(flash);
        HAL_FLASH_Unlock();
        while(act_len < length) {
            _DO_CALLBACK(flash);
            memcpy(&data, p, sizeof(data));
            if(HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data)) {
                break;
            }
            act_len += sizeof(data);
            p += sizeof(data);
            addr += sizeof(data);
        }
        if(act_len > length) {
            act_len = length;
        }
        HAL_FLASH_Lock();
        _DO_UNLOCK(flash);
    } while(0);

    return act_len;
}

static uint32_t bsp_read(uint8_t *buf, uint32_t offset, uint32_t length)
{
    uint32_t addr = flash.start + offset;
    uint32_t act_len = 0;

    do {
        if(!buf) {
            break;
        }
        if(!_IS_READ_ADDRESS_LEGAL(addr)) {
            break;
        }
        if((addr + length) > flash.end) {
            act_len = flash.end - addr;
        } else {
            act_len = length;
        }
        _DO_LOCK(flash);
        memcpy(buf, (void *)addr, act_len);
        _DO_UNLOCK(flash);
        _DO_CALLBACK(flash);
    } while(0);

    return act_len;
}

static uint32_t bsp_erase_block(uint32_t offset)
{
    uint32_t addr = flash.start + offset;
    uint32_t erased = 0;
    FLASH_EraseInitTypeDef erase_init = {0};
    uint32_t error = 0;

    if(_IS_READ_ADDRESS_LEGAL(addr)) {
        _DO_LOCK(flash);
        HAL_FLASH_Unlock();
        erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
        erase_init.PageAddress = addr;
        erase_init.NbPages = 1;
        if(HAL_OK == HAL_FLASHEx_Erase(&erase_init, &error)) {
            erased = flash.block_size;
        }
        HAL_FLASH_Lock();
        _DO_UNLOCK(flash);
    }
    _DO_CALLBACK(flash);

    return erased;
}

static bool bsp_block_start_check(uint32_t offset)
{
    uint32_t addr = flash.start + offset;
    bool check = false;
    uint32_t blk_size = flash.block_size;

    if(_IS_READ_ADDRESS_LEGAL(addr)) {
        check = ((addr & (blk_size - 1)) == 0);
    }

    return check;
}
