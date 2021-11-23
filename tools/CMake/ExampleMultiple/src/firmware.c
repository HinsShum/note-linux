/**
 * @file src\firmware.c
 *
 * Copyright (C) 2021
 *
 * firmware.c is free software: you can redistribute it and/or modify
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
#include "firmware.h"
#include "flash.h"
#include "platform.h"
#include "checksum.h"
#include "utils.h"
#include "md5.h"
#include "config/errorno.h"
#include "config/options.h"
#include <string.h>

/*---------- macro ----------*/
#define FIRMWARE_INFO_MAGIC                         (0x00769394)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
typedef struct {
    uint32_t magic;
    uint32_t fsize;
    uint8_t md5[16];
    bool updated;
    uint8_t reserve[5];
    uint16_t crc16;
} firmware_info_t;

/*---------- variable ----------*/
static firmware_info_t _firmware;

/*---------- function ----------*/
static void _set_default_information(firmware_info_t *info)
{
    info->magic = FIRMWARE_INFO_MAGIC;
    info->fsize = 0;
    info->updated = true;
    info->crc16 = 0;
    memset(info->md5, 0, ARRAY_SIZE(info->md5));
    memset(info->reserve, 0, ARRAY_SIZE(info->reserve));
}

int32_t firmware_init(void)
{
    int32_t retval = CY_ERROR;

    do {
        /* get app backup information from flash */
        if(!device_read(g_plat.dev.backup_flash, &_firmware, CONFIG_APP_BK_INFO_LOCATION, sizeof(_firmware))) {
            break;
        }
        /* check information */
        if(_firmware.magic != FIRMWARE_INFO_MAGIC) {
            break;
        }
        if(checksum_crc16_xmodem((void *)&_firmware, sizeof(_firmware))) {
            break;
        }
        retval = CY_EOK;
    } while(0);
    if(retval != CY_EOK) {
        _set_default_information(&_firmware);
    }
#ifndef NDEBUG
    __debug_info("App size: %dBytes\n", _firmware.fsize);
    __debug_info("App md5: ");
    for(uint8_t i = 0; i < ARRAY_SIZE(_firmware.md5); ++i) {
        __debug_cont("%02X", _firmware.md5[i]);
    }
    __debug_cont("\n");
    __debug_info("App updated: %s\n", _firmware.updated ? "true" : "false");
#endif

    return retval;
}

bool firmware_get_updated_flag(void)
{
    return _firmware.updated;
}

void firmware_clear_update_flag(void)
{
    _firmware.updated = true;
}

int32_t firmware_update_info(uint32_t fsize, uint8_t *md5, bool updated)
{
    int32_t retval = CY_ERROR;
    uint32_t addr = CONFIG_APP_BK_INFO_LOCATION;

    if(device_ioctl(g_plat.dev.backup_flash, IOCTL_FLASH_ERASE_BLOCK, &addr)) {
        _firmware.magic = FIRMWARE_INFO_MAGIC;
        _firmware.fsize = fsize;
        memcpy(_firmware.md5, md5, ARRAY_SIZE(_firmware.md5));
        _firmware.updated = updated;
        memset(_firmware.reserve, 0, ARRAY_SIZE(_firmware.reserve));
        _firmware.crc16 = utils_htons(checksum_crc16_xmodem((void *)&_firmware, offsetof(firmware_info_t, crc16)));
        if(sizeof(_firmware) == device_write(g_plat.dev.backup_flash, (void *)&_firmware, CONFIG_APP_BK_INFO_LOCATION, sizeof(_firmware))) {
            retval = CY_EOK;
        } else {
            debug_warn("Save firmware information failed\n");
        }
    }

    return retval;
}

static int32_t _erase(void)
{
    int32_t retval = CY_EOK;

    for(uint32_t addr = CONFIG_APP_LOCATION_BASE; addr < (CONFIG_APP_LOCATION_BASE + CONFIG_APP_MAX_SIZE);) {
        uint32_t erased = device_ioctl(g_plat.dev.embed_flash, IOCTL_FLASH_ERASE_BLOCK, &addr);
        if(erased <= 0) {
            retval = CY_ERROR;
            debug_error("\r\nERROR\r\n");
            break;
        }
        addr += erased;
        debug_cont("#");
    }

    return retval;
}

static int32_t _update(void)
{
    uint32_t fsize = _firmware.fsize;
    uint32_t offset = 0;
    uint8_t cp[32] = {0};
    uint32_t cp_len = 0;
    int32_t retval = CY_ERROR;
    uint8_t md5[16] = {0};
    struct st_md5_ctx ctx = {0};

    md5_init(&ctx);
    debug_message("Start update firmware");
    if(_erase() == CY_EOK) {
        do {
            if((offset + ARRAY_SIZE(cp)) < fsize) {
                cp_len = ARRAY_SIZE(cp);
            } else {
                cp_len = fsize - offset;
            }
            /* copy firmware from backup flash to embed flash */
            if(cp_len != device_read(g_plat.dev.backup_flash, cp, CONFIG_APP_BK_LOCATION_BASE + offset, cp_len)) {
                debug_error("\r\nERROR\r\nRead firmware from backup failed, offset=%08X\r\n", offset);
                break;
            }
            md5_update(&ctx, cp, cp_len);
            if(cp_len != device_write(g_plat.dev.embed_flash, cp, CONFIG_APP_LOCATION_BASE + offset, cp_len)) {
                debug_error("\r\nERROR\r\nWrite firmware to embed flash failed, offset=%08X\r\n", offset);
                break;
            }
            debug_cont(".");
            offset += cp_len;
            if(offset == fsize) {
                md5_final(&ctx, md5);
                if(memcmp(md5, _firmware.md5, ARRAY_SIZE(md5)) == 0) {
                    debug_message("\r\nOK\r\n");
                    retval = firmware_update_info(fsize, md5, true);
                } else {
                    debug_error("\r\nERROR\r\nCheck md5 failed\r\n");
                }
            }
        } while(offset < fsize);
    }

    return retval;
}

int32_t firmware_update(void)
{
    int32_t retval = CY_EOK;

    do {
        if(_firmware.updated == true) {
            debug_message("No new firmware exit\r\n");
            break;
        }
        retval = _update();
    } while(0);

    return retval;
}
