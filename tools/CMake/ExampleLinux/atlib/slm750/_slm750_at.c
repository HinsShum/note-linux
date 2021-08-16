/**
 * @file atlib/slm750/_slm750_at.c
 *
 * Copyright (C) 2019
 *
 * _slm750_at.c is free software: you can redistribute it and/or modify
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
 */

/*---------- includes ----------*/
#include "_slm750_at.h"
#include "includes.h"
#include "ryat_if.h"
#include <ctype.h>

/*---------- macro ----------*/
#ifndef RYAT_DEBUG_ENABLE
#define RYAT_DEBUG_ENABLE (0)
#endif

#ifndef __ryat_debug
#if (RYAT_DEBUG_ENABLE > 0)
#include <stdlib.h>
#define __ryat_debug(x, ...)    debug_info(x, ##__VA_ARGS__)
#else
#define __ryat_debug(x, ...)
#endif
#endif

#define STATE_GET_READ_PACKAGE_HEAD     (0)
#define STATE_GET_SOCKET_ID             (1)
#define STATE_GET_LOCAL_IP              (2)
#define STATE_GET_LOCAL_PORT            (3)
#define STATE_GET_DATA                  (4)
#define STATE_END                       (5)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
struct st_ryat_ret_describe const ryat_ret_tbl_slm750[] = {
    { RYAT_RET_OK, "OK", 2 },
    { RYAT_RET_ERROR, "ERROR", 5 },
    { RYAT_RET_CME, "+CME", 4 },
    { RYAT_RET_CSQ, "+CSQ", 4 },
    { RYAT_RET_GMM, "+GMM", 4 },
    { RYAT_RET_COPS, "+COPS", 5 },
    { RYAT_RET_CCID, "ICCID", 5 },
    { RYAT_RET_MIPCALL, "+MIPCALL", 8 },
    { RYAT_RET_MIPOPEN, "+MIPOPEN", 8 },
    { RYAT_RET_MIPCLOSE, "+MIPCLOSE", 9 },
    { RYAT_RET_CELL_INFO, " LAC_ID", 7 },
    { RYAT_RET_AT_P, "AT+", 3 },
    { RYAT_RET_P, "+", 1 },
    { RYAT_RET_NULL, NULL, 0}
};

/*---------- function ----------*/
int32_t _slm750_at_get_data(struct st_ryat_describe *pat, uint8_t channel, uint8_t *pdata, uint16_t len, uint32_t timeout)
{
    int16_t tick = __ms_to_tick(timeout);
    char ch = 0;
    ryat_rx_line_t *pcurrent = &pat->cache.line[0];
    uint8_t state = STATE_GET_READ_PACKAGE_HEAD;
    char fmt[3] = {0};
    int16_t reallen = 0;

    ryat_reset_cache(pat);
    // +MIPRUDP=1,10.2.113.184,33259,494D4F4B
    // OK
    while(tick > 0 && state != STATE_END) {
        if(pat->port_read_char(&ch)) {
            switch(state) {
                case STATE_GET_READ_PACKAGE_HEAD: {
                    if(ch == '+' || pcurrent->len != 0) {
                        pcurrent->buf[pcurrent->len++] = ch;
                    }
                    if(pcurrent->len == strlen("+MIPRUDP=")) {
                        if(!strncmp(pcurrent->buf, "+MIPRUDP", strlen("+MIPRUDP"))) {
                            state = STATE_GET_SOCKET_ID;
                            pcurrent++;
                        } else {
                            pcurrent->len = 0;
                        }
                    } else if(pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        pcurrent->len = 0;
                        pcurrent->buf[0] = 0x00;
                    }
                    break;
                }
                case STATE_GET_SOCKET_ID: {
                    pcurrent->buf[pcurrent->len++] = ch;
                    if(ch == ',') {
                        pcurrent->buf[pcurrent->len - 1] = '\0';
                        __ryat_debug("Socket id: %s\r\n", pcurrent->buf);
                        state = STATE_GET_LOCAL_IP;
                        pcurrent++;
                    } else if(ch == '\r' || ch == '\n' || ch == '\0' ||
                              pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        __ryat_debug("_slm750_at_get_data() get socket id failed\r\n");
                        return RYAT_E_ERROR;
                    }
                    break;
                }
                case STATE_GET_LOCAL_IP: {
                    pcurrent->buf[pcurrent->len++] = ch;
                    if(ch == ',') {
                        pcurrent->buf[pcurrent->len - 1] = '\0';
                        __ryat_debug("Local ip: %s\r\n", pcurrent->buf);
                        state = STATE_GET_LOCAL_PORT;
                        pcurrent++;
                    } else if(ch == '\r' || ch == '\n' || ch == '\0' ||
                              pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        __ryat_debug("_slm750_at_get_data() get local ip failed\r\n");
                        return RYAT_E_ERROR;
                    }
                    break;
                }
                case STATE_GET_LOCAL_PORT: {
                    pcurrent->buf[pcurrent->len++] = ch;
                    if(ch == ',') {
                        pcurrent->buf[pcurrent->len - 1] = '\0';
                        __ryat_debug("Local port: %s\r\n", pcurrent->buf);
                        state = STATE_GET_DATA;
                    } else if(ch == '\r' || ch == '\n' || ch == '\0' ||
                              pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        __ryat_debug("_slm750_at_get_data() get local port failed\r\n");
                        return RYAT_E_ERROR;
                    }
                    break;
                }
                case STATE_GET_DATA: {
                    if(isalnum(ch)) {
                        uint32_t tmp = 0;
                        uint8_t fmt_index = reallen % 2;

                        fmt[fmt_index] = ch;
                        if(fmt_index) {
                            sscanf(fmt, "%02X", &tmp);
                            pdata[reallen / 2] = (uint8_t)tmp;
                        }
                        reallen++;
                    } else {
                        state = STATE_END;
                    }
                    break;
                }
                default : break;
            }
        } else {
            tick--;
            pat->delay_tick(1);
        }
    }
    if(tick <= 0) {
        reallen = RYAT_E_ERROR;
    } else {
        reallen /= 2;
    }

    return reallen;
}
