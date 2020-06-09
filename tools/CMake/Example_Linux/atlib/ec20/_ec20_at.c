/**
 * @file atlib/ec20/_ec20_at.c
 *
 * Copyright (C) 2020
 *
 * _slm750_at.h is free software: you can redistribute it and/or modify
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
#include "_ec20_at.h"
#include "includes.h"
#include "ryat_if.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/*---------- macro ----------*/
#ifndef RYAT_DEBUG_ENABLE
#define RYAT_DEBUG_ENABLE (0)
#endif

#define MAX_SEND_DATA_LENGTH        (1460)

#define EC20_STATE_GET_READ_PACKAGE_HEAD     (0)
#define EC20_STATE_GET_SOCKET_STATE          (1)
#define EC20_STATE_GET_SOCKET_ID             (2)
#define EC20_STATE_GET_DATA_LENGTH           (3)
#define EC20_STATE_GET_DATA                  (4)
#define EC20_STATE_END                       (5)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
struct st_ryat_ret_describe const ryat_ret_tbl_ec20[] = {
    { RYAT_RET_OK, "OK", 2 },
    { RYAT_RET_ERROR, "ERROR", 5 },
    { RYAT_RET_CME, "+CME", 4 },
    { RYAT_RET_CSQ, "+CSQ", 4 },
    { RYAT_RET_COPS, "+COPS", 5 },
    { RYAT_RET_CCID, "+QCCID", 6 },
    { RYAT_RET_SENDOK, "SEND OK", 7 },
    { RYAT_RET_MIPOPEN, "+QIOPEN", 7 },
    { RYAT_RET_AT_P, "AT+", 3 },
    { RYAT_RET_P, "+", 1 },
    { RYAT_RET_NULL, NULL, 0}
};

int32_t _ec20_at_send_data(struct st_ryat_describe *pat, ryat_cmd_t *pcmd, uint8_t channel, uint8_t *pdata, uint16_t len)
{
    char ch = 0;
    ryat_rx_line_t *pcurrent = NULL;
    char buf[3] = {0};

    if(len > MAX_SEND_DATA_LENGTH) {
        __ryat_debug("AT-E send data length can not more than %d\r\n", MAX_SEND_DATA_LENGTH);
        return RYAT_E_WRONG_ARGS;
    }
    __ryat_debug("AT-W %d:%s%s\r\n", pcmd->tick, pcmd->cmd, pdata);
    pat->port_discard_inpurt();
    pat->port_write_string(pcmd->cmd, pcmd->cmdlen);
    pat->delay_tick(__ms_to_tick(100));
    for(uint8_t i = 0; i < len; ++i) {
        snprintf(buf, sizeof(buf), "%02X", pdata[i]);
        pat->port_write_string(buf, 2);
    }
    /*
     * read the data from the modem and check the result
     */ 
    ryat_reset_cache(pat);
    pcurrent = &pat->cache.line[0];
    while (pcmd->tick > 0) {
        // read char and add to the buffer
        if(pat->port_read_char(&ch)) {
            if((ch == '\r') || (ch == '\n') || (ch == '\0')) {
                // end of the line
                if (pcurrent->len != 0x00) {
                    // add zero to the end
                    pcurrent->buf[pcurrent->len] = 0x00;
                    __ryat_debug("AT-line:%s\r\n", pcurrent->buf);
                    // the line over begin a new line
                    ryat_update_type(pat, pcurrent);
                    __ryat_debug("AT-Type:%08X\r\n", pcurrent->type);
                    pat->cache.rxtail++;
                    // get the needed answer, return right now
                    if(pcmd->ret & pcurrent->type) {
                        return (ryat_res_t)pcurrent->type;
                    }
                    // else cache the middle data
                    if(pat->cache.rxtail >= RYAT_LINE_MAX) {
                        __ryat_debug("Too many lines to read\r\n");
                        //drop the last buffer in order to read the right answer
                        pat->cache.rxtail = RYAT_LINE_MAX;
                        pcurrent->type = RYAT_RET_NULL;
                        pcurrent->len = 0x00;
                    } else {
                        pcurrent++;
                    }
                }
            } else {
                // fill the data to the line buf
                if(pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                    __ryat_debug("The line is too long\r\n");
                    continue;
                }
                pcurrent->buf[pcurrent->len++] = ch;
            }
        } else {
            pcmd->tick--;
            pat->delay_tick(1);
        }
    }
    return RYAT_RET_NULL;
}

