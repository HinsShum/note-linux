/**
 * /atlib/u9x07/u9x07_if.c
 *
 * Copyright (C) 2018 HinsShum
 *
 * u9x07_if.c is free software: you can redistribute it and/or modify
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
#include "u9x07_if.h"
#include "_u9x07_at.h"
#include "ryat.h"
#include "ryat_utils.h"
#include <ctype.h>
#include <stdlib.h>

/*---------- marco ----------*/
#if defined(__CC_ARM)
#define ISDIGIT_RET_VALUE   (NULL)
#else
#define ISDIGIT_RET_VALUE   (0)
#define strlcpy             strncpy
#endif

#define U9X07_MAX_BUF_LEN   (1500)

#define U9X07_USING_PASSTHROUGH

/*---------- variable prototype ----------*/
extern struct st_ryat_describe *gp_at;

/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static int32_t ryat_get_imei(uint8_t *pbuf) {
    ryat_cmd_t cmd = {
        __ms_to_tick(500),
        "AT+CGSN\r\n",
        RYAT_RET_OK | RYAT_RET_ERROR
    };

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // 862104020007479
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
        if(prx != NULL && prx->len == 15) {
            memcpy(pbuf, prx->buf, prx->len);
            return RYAT_E_OK;
        }
    }
    return RYAT_E_ERROR;
}

static int32_t ryat_get_iccid(uint8_t *pbuf)
{
    ryat_cmd_t cmd = {
        __ms_to_tick(2000),
        "AT+ICCID\r\n",
        RYAT_RET_OK | RYAT_RET_ERROR
    };

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // ^SCID: 89860111831001574065
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_CCID);
        if(prx != NULL) {
            char *p = prx->buf;
            for(uint16_t i = 0; (i < RYAT_COMMAND_LINE_LENGTH) && (ISDIGIT_RET_VALUE == isdigit(*++p)); ++i);
            if(p < (prx->buf + RYAT_COMMAND_LINE_LENGTH)) {
                memcpy(pbuf, p, strlen(p));
                return RYAT_E_OK;
            }
        }
    }
    return RYAT_E_ERROR;
}

static int32_t ryat_get_imsi(uint8_t *pbuf)
{
    ryat_cmd_t cmd = {
        __ms_to_tick(500),
        "AT+CIMI\r\n",
        RYAT_RET_OK | RYAT_RET_ERROR
    };

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // 460011512662442
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
        if(prx != NULL && prx->len == 15) {
            memcpy(pbuf, prx->buf, prx->len);
            return RYAT_E_OK;
        }
    }
    return RYAT_E_ERROR;
}

static int32_t ryat_get_creg_cell(struct st_ryat_cell_describe *pcell)
{
    ryat_cmd_t cmd = {
        __ms_to_tick(1000),
        "AT+CREG=2\r\n",
        RYAT_RET_OK | RYAT_RET_ERROR |RYAT_RET_CME
    };

    cmd.cmdlen = strlen(cmd.cmd);
    memset(pcell, 0, sizeof(*pcell));

    /* set the creg=2 first */
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_RET_ERROR;
    }

    /* get the creg status */
    cmd.cmd = "AT+CREG?\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(45000);

    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = NULL;
        ryat_utils_args_t args = { 0 };

        prx = ryat_get_line_with_type(gp_at, RYAT_RET_CEREG);
        if(prx != NULL) {
            // +CREG:2,1,9191,2E50
            char *p = NULL;
            p = strstr(prx->buf, ":");
            if(p == NULL) {
                return RYAT_E_ERROR;
            }
            ++p;
            if(RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
                return RYAT_E_ERROR;    
            }
            if((args.pnext == NULL) || (isdigit(args.pnext[0]) == ISDIGIT_RET_VALUE) || (args.pnext[0] != '1')) {
                return RYAT_E_ERROR;
            }
            p = strstr(args.pnext, ",");
            if(p == NULL) {
                return RYAT_E_ERROR;
            }
            ++p;
            if(RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args) || args.pnext == NULL) {
                return RYAT_E_ERROR;
            }
            pcell->lac = strtoul(args.pval, NULL, 16);
            p = args.pnext;
            if(RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
                return RYAT_E_ERROR;
            }
            pcell->cellid = strtoul(args.pval, NULL, 16);
        }
    }
    cmd.cmd = "AT+COPS=0,2\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(1000);
    cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR | RYAT_RET_CME;
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    cmd.cmd = "AT+COPS?\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = NULL;
        ryat_utils_args_t args = { 0 };

        prx = ryat_get_line_with_type(gp_at, RYAT_RET_COPS);
        if(prx != NULL) {
            char *p = prx->buf;
            for(uint8_t i = 0; i < 2; ++i) {
                p = strstr(++p, ",");
                if(p == NULL) {
                    return RYAT_E_ERROR;
                }
            }
            if(RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
                return RYAT_E_ERROR;
            }
            pcell->mcc = strtoul(args.pval, NULL, 10);
            pcell->mnc = pcell->mcc % 100;
            pcell->mcc = pcell->mcc / 100;
            return RYAT_E_OK;
        }
    }
    return RYAT_E_ERROR;
}

