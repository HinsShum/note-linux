/**
 * _nb05_01_at.c
 *
 * Copyright (C) 2018 HinsShum
 *
 * _nb05_01_at.c is free software: you can redistribute it and/or modify
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
 */

/*---------- includes ----------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ryat_if.h"
#include "_nbxx_01_at.h"

/*---------- marco ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
struct st_ryat_ret_describe const ryat_ret_tbl_nbxx_01[] = {
    { RYAT_RET_OK, "OK", 2 },
    { RYAT_RET_ERROR, "ERROR", 5 },
    { RYAT_RET_CSQ, "+CSQ", 4 },
    { RYAT_RET_CME, "+CME", 4},
    { RYAT_RET_CEREG, "+CEREG", 6},
    { RYAT_RET_COPS, "+COPS", 5},
    { RYAT_RET_NNMI, "+NNMI", 5},
    { RYAT_RET_P, "+", 1},
};

int32_t _nbxx_01_at_get_data(struct st_ryat_describe *pat, uint8_t *pdata, uint16_t len, uint16_t *remain, uint32_t timeout)
{
    ryat_rx_line_t *pcurrent = NULL;
    int16_t reallen = 0;
    uint16_t cnt = 0;
    char fmt[3] = { 0 };
    char ch = 0;
    uint32_t tick = __ms_to_tick(timeout);

    if(len == 0) {
        return RYAT_E_ERROR;
    }
    *remain = 0;
    // reset the cache
    ryat_reset_cache(pat);
    pat->port_discard_inpurt();

    pcurrent = &pat->cache.line[0];
    /* wait "+NNMI" */
    while(tick > 0) {
        if(pat->port_read_char(&ch)) {
            if(ch == '\r' || ch == '\n' || ch == '\0') {
                // end of the line
                if(pcurrent->len != 0) {
                    // add zero to the end
                    pcurrent->buf[pcurrent->len] = '\0';
                    // the line over begin a new line
                    ryat_update_type(pat, pcurrent);
                    pat->cache.rxtail++;
                    if(pcurrent->type == RYAT_RET_NNMI) {
                        break;
                    }
                    if(pat->cache.rxtail >= RYAT_LINE_MAX) {
                        // drop the last buffer in order to read the right answer
                        pat->cache.rxtail = RYAT_LINE_MAX;
                        pcurrent->type = RYAT_RET_NULL;
                        pcurrent->len = 0x00;
                        return RYAT_E_ERROR;
                    } else {
                        pcurrent++;
                    }
                }
            } else {
                // fill the data to the line buf
                if(pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                    continue;
                }
                pcurrent->buf[pcurrent->len++] = ch;
            }
        } else {
            tick--;
            pat->delay_tick(1);
        }
    }
    /* parse data */
    pcurrent = &pat->cache.line[0];
    pcurrent->len = 0;
    strncpy(pcurrent->buf, "AT+NMGR\r\n", sizeof(pcurrent->buf));
    pat->port_discard_inpurt();
    pat->port_write_string(pcurrent->buf, strlen(pcurrent->buf));
    while(tick > 0) {
        if(pat->port_read_char(&ch)) {
            if(ch == '\r' || ch == '\n' || ch == '\0') {
                /* end of data */
                if(pcurrent->len != 0) {
                    break;
                }
            } else {
                if(reallen == 0) {
                    if(ch == ',') {
                        pcurrent->buf[pcurrent->len] = '\0';
                        reallen = atoi(pcurrent->buf);
                        *remain = (reallen < len) ? 0 : (reallen - len);
                        reallen = (reallen < len) ? reallen : len;
                    } else {
                        pcurrent->buf[pcurrent->len++] = ch;
                    }
                } else {
                    /* format data */
                    uint32_t tmp = 0;
                    uint8_t fmt_index = cnt % 2;
                    uint16_t data_index = cnt / 2;

                    if(data_index < reallen) {
                        fmt[fmt_index] = ch;
                        if(fmt_index) {
                            sscanf(fmt, "%02x", &tmp);
                            pdata[data_index] = (uint8_t)tmp;
                        }
                        cnt++;
                    }
                }
            }
        } else {
            tick--;
            pat->delay_tick(1);
        }
    }

    return (tick > 0) ? reallen : RYAT_E_ERROR;
}
/*---------- function ----------*/