int32_t _ec20_at_get_data(struct st_ryat_describe *pat, uint8_t channel, uint8_t *pdata, uint16_t len, uint32_t timeout)
{
    int16_t tick = __ms_to_tick(timeout);
    char ch = 0;
    ryat_rx_line_t *pcurrent = &pat->cache.line[0];
    uint8_t state = STATE_GET_READ_PACKAGE_HEAD;
    char fmt[3] = {0};
    int16_t reallen = 0;
    ryat_utils_args_t args = {0};

    ryat_reset_cache(pat);
    //+QIURC: "recv",0,4,494D4F4B
    while(tick > 0 && state != STATE_END) {
        if(pat->port_read_char(&ch)) {
            switch(state) {
                case EC20_STATE_GET_READ_PACKAGE_HEAD: {
                    if(ch == '+' || pcurrent->len != 0) {
                        pcurrent->buf[pcurrent->len++] = ch;
                    }
                    if(pcurrent->len == strlen("+QIURC:")) {
                        if(!strncmp(pcurrent->buf, "+QIURC:", strlen("+QIURC:"))) {
                            state = EC20_STATE_GET_SOCKET_STATE;
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
                case EC20_STATE_GET_SOCKET_STATE: {
                    pcurrent->buf[pcurrent->len++] = ch;
                    if(ch == ',') {
                        pcurrent->buf[pcurrent->len - 1] = '\0';
                        if(RYAT_E_OK != ryat_utils_get_args(pcurrent->buf, pcurrent->len, &args)) {
                            return RYAT_E_ERROR;
                        }
                        __ryat_debug("Socket State: %s\r\n", args.pval);
                        state = EC20_STATE_GET_SOCKET_ID;
                        pcurrent++;
                    } else if(ch == '\r' || ch == '\n' || ch == '\0' ||
                              pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        __ryat_debug("__ec20_at_get_data() get socket state failed\r\n");
                        return RYAT_E_ERROR;
                    }
                    break;
                }
                case EC20_STATE_GET_SOCKET_ID: {
                    pcurrent->buf[pcurrent->len++] = ch;
                    if(ch == ',') {
                        pcurrent->buf[pcurrent->len - 1] = '\0';
                        __ryat_debug("Socket id: %s\r\n", pcurrent->buf);
                        state = EC20_STATE_GET_DATA_LENGTH;
                        pcurrent++;
                    } else if(ch == '\r' || ch == '\n' || ch == '\0' ||
                              pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        __ryat_debug("__ec20_at_get_data() get socket id failed\r\n");
                        return RYAT_E_ERROR;
                    }
                    break;
                }
                case EC20_STATE_GET_DATA_LENGTH: {
                    pcurrent->buf[pcurrent->len++] = ch;
                    if(ch == ',') {
                        pcurrent->buf[pcurrent->len - 1] = '\0';
                        __ryat_debug("Data length: %s\r\n", pcurrent->buf);
                        state = EC20_STATE_GET_DATA;
                    } else if(ch == '\r' || ch == '\n' || ch == '\0' ||
                              pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        __ryat_debug("__ec20_at_get_data() get data length failed\r\n");
                        return RYAT_E_ERROR;
                    }
                    break;
                }
                case EC20_STATE_GET_DATA: {
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
                        state = EC20_STATE_END;
                    }
                    break;
                }
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