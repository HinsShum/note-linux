/**
 * @file atlib/l620/_l620_at.c
 *
 * Copyright (C) 2020
 *
 * _l620_at.c is free software: you can redistribute it and/or modify
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
#include "_l620_at.h"
#include "includes.h"
#include "ryat_if.h"
#include <string.h>

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

#define L620_STATE_GET_READ_PACKAGE_HEAD         (0)
#define L620_STATE_GET_READ_DATA                 (1)
#define L620_STATE_END                           (2)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
struct st_ryat_ret_describe const ryat_ret_tbl_l620[] = {
    { RYAT_RET_OK, "OK", 2 },
    { RYAT_RET_ERROR, "ERROR", 5 },
    { RYAT_RET_CME, "+CME", 4 },
    { RYAT_RET_CGSN, "+CGSN", 5 },
    { RYAT_RET_CCID, "*MICCID", 7 },
    { RYAT_RET_CSQ, "+CSQ", 4 },
    { RYAT_RET_COPS, "+COPS", 5 },
    { RYAT_RET_IP, "+IP", 3 },
    { RYAT_RET_CON_OK, "CONNECT OK", 10 },
    { RYAT_RET_CON_FAIL, "CONNECT FAIL", 12 },
    { RYAT_RET_SENDOK, "SEND OK", 7 },
    { RYAT_RET_CLOSE_OK, "CLOSE OK", 8 },
    { RYAT_RET_CTM2M_ERROR, "+CTM2M ERROR", 12},
    { RYAT_RET_CTM2M_REG, "+CTM2M:obsrv", 12},
    { RYAT_RET_NULL, NULL, 0 }
};

/*---------- function ----------*/
int32_t _l620_at_get_data(struct st_ryat_describe *pat, uint8_t channel, uint8_t *pdata, uint16_t len, uint32_t timeout) 
{
    int16_t tick = __ms_to_tick(timeout);
    char ch = 0;
    ryat_rx_line_t *pcurrent = &pat->cache.line[0];
    int16_t reallen = 0;
    uint8_t state = L620_STATE_GET_READ_PACKAGE_HEAD;
    ryat_utils_args_t args = {0};

    ryat_reset_cache(pat);
    /* +RECEIVE:0,4
     * IMOK
     */
    while(tick > 0 && state != L620_STATE_END) {
        if(pat->port_read_char(&ch)) {
            switch(state) {
                case L620_STATE_GET_READ_PACKAGE_HEAD:
                    if((ch == '\r') || (ch == '\n') || (ch == '\0')) {
                        if(pcurrent->len != 0) {
                            pcurrent->buf[pcurrent->len] = '\0';
                            __ryat_debug("AT-line:%s\n", pcurrent->buf);
                            if(strstr(pcurrent->buf, "+RECEIVE")) {
                                if(RYAT_E_OK == ryat_utils_get_args(pcurrent->buf, pcurrent->len, &args) &&
                                   args.pnext != NULL &&
                                   RYAT_E_OK == ryat_utils_get_args(args.pnext, strlen(args.pnext), &args) &&
                                   args.pnext != NULL &&
                                   RYAT_E_OK == ryat_utils_get_args(args.pnext, strlen(args.pnext), &args)) {
                                    reallen = strtoul(args.pval, NULL, 10);
                                    if(len < reallen) {
                                        reallen = len;
                                    }
                                    len = 0;
                                    state = L620_STATE_GET_READ_DATA;
                                }
                            }
                            memset(pcurrent->buf, 0, sizeof(pcurrent->buf));
                            pcurrent->len = 0;
                        }
                    } else {
                        if(pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                            __ryat_debug("The line is too long\n");
                            memset(pcurrent->buf, 0, sizeof(pcurrent->buf));
                            pcurrent->len = 0;
                            continue;
                        }
                        pcurrent->buf[pcurrent->len++] = ch;
                    }
                    break;
                case L620_STATE_GET_READ_DATA:
                    if((len != 0) || (ch != '\r' && ch != '\n' && ch != '\0')) {
                        pdata[len++] = ch;
                        if(len >= reallen) {
                            state = L620_STATE_END;
                        }
                    }
                    break;
                default:
                    state = L620_STATE_END;
                    break;
            }
        } else {
            tick--;
            pat->delay_tick(1);
        }
    }
    if(tick <= 0) {
        reallen = RYAT_E_ERROR;
    }

    return reallen;
}

int32_t _l620_at_get_data_lwm2m(struct st_ryat_describe *pat,  uint8_t channel, uint8_t *pdata, uint16_t len, uint32_t timeout)
{
    int16_t tick = __ms_to_tick(timeout);
    char ch = 0;
    ryat_rx_line_t *pcurrent = &pat->cache.line[0];
    int16_t reallen = 0;
    uint8_t state = L620_STATE_GET_READ_PACKAGE_HEAD;
    uint8_t fmt[3] = {0};

    ryat_reset_cache(pat);
    /* +CTM2MRECV:494D4F4B */
    while(tick > 0 && state != L620_STATE_END) {
        if(pat->port_read_char(&ch)) {
            switch(state) {
                case L620_STATE_GET_READ_PACKAGE_HEAD:
                    if(ch == '\r' || ch == '\n' || ch == ':') {
                        if(pcurrent->len != 0) {
                            pcurrent->buf[pcurrent->len] = '\0';
                            __ryat_debug("AT-line:%s\n", pcurrent->buf);
                            if(ch == ':' && !strncmp(pcurrent->buf, "+CTM2MRECV", strlen("+CTM2MRECV"))) {
                                state = L620_STATE_GET_READ_DATA;
                            }
                        }
                        memset(pcurrent->buf, 0, sizeof(pcurrent->buf));
                        pcurrent->len = 0;
                    } else {
                        if(pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                            __ryat_debug("The line is to long\n");
                            memset(pcurrent->buf, 0, sizeof(pcurrent->buf));
                            pcurrent->len = 0;
                            continue;
                        }
                        pcurrent->buf[pcurrent->len++] = ch; 
                    }
                    break;
                case L620_STATE_GET_READ_DATA:
                    if(ch != '\r' && ch != '\n' && ch != '\0') {
                        uint8_t fmt_i = reallen % 2, data_i = reallen / 2;
                        if(reallen < (len * 2)) {
                            reallen++;
                            fmt[fmt_i] = ch;
                            if(fmt_i) {
                                ryat_utils_string_to_hex(fmt, &pdata[data_i], 1);
                            }
                        }
                    } else {
                        if(reallen) {
                            state = L620_STATE_END;
                        }
                    }
                    break;
                default:
                    state = L620_STATE_END;
                    break;
            }
        } else {
            tick--;
            pat->delay_tick(1);
        }
    }
    if(tick <= 0) {
        reallen = RYAT_E_ERROR;
    } else {
        reallen = reallen / 2;
    }

    return reallen;
}
