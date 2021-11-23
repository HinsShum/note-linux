/**
 * @file src\strategy.c
 *
 * Copyright (C) 2021
 *
 * strategy.c is free software: you can redistribute it and/or modify
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
#include "strategy.h"
#include "platform.h"
#include "simplefifo.h"
#include "flash.h"
#include "download.h"
#include "firmware.h"
#include "config/errorno.h"
#include "config/options.h"
#include <ctype.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _echo_usage(void);
static int32_t _erase_app_backup(void);
static int32_t _echo_info(void);
static int32_t _jump2app(void);
static int32_t _reboot(void);
static int32_t _ymodem(void);
static int32_t _update(void);

/*---------- type define ----------*/
typedef int32_t (*func_t)(void);
typedef struct {
    char ch;
    func_t cb;
} cb_t;

/*---------- variable ----------*/
static cb_t cb_array[] = {
    {'E', _erase_app_backup},
    {'I', _echo_info},
    {'Y', _ymodem},
    {'G', _jump2app},
    {'R', _reboot},
    {'U', _update},
    {' ', _echo_usage}
};
static const char *usage[] = {
    "Usage:",
    "\t[E|e] - Erase app backup space",
    "\t[I|i] - Echo app inforamtion",
    "\t[Y|y] - Start ymodem to transfer bin file",
    "\t[U|u] - Update app from backup",
    "\t[G|g] - Jump to app right now",
    "\t[R|r] - Reboot",
    "\0"
};

/*---------- function ----------*/
static bool __get_space_char(uint32_t ms)
{
    bool retval = false;
    uint8_t ch = 0;

    do {
        if(simplefifo_read(g_plat.dev.fifo, &ch, sizeof(ch))) {
            if(ch == ' ') {
                retval = true;
                break;
            }
        }
        __delay_ms(1);
        plat_wdt_feed();
    } while(--ms);

    return retval;
}

static bool _get_space_char(void)
{
    debug_info("BOOTME\r\n");
    simplefifo_reset(g_plat.dev.fifo);
    return __get_space_char(CONFIG_YBOOT_WAIT_SPACE_TIME);
}

static bool _get_char(char *ch, uint32_t ms)
{
    bool retval = false;

    do {
        if(simplefifo_read(g_plat.dev.fifo, (uint8_t *)ch, 1)) {
            retval = true;
            break;
        }
        __delay_ms(1);
    } while(--ms);

    return retval;
}

static int32_t _echo_usage(void)
{
    debug_info("CMD\r\n");
    for(char **pusage = (char **)usage; **pusage != '\0'; ++pusage) {
        debug_info("%s\r\n", *pusage);
    }

    return STRATEGY_ERR_OK;
}

static int32_t _erase_app_backup(void)
{
    int32_t retval = STRATEGY_ERR_OK;

    debug_message("Erase app backup: ");
    for(uint32_t addr = CONFIG_APP_BK_INFO_LOCATION; addr < CONFIG_APP_BK_LOCATION_END;) {
        int32_t erased = device_ioctl(g_plat.dev.backup_flash, IOCTL_FLASH_ERASE_BLOCK, &addr);
        if(erased <= 0) {
            retval = STRATEGY_ERR_FAILED;
            break;
        }
        addr += erased;
        debug_cont("#");
    }
    if(retval == STRATEGY_ERR_OK) {
        debug_message("\r\nOK\r\n");
    } else {
        debug_error("\r\nERROR\r\n");
    }
    firmware_clear_update_flag();

    return retval;
}

static int32_t _echo_info(void)
{
    debug_info("Yboot: V%s\r\n", SYS_VERSION_STRING);
    debug_info("Model: %s\r\n", SYS_MODEL_NAME);
    debug_info("Vendor: %s\r\n", SYS_VENDOR);
    debug_info("Build time: %s\r\n", SYS_PRODUCT_TIME);

    return STRATEGY_ERR_OK;
}

static int32_t _jump2app(void)
{
    return STRATEGY_ERR_JUMP;
}

static int32_t _reboot(void)
{
    return STRATEGY_ERR_REBOOT;
}

static int32_t _ymodem(void)
{
    int32_t retval = STRATEGY_ERR_OK;

    if(STRATEGY_ERR_OK == (retval = _erase_app_backup())) {
        if(CY_EOK != download_file()) {
            retval = STRATEGY_ERR_FAILED;
        }
    }

    return retval;
}

static int32_t _update(void)
{
    firmware_update();

    return STRATEGY_ERR_OK;
}

static int32_t _process(void)
{
    int32_t retval = STRATEGY_ERR_OK;
    char ch = 0;

    simplefifo_reset(g_plat.dev.fifo);
    _echo_usage();
    for(;;) {
        retval = STRATEGY_ERR_OK;
        plat_wdt_feed();
        if(_get_char(&ch, 5000)) {
            ch = toupper(ch);
            for(uint8_t i = 0; i < ARRAY_SIZE(cb_array); ++i) {
                if(cb_array[i].ch == ch) {
                    retval = cb_array[i].cb();
                    break;
                }
            }
            if(retval == STRATEGY_ERR_JUMP || retval == STRATEGY_ERR_REBOOT) {
                break;
            }
        } else {
            debug_warn("Get cmd timeout\r\n");
            break;
        }
    }

    return retval;
}

int32_t strategy_process(void)
{
    int32_t retval = STRATEGY_ERR_OK;

    do {
        if(!_get_space_char()) {
            break;
        }
        retval = _process();
    } while(0);

    return retval;
}