static int32_t ryat_gprs_deactivate(void)
{
    ryat_cmd_t cmd = {
        __ms_to_tick(20000),
        "AT+MIPCALL=0\r\n",
        RYAT_RET_OK | RYAT_RET_ERROR
    };

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

static int32_t ryat_gprs_active(uint8_t *ipbuf, uint16_t buflen)
{
    char *p = NULL;
    ryat_cmd_t cmd = {
        __ms_to_tick(30000),
        "AT+MIPCALL=1\r\n",
        RYAT_RET_MIPCALL | RYAT_RET_ERROR
    };

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_MIPCALL != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    gp_at->delay_tick(__ms_to_tick(1000));
    cmd.cmd = "AT+MIPCALL?\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
    cmd.tick = __ms_to_tick(3000);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    for(; gp_at->cache.rxhead < gp_at->cache.rxtail;) {
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_MIPCALL);
        if(prx == NULL) {
            return RYAT_E_ERROR;
        }
        p = strstr(prx->buf, ",");
        if(p != NULL) {
            ++p;
            strlcpy((char *)ipbuf, p, buflen);
            break;
        }
    }
    return (p != NULL) ? RYAT_E_OK : RYAT_E_ERROR;
}

static int32_t ryat_gprs_set_apn(char *apn, char *name, char *passwd)
{
    char cmdline[64] = { 0 };
    ryat_cmd_t cmd = {
        __ms_to_tick(1000),
        NULL,
        RYAT_RET_OK | RYAT_RET_ERROR
    };

    if(apn == NULL){
        return RYAT_RET_OK;
    }

    snprintf(cmdline, sizeof(cmdline), "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", apn);

    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    return RYAT_E_OK;
}

static int32_t ryat_socket_udp(char *domain_ip, uint16_t port, uint8_t ch)
{
    char cmdline[64] = { 0 };
    ryat_cmd_t cmd = {
        __ms_to_tick(30000),
        NULL,
        RYAT_RET_ERROR | RYAT_RET_MIP | RYAT_RET_MIPOPEN
    };

    snprintf(cmdline, sizeof(cmdline), "AT+MIPOPEN=%d,0,\"%s\",%d,1\r\n", ch, domain_ip, port);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_MIPOPEN != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    /* use recieve cache */
    cmd.cmd = "AT+MIPRS=0\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(1000);
    cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

#ifndef U9X07_USING_PASSTHROUGH
    cmd.cmd = "AT+MIPHEX=1\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(1000);
    cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
#endif
    return RYAT_E_OK;
}

