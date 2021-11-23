/**
 * @file bsp\win32\bsp_flash2.c
 *
 * Copyright (C) 2021
 *
 * bsp_flash.c is free software: you can redistribute it and/or modify
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
#include "bsp_flash.h"
#include "flash.h"
#include "config/options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------- macro ----------*/
#define _FLASH_FILE_NAME                        "flash.bin"

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
static bool bsp_init(void);
static void bsp_deinit(void);
static uint32_t bsp_write(const uint8_t *buf, uint32_t offset, uint32_t length);
static uint32_t bsp_read(uint8_t *buf, uint32_t offset, uint32_t length);
static uint32_t bsp_erase_block(uint32_t offset);
static bool bsp_block_start_check(uint32_t offset);

/*---------- type define ----------*/
/*---------- variable ----------*/
static FILE *pfile = NULL;
static flash_describe_t flash = {
    .start = CONFIG_EMBED_FLASH_BASE,
    .end = CONFIG_EMBED_FLASH_END,
    .block_size = CONFIG_EMBED_FLASH_BLOCK_SIZE,
    .ops.init = bsp_init,
    .ops.deinit = bsp_deinit,
    .ops.write = bsp_write,
    .ops.read = bsp_read,
    .ops.erase_block = bsp_erase_block,
    .ops.erase_chip = NULL,
    .ops.addr_is_block_start = bsp_block_start_check,
    .ops.cb = NULL,
    .ops.lock = NULL,
    .ops.unlock = NULL
};
DEVICE_DEFINED(flash2, flash, &flash);

/*---------- function ----------*/
static bool bsp_init(void)
{
    uint8_t *pbuf = malloc(CONFIG_EMBED_FLASH_END - CONFIG_EMBED_FLASH_BASE);

    if(!pbuf) {
        __debug_error("malloc memory for flash init faield\n");
        exit(EXIT_FAILURE);
    }
    memset(pbuf, 0xFF, CONFIG_EMBED_FLASH_END - CONFIG_EMBED_FLASH_BASE);
    pfile = fopen(_FLASH_FILE_NAME, "rb");
    if(pfile) {
        fread(pbuf, CONFIG_EMBED_FLASH_END - CONFIG_EMBED_FLASH_BASE, 1, pfile);
        fclose(pfile);
    }
    pfile = fopen(_FLASH_FILE_NAME, "wb+");
    if(!pfile) {
        __debug_error("Open embed flash failed\n");
        free(pbuf);
        exit(EXIT_FAILURE);
    }
    fwrite(pbuf, CONFIG_EMBED_FLASH_END - CONFIG_EMBED_FLASH_BASE, 1, pfile);
    free(pbuf);

    return true;
}

static void bsp_deinit(void)
{
    if(pfile) {
        fclose(pfile);
        pfile = NULL;
    }
}

static uint32_t bsp_write(const uint8_t *buf, uint32_t offset, uint32_t length)
{
    uint32_t act_len = 0;
    uint32_t addr = CONFIG_EMBED_FLASH_BASE + offset;

    do {
        if(!buf) {
            __debug_warn("Args error, can not write data to flash\n");
            break;
        }
        if(!pfile) {
            break;
        }
        if((addr + length) > CONFIG_EMBED_FLASH_END) {
            act_len = CONFIG_EMBED_FLASH_END - addr;
        } else {
            act_len = length;
        }
        _DO_LOCK(flash);
        fseek(pfile, addr, SEEK_SET);
        fwrite(buf, act_len, 1, pfile);
        fflush(pfile);
        _DO_UNLOCK(flash);
        _DO_CALLBACK(flash);
    } while(0);

    return act_len;
}

static uint32_t bsp_read(uint8_t *buf, uint32_t offset, uint32_t length)
{
    uint32_t act_len = 0;
    uint32_t addr = CONFIG_EMBED_FLASH_BASE + offset;

    do {
        if(!buf || !pfile) {
            break;
        }
        if((addr + length) > CONFIG_EMBED_FLASH_END) {
            act_len = CONFIG_EMBED_FLASH_END - addr;
        } else {
            act_len = length;
        }
        _DO_LOCK(flash);
        fseek(pfile, addr, SEEK_SET);
        act_len = fread(buf, act_len, 1, pfile) * act_len;
        _DO_UNLOCK(flash);
        _DO_CALLBACK(flash);
    } while(0);

    return act_len;
}

static uint32_t bsp_erase_block(uint32_t offset)
{
    uint32_t addr = CONFIG_EMBED_FLASH_BASE + offset;
    uint8_t erase = 0xFF;
    uint32_t length = 0;

    addr = (addr / CONFIG_EMBED_FLASH_BLOCK_SIZE) * CONFIG_EMBED_FLASH_BLOCK_SIZE;
    do {
        if(!pfile) {
            break;
        }
        if(addr >= CONFIG_EMBED_FLASH_END) {
            break;
        }
        _DO_LOCK(flash);
        fseek(pfile, addr, SEEK_SET);
        for(uint32_t i = 0; i < CONFIG_EMBED_FLASH_BLOCK_SIZE; ++i) {
            fwrite(&erase, 1, 1, pfile);
        }
        fflush(pfile);
        _DO_UNLOCK(flash);
        _DO_CALLBACK(flash);
        length = CONFIG_EMBED_FLASH_BLOCK_SIZE;
    } while(0);

    return length;
}

static bool bsp_block_start_check(uint32_t offset)
{
    uint32_t addr = CONFIG_EMBED_FLASH_BASE + offset;
    bool check = false;
    uint32_t blksize = CONFIG_EMBED_FLASH_BLOCK_SIZE;

    if(addr < CONFIG_EMBED_FLASH_END) {
        check = ((addr & (blksize - 1)) == 0);
    }

    return check;
}
