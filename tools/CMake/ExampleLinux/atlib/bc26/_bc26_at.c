/**
 * /atlib/bc26/_bc26_at.c
 *
 * Copyright (C) 2019 HinsShum
 *
 * _bc26_at.c is free software: you can redistribute it and/or modify
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
#include "_bc26_at.h"
#include "includes.h"
#include "ryat.h"
#include "ryat_utils.h"
#include "ryat_if.h"
#include <ctype.h>
#include <stdlib.h>

/*---------- marco ----------*/
#if defined(__CC_ARM)
#define ISDIGIT_RET_VALUE   (NULL)
#else
#define ISDIGIT_RET_VALUE   (0)
#endif

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
struct st_ryat_ret_describe const ryat_ret_tbl_bc26[] = {
    {RYAT_RET_OK, "OK", 2},
    {RYAT_RET_ERROR, "ERROR", 5},
    {RYAT_RET_CGSN, "+CGSN", 5},
    {RYAT_RET_CCID, "+QCCID", 6},
    {RYAT_RET_CSQ, "+CSQ", 4},
    {RYAT_RET_CEREG, "+CEREG", 6},
    {RYAT_RET_COPS, "+COPS", 5},
    {RYAT_RET_SENDOK, "SEND OK", 7},
    {RYAT_RET_QIRD, "+QIRD", 5},
    {RYAT_RET_AT_P, "AT+", 3},
    {RYAT_RET_P, "+", 1},

};

/*---------- function ----------*/
int32_t _bc26_at_send_data(struct st_ryat_describe *pat, ryat_cmd_t* pcmd, uint8_t channal, uint8_t *pdata, uint16_t len)
{
    ryat_rx_line_t *pcurrent = NULL;
    char ch = 0;

    __ryat_debug("AT-W %d:%s%s\r\n", pcmd->tick, pcmd->cmd, pdata);
     pat->port_discard_inpurt();
     pat->port_write_string(pcmd->cmd, pcmd->cmdlen);
     pat->port_write_string((char *)pdata, len);
     pat->port_write_string("\r\n", 2);

     /*
      * read the data from the modem and check the result
      */ 
     ryat_reset_cache(pat);
     pcurrent = &pat->cache.line[0];
     while(pcmd->tick > 0) {
         // read char and add to the buffer
         if(pat->port_read_char(&ch)) {
             if((ch == '\r') || (ch == '\n') || (ch == '\0')) {
                 // end of the line
                 if(pcurrent->len != 0x00) {
                     // add zero the end
                     pcurrent->buf[pcurrent->len] = 0x00;
                     __ryat_debug("AT-line:%s\r\n", pcurrent->buf);
                     //the line over begin a new line
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

int32_t _bc26_at_get_data(struct st_ryat_describe *pat, uint8_t channal, uint8_t *pdata, uint16_t len, uint32_t timeout)
{
    ryat_rx_line_t *pcurrent = NULL;
    int16_t tick = __ms_to_tick(timeout), cnt = 0;
    int reallen = 0;
    char ch = 0, *p = NULL;

    // reset the cache
    ryat_reset_cache(pat);

    pcurrent = &pat->cache.line[0];
    pcurrent->len = 0;
    strncpy(pcurrent->buf, "AT+QIRD=0,512\r\n", sizeof(pcurrent->buf));
    __ryat_debug("AT-W %d:%s", tick, pcurrent->buf);
    // write the command
    pat->port_discard_inpurt();
    pat->port_write_string(pcurrent->buf, strlen(pcurrent->buf));

    // read the data, get the responds line
    while(tick > 0) {
        if(pat->port_read_char(&ch)) {
            if((ch == '\r') || (ch == '\n') || (ch == '\0')) {
                // end of the line
                if(pcurrent->len != 0x00) {
                    // add zero to the end
                    pcurrent->buf[pcurrent->len] = 0x00;
                    __ryat_debug("AT-line:%s\r\n", pcurrent->buf);
                    // the line over begin a new line
                    ryat_update_type(pat, pcurrent);
                    __ryat_debug("AT-Type:%08X\r\n", pcurrent->type);
                    if(RYAT_RET_QIRD & pcurrent->type) {
                        break;
                    }
                    if(RYAT_RET_OK & pcurrent->type || RYAT_RET_ERROR & pcurrent->type) {
                        return RYAT_E_ERROR;
                    }
                    pcurrent->len = 0;
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
            tick--;
            pat->delay_tick(1);
        }
    }
    if(tick <= 0) {
        return RYAT_E_TIMEOUT;
    }
    p = strstr(pcurrent->buf, ":");
    if(p == NULL) {
        return RYAT_E_ERROR;
    }
    reallen = atoi(++p);
    if(reallen == 0) {
        return RYAT_E_BUSY;
    }
    if(reallen > len) {
        reallen = len;
    }
    // copy data
    tick = __ms_to_tick(100);
    while(tick > 0) {
        if(pat->port_read_char(&ch)) {
            if(cnt == 0 && ch == '\n') {
                tick--;
                continue;
            }
            pdata[cnt++] = ch;
            if(cnt >= reallen) {
                break;
            }
        } else {
            tick--;
            pat->delay_tick(1);
        }
    }
    // check last ok
    tick = __ms_to_tick(100);
    pcurrent = &pat->cache.line[0];
    pcurrent->len = 0;
    pcurrent->type = RYAT_RET_NULL;
    while(tick > 0) {
        if(pat->port_read_char(&ch)) {
            if(ch == '\r' || ch == '\n' || ch == '\0') {
                // end of the line
                if(pcurrent->len != 0x00) {
                    // add zero to the end
                    pcurrent->buf[pcurrent->len] = 0x00;
                    __ryat_debug("AT-line:%s\r\n", pcurrent->buf);
                    // line over begin a new line
                    ryat_update_type(pat, pcurrent);
                    __ryat_debug("AT-Type:%08X\r\n", pcurrent->type);
                    // get the needed answer, return right now
                    if(RYAT_RET_OK & pcurrent->type) {
                        break;
                    }
                    pcurrent->len = 0;
                }
            }  else {
                // fill the data to the line buf
                if(pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                    __ryat_debug("The line is to long\r\n");
                    continue;
                }
                pcurrent->buf[pcurrent->len++] = ch;
            }
        } else {
            tick--;
            pat->delay_tick(1);
        }
    }
    if(tick <= 0) {
        return RYAT_E_ERROR;
    }

    return reallen;
}
