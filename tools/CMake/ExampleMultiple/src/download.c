/**
 * @file src\download.c
 *
 * Copyright (C) 2021
 *
 * download.c is free software: you can redistribute it and/or modify
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
#include "download.h"
#include "simplefifo.h"
#include "platform.h"
#include "ymodem.h"
#include "md5.h"
#include "flash.h"
#include "firmware.h"
#include "config/errorno.h"
#include "config/options.h"
#include <string.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static ymodem_context_t _ymodem_ctx;
static uint8_t _buf[CONFIG_RECV_BUFFER_SIZE];
static uint32_t fsize;
static struct st_md5_ctx md5_ctx;
static uint8_t md5_val[16];

/*---------- function ----------*/
static bool _getc(uint8_t *ch, uint32_t ms)
{
    bool retval = false;

    do {
        if(simplefifo_read(g_plat.dev.fifo, ch, 1)) {
            retval = true;
            break;
        }
        plat_wdt_feed();
        __delay_ms(1);
    } while(--ms);

    return retval;
}

static bool _putc(const uint8_t ch)
{
    debug_cont("%c", (char)ch);

    return true;
}

static bool _file_verify(const char *filename, uint32_t filesize)
{
    bool retval = false;

    do {
        if(strstr(filename, SYS_MODEL_NAME) == NULL) {
            __debug_warn("File name verify failed\n");
            break;
        }
        if(filesize > CONFIG_APP_MAX_SIZE) {
            __debug_warn("File size is too large, allow size is %dBytes\n", CONFIG_APP_MAX_SIZE);
            break;
        }
        fsize = filesize;
        retval = true;
    } while(0);

    return retval;
}

static bool _frame_save(uint32_t offset, const uint8_t *frame, uint32_t size)
{
    bool retval = false;
    uint32_t addr = CONFIG_APP_BK_LOCATION_BASE + offset;

    do {
        if(!offset) {
            md5_init(&md5_ctx);
        }
        md5_update(&md5_ctx, (uint8_t *)frame, size);
        if(size != device_write(g_plat.dev.backup_flash, (void *)frame, addr, size)) {
            break;
        }
        retval = true;
        if((offset + size) == fsize) {
            md5_final(&md5_ctx, md5_val);
        }
    } while(0);

    return retval;
}

int32_t download_file(void)
{
    int32_t retval = CY_ERROR;

    memset(&_ymodem_ctx, 0, sizeof(_ymodem_ctx));
    _ymodem_ctx.pbuf = _buf;
    _ymodem_ctx.size = CONFIG_RECV_BUFFER_SIZE;
    _ymodem_ctx.get_char = _getc;
    _ymodem_ctx.putc = _putc;
    _ymodem_ctx.frame_save = _frame_save;
    _ymodem_ctx.file_verify = _file_verify;
    debug_message("Start YMODEM...\r\n");
    simplefifo_reset(g_plat.dev.fifo);
    ymodem_init(&_ymodem_ctx);
    if(YMODEM_ERR_OVER == ymodem_recv_file()) {
        retval = firmware_update_info(fsize, md5_val, false);
    }
    (retval == CY_EOK) ? debug_message("\r\nOK\r\n") : debug_error("\r\nERROR\r\n");
    simplefifo_reset(g_plat.dev.fifo);

    return retval;
}