static int32_t ryat_socket_tcp(char *domain_ip, uint16_t port, uint8_t ch)
{
    char cmdline[64] = { 0 };
    ryat_cmd_t cmd = {
        __ms_to_tick(30000),
        NULL,
        RYAT_RET_ERROR | RYAT_RET_CME | RYAT_RET_MIPOPEN
    };

    snprintf(cmdline, sizeof(cmdline), "AT+MIPOPEN=%d,0,\"%s\",%d,0\r\n", ch, domain_ip, port);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_MIPOPEN == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

static int32_t ryat_socket_shut(uint8_t ch)
{
    char cmdline[64] = { 0 };
    ryat_cmd_t cmd = {
        __ms_to_tick(1000),
        NULL,
        RYAT_RET_MIPCLOSE | RYAT_RET_ERROR
    };

    snprintf(cmdline, sizeof(cmdline), "AT+MIPCLOSE=%d\r\n", ch);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_MIPCLOSE == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

static int32_t ryat_write_data(uint8_t *pdata, uint16_t len, uint8_t ch)
{
#ifdef U9X07_USING_PASSTHROUGH
    char cmdbuf[32] = { 0 };
    ryat_cmd_t cmd = {
        __ms_to_tick(1000),
        "AT+MIPHEX=0\r\n",
        RYAT_RET_ERROR | RYAT_RET_OK
    };

    if(len >= (U9X07_MAX_BUF_LEN - 2)) {
        return RYAT_E_ERROR;
    }
    /* close hex mode */
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    /* send data */
    snprintf(cmdbuf, sizeof(cmdbuf), "AT+MIPTPS=3,%d,5000,%d\r\n", ch, len);
    cmd.cmd = cmdbuf;
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.ret = RYAT_RET_ERROR;
    if(RYAT_RET_ERROR == ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    cmd.cmd = (char *)pdata;
    cmd.ret = RYAT_RET_CME | RYAT_RET_ERROR | RYAT_RET_MIP;
    cmd.cmdlen = len;
    cmd.tick = __ms_to_tick(0);
    ryat_set(gp_at, &cmd);
    cmd.cmd = "\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(5000);

    if(RYAT_RET_MIP == ryat_set(gp_at, &cmd)) {
        return len;
    }
    return RYAT_E_ERROR;
#else
    char cmdbuf[U9X07_MAX_BUF_LEN + 20] = { 0 };
    ryat_cmd_t cmd = {
        __ms_to_tick(0),
        NULL,
        RYAT_RET_OK | RYAT_RET_ERROR
    };

    if(len >= U9X07_MAX_BUF_LEN) {
        return RYAT_E_ERROR;
    }
    /* send data */
    snprintf(cmdbuf, sizeof(cmdbuf), "AT+MIPSEND=%d,\"", ch);
    for(uint16_t i = 0; i < len; ++i) {
        char *p = cmdbuf + strlen(cmdbuf);
        uint8_t *c = pdata + i;
        sprintf(p, "%02X", *c);
    }
    strcat(cmdbuf, "\"\r\n");
    cmd.cmd = cmdbuf;
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(5000);
    cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    snprintf(cmdbuf, sizeof(cmdbuf), "AT+MIPPUSH=%d\r\n", ch);
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(1000);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    return RYAT_E_OK;
#endif
}

static int32_t ryat_read_cache(uint8_t ch, uint8_t *pdata, uint16_t len, uint16_t *remain, uint32_t timeout)
{
    ryat_rx_line_t *prx = NULL;
    char *p = NULL;
    ryat_cmd_t cmd = {
        __ms_to_tick(timeout),
        " ",
        RYAT_RET_MIPRUDP
    };
    
    /* wait +MIPRUDP */
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_MIPRUDP != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    /* get socket id */
    prx = ryat_get_line_with_type(gp_at, RYAT_RET_MIPRUDP);
    if(prx == NULL) {
        return RYAT_E_ERROR;
    }
    p = strstr(prx->buf, "=");
    if(p == NULL) {
        return RYAT_E_ERROR;
    }
    ch = atoi(p + 1);

#ifdef U9X07_USING_PASSTHROUGH
    cmd.cmd = "AT+MIPHEX=1\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(1000);
    cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
#endif
    return _u9x07_at_get_data(gp_at, ch, pdata, len, timeout);
}

/**
 * @brief get module model name
 * @param buf: buf to store module model name
 * @param buf_len: buf length
 *
 * @retval SL_EOK or SL_ERROR
 */
static int32_t ryat_get_model(char *buf, uint16_t buf_len)
{
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+GMM\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "9x07");
        ryat_utils_args_t args = {0};

        if(prx == NULL || RYAT_E_OK != ryat_utils_get_args(prx->buf, prx->len, &args)) {
            return RYAT_E_ERROR;
        }
        strlcpy(buf, args.pval, buf_len);
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

struct st_ryat_interface const ryat_interface_u9x07 = {
    .test = ryat_test,
    .close_echo = ryat_close_echo,
    .shutdown = ryat_shutdown,
    .get_csq = ryat_get_csq,
    .get_imei = ryat_get_imei,
    .get_iccid = ryat_get_iccid,
    .get_imsi = ryat_get_imsi,
    .attach = ryat_gprs_attach,
    .is_attached = ryat_gprs_is_attached,
    .set_apn = ryat_gprs_set_apn,
    .active = ryat_gprs_active,
    .deactivate = ryat_gprs_deactivate,
    .socket_udp = ryat_socket_udp,
    .socket_tcp = ryat_socket_tcp,
    .socket_shut = ryat_socket_shut,
    .write_data = ryat_write_data,
    .read_cache = ryat_read_cache,
    .get_creg_cell = ryat_get_creg_cell,
    .get_model = ryat_get_model,
};
