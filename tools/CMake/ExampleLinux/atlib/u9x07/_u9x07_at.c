/**
 * /atlib/u9x07/_u9x07_at.c
 *
 * Copyright (C) 2018 HinsShum
 *
 * _u9x07_at.c is free software: you can redistribute it and/or modify
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
#include "_u9x07_at.h"
#include "ryat.h"
#include "ryat_utils.h"
#include "ryat_if.h"

/*---------- marco ----------*/
#ifndef RYAT_DEBUG_ENABLE
#define RYAT_DEBUG_ENABLE (0)
#endif

#ifndef __ryat_debug
#if (RYAT_DEBUG_ENABLE > 0)
#define __ryat_debug(x, ...) debug_info(x, ##__VA_ARGS__)
#else
#define __ryat_debug(x, ...)
#endif /* RYAT_DEBUG_ENABLE */
#endif /* __ryat_debug */

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
struct st_ryat_ret_describe const ryat_ret_tbl_u9x07[] = {
    {RYAT_RET_OK, "OK", 2},
    {RYAT_RET_ERROR, "ERROR", 5},
    {RYAT_RET_CSQ, "+CSQ", 4},
    {RYAT_RET_CEREG, "+CREG", 5},
    {RYAT_RET_COPS, "+COPS", 5},
    {RYAT_RET_CCID, "^SCID", 5},
	{RYAT_RET_MIPRUDP,"+MIPRUDP", 8},
    {RYAT_RET_MIPCALL, "+MIPCALL", 8},
    {RYAT_RET_MIPOPEN, "+MIPOPEN", 8},
    {RYAT_RET_MIPCLOSE, "+MIPCLOSE", 9},
    {RYAT_RET_MIP, "+MIP", 4},
    {RYAT_RET_P, "+", 1}
};

int32_t _u9x07_at_get_data(struct st_ryat_describe *pat, uint8_t channel, uint8_t *pdata, uint16_t max_len, uint32_t timeout)
{
    int16_t tick = __ms_to_tick(timeout);
    int16_t reallen = 0;
    ryat_utils_args_t args = { 0 };
    char ch = 0;
    char fmt[3] = { 0 };
    char *p = NULL;
    char *remote_ip = NULL, *remote_port = NULL;
    ryat_rx_line_t* pcurrent = NULL;

    if(max_len == 0) {
        return RYAT_E_ERROR;
    }
    // reset the cache
    ryat_reset_cache(pat);
    pcurrent = &pat->cache.line[0];
    snprintf(pcurrent->buf, sizeof(pcurrent->buf), "AT+MIPREAD=%d\r\n", channel);
    __ryat_debug("AT-W %d:%s", tick, pcurrent->buf);
    // write command
    pat->port_discard_inpurt();
    pat->port_write_string(pcurrent->buf, strlen(pcurrent->buf));

    /* wait response */
    while(tick > 0) {
        if(pat->port_read_char(&ch)) {
            if(ch == '\r' || ch == '\n' || ch == '\0') {
                if(reallen == 0) {
                    continue;
                }
                if(strstr((char *)pdata, "+MIPRUDP") == NULL) {
                    tick = 0;
                }
                pdata[reallen++] = '\0';
                break;
            } else {
                if((ch == 'O' || ch == 'K') && (reallen == 0)) {
                    continue;
                }
                pdata[reallen++] = ch;
                if(reallen >= max_len) {
                    return RYAT_E_ERROR;
                }
            }
        } else {
            tick--;
            pat->delay_tick(1);
        }
    }
    if(tick == 0) {
        return RYAT_E_TIMEOUT;
    }
    p = strstr((char *)pdata, ",");
    if(p == NULL) {
        return RYAT_E_ERROR;
    }
    if(RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
        return RYAT_E_ERROR;
    }
    remote_ip = args.pval;
    p = args.pnext;
    if(RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
        return RYAT_E_ERROR;
    }
    remote_port = args.pval;
    __ryat_debug("Remote ip:%s, port:%s\r\n", remote_ip, remote_port);
    p = args.pnext;
    if(RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
        return RYAT_E_ERROR;
    }
    reallen = atoi(args.pval);
    p = args.pnext;
    for(uint16_t i = 0; i < reallen; ++i) {
        /* format data */
        uint32_t tmp = 0;
        uint8_t fmt_index = i % 2;
        uint8_t data_index = i / 2;

        fmt[fmt_index] = p[i];
        if(fmt_index) {
            sscanf(fmt, "%02X", &tmp);
            pdata[data_index] = (uint8_t)tmp;
        }
    }
    reallen /= 2;
    pdata[reallen] = 0x00;
    
    return reallen;
}

