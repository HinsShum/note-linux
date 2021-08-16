/**
 * /atlib/bc26/bc26_if.c
 *
 * Copyright (C) 2019 HinsShum
 *
 * bc26_if.c is free software: you can redistribute it and/or modify
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
#include "bc26_if.h"
#include "_bc26_at.h"
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

/*---------- variable prototype ----------*/
extern struct st_ryat_describe *gp_at;

/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * ryat_get_imei() - get imei
 * @pbuf: buffer to store imei
 *
 * retval: int32_t
 */
static int32_t ryat_get_imei(uint8_t *pbuf) {
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+CGSN=1\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // 15 string +CGSN:865352030017056
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_CGSN);
        if (prx != NULL) {
            char *p = prx->buf;

            for (uint16_t i = 0; (i < RYAT_COMMAND_LINE_LENGTH) && (ISDIGIT_RET_VALUE == isdigit(*++p)); ++i);
            if (p < (prx->buf + RYAT_COMMAND_LINE_LENGTH)) {
                memcpy(pbuf, p, strlen(p));
                return RYAT_E_OK;
            }
        }
    }
    return RYAT_E_ERROR;
}

/**
 * ryat_get_iccid() - get iccid
 * @pbuf: buffer to store iccid
 *
 * retval: int32_t
 */
static int32_t ryat_get_iccid(uint8_t *pbuf) {
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+QCCID\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // 20 string +QCCID:89860317492043342789
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_CCID);
        if (prx != NULL) {
            char *p = prx->buf;

            for (uint16_t i = 0; (i < RYAT_COMMAND_LINE_LENGTH) && (ISDIGIT_RET_VALUE == isdigit(*++p)); ++i);
            if (p < (prx->buf + RYAT_COMMAND_LINE_LENGTH)) {
                memcpy(pbuf, p, strlen(p));
                return RYAT_E_OK;
            }
        }
    }
    return RYAT_E_ERROR;
}

/**
 * ryat_get_imsi() - get imsi
 * @pbuf: buffer to store imsi
 *
 * retval: int32_t
 */
static int32_t ryat_get_imsi(uint8_t *pbuf) {
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+CIMI\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // 15 string 460111172151911
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
        if (prx != NULL && prx->len == 15) {
          memcpy(pbuf, prx->buf, prx->len);
          return RYAT_E_OK;
        }
    }
    return RYAT_E_ERROR;
}

/**
 * ryat_gprs_set_apn() - set apn
 * @apn: apn name
 * @name: apn user name
 * @pass: apn user pass
 *
 * retval: RYAT_E_OK/RYAT_E_ERROR
 */
static int32_t ryat_gprs_set_apn(char *apn, char *name, char *passwd) {
    char cmdline[64] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_OK | RYAT_RET_ERROR};

    if (apn == NULL) {
        return RYAT_E_OK;
    }

    snprintf(cmdline, sizeof(cmdline), "AT+QGACT=1,1,\"%s\",\"%s\",\"%s\"\r\n", apn,
                      (name != NULL) ? name : "", (passwd != NULL) ? passwd : "");

    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    return RYAT_E_OK;
}

/**
 * ryat_gprs_active() - get module ip address
 * @pbuf: store ip address
 * @len: buf length
 *
 * retval: RYAT_E_OK/RYAT_E_ERROR
 */
static int32_t ryat_gprs_active(uint8_t *pbuf, uint16_t len)
{
    char *p = NULL;
    ryat_rx_line_t *prx = NULL;
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CGACT?\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    prx = ryat_get_line_with_prefix(gp_at, "+CGACT");
    if(prx == NULL) {
        return RYAT_E_ERROR;
    }
    p = strstr(prx->buf, ",");
    if(p == NULL) {
        return RYAT_E_ERROR;
    }
    if(*++p == '0') {
        cmd.cmd = "AT+CGACT=1,1\r\n";
        cmd.cmdlen = strlen(cmd.cmd);
        if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
            return RYAT_E_ERROR;
        }
    }

    gp_at->delay_tick(__ms_to_tick(1000));

    cmd.cmd = "AT+CGPADDR=1\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    prx = ryat_get_line_with_prefix(gp_at, "+CGPADDR");
    if(prx == NULL) {
        return RYAT_E_ERROR;
    }
    p = strstr(prx->buf, ",");
    if(p == NULL) {
        return RYAT_E_ERROR;
    }
    strncpy((char *)pbuf, ++p, len);

    return RYAT_E_OK;
}

/**
 * ryat_gprs_deactivate() - For this command, the term roaming
 * corresponds to being registered to a VPLMN which is not
 * equivalent to HPLMN or EHPLMN
 *
 * retval: RYAT_E_ERROR/RYAT_E_OK
 */
static int32_t ryat_gprs_deactivate(void)
{
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CGACT=0,1\r\n", RYAT_RET_OK | RYAT_RET_ERROR | RYAT_RET_CME};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        char *p = NULL;
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "+CGACT");

        p = strstr(prx->buf, ",");
        if(p != NULL && *++p == '0') {
            return RYAT_E_OK;
        }
    }

    return RYAT_E_ERROR;
}

/**
 * ryat_get_creg_cell() - get cell
 * @pcell:
 *
 * retval: int32_t
 */
static int32_t ryat_get_creg_cell(struct st_ryat_cell_describe *pcell)
{
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CEREG=2\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    memset(pcell, 0, sizeof(*pcell));

    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    /* get the creg status */
    cmd.cmd = "AT+CEREG?\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(45000);
    cmd.ret = RYAT_RET_OK | RYAT_RET_CME;

    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = NULL;
        ryat_utils_args_t args;

        prx = ryat_get_line_with_type(gp_at, RYAT_RET_CEREG);
        if(prx != NULL) {
            // +CEREG:2,1,XXX,YYY,Z
            char *p = NULL;

            p = strstr(prx->buf, ":");
            if(p == NULL) {
                return RYAT_E_ERROR;
            }
            ++p;
            if(RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
                return RYAT_E_ERROR;
            }
            /* get register state */
            if((args.pnext == NULL) || (ISDIGIT_RET_VALUE == isdigit(args.pnext[0]) || ('1' != args.pnext[0]))) {
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
    /* get mcc and mnc */
    cmd.cmd = "AT+COPS?\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(1000);
    cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = NULL;
        ryat_utils_args_t args;

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

/**
 * ryat_socket_init() - set data format
 * @ch: no use
 *
 * retval: RYAT_E_OK/RYAT_E_ERROR
 */
static int32_t ryat_socket_init(bool iscached)
{
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+QICFG=\"dataformat\",0,0\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);

    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    cmd.cmd = "AT+QICFG=\"viewmode\",0\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    return RYAT_E_OK;
}

/**
 * ryat_socket_shut() - close the connecting socket, connectID is 0
 * @ch: no use
 *
 * retval: RYAT_E_OK/RYAT_E_ERROR
 */
static int32_t ryat_socket_shut(uint8_t ch)
{
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+QICLOSE=0\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    return RYAT_E_OK;
}

/**
 * ryat_socket() - configure IP PORT
 * @domain_ip: ipdomain
 * @port: coap port, it must be 5683 when use Neul Hi2110
 * @ch: no use
 *
 * retval: RYAT_E_OK/RYAT_E_ERROR
 */
static int32_t ryat_socket(char *domain_ip, uint16_t port, uint8_t ch, char *service_type)
{
    char cmdline[64] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_OK | RYAT_RET_ERROR};

    snprintf(cmdline, sizeof(cmdline), "AT+QIOPEN=1,0,\"%s\",\"%s\",%d,0,0\r\n", service_type, domain_ip, port);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        /* If connection failed, the host must execute 'AT+QICLOSE=<connectID>'
         * to close the socket.
         */
        ryat_socket_shut(0);
        return RYAT_E_ERROR;
    }
    return RYAT_E_OK;
}

/**
 * ryat_socket_udp() - configure UDP
 * @domain_ip: ipdomain
 * @port: coap port, it must be 5683 when use Neul Hi2110
 * @ch: no use
 *
 * retval: RYAT_E_OK/RYAT_E_ERROR
 */
static int32_t ryat_socket_udp(char *domain_ip, uint16_t port, uint8_t ch)
{
    return ryat_socket(domain_ip, port, ch, "UDP");
}

/**
 * ryat_socket_tcp() - configure TCP
 * @domain_ip: ipdomain
 * @port: coap port, it must be 5683 when use Neul Hi2110
 * @ch: no use
 *
 * retval: RYAT_E_OK/RYAT_E_ERROR
 */
static int32_t ryat_socket_tcp(char *domain_ip, uint16_t port, uint8_t ch)
{
    return ryat_socket(domain_ip, port, ch, "TCP");
}

/**
 * ryat_write_data() - write data to module
 * @pdata: data
 * @len: data length
 * @ch: no use
 *
 * retval:
 */
static int32_t ryat_write_data(uint8_t *pdata, uint16_t len, uint8_t ch)
{
    char cmdline[20] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_SENDOK};

    snprintf(cmdline, sizeof(cmdline), "AT+QISEND=0,%d,", len);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);

    return _bc26_at_send_data(gp_at, &cmd, ch, pdata, len);
}

/**
 * ryat_read_cache() - get data from module
 * @ch: no use
 * @pdata: store data
 * @len: buf length
 * @remain: remain data length
 * @timeout: no use
 *
 * retval: RYAT_E_ERROR/data length
 */
static int32_t ryat_read_cache(uint8_t ch, uint8_t *pdata, uint16_t len, uint16_t *remain, uint32_t timeout)
{
    return _bc26_at_get_data(gp_at, ch, pdata, len, timeout);
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
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+CGMM\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "Quectel_BC26");
        ryat_utils_args_t args = {0};

        if(prx == NULL || RYAT_E_OK != ryat_utils_get_args(prx->buf, prx->len, &args)) {
            return RYAT_E_ERROR;
        }
        strlcpy(buf, args.pval, buf_len);
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

struct st_ryat_interface const ryat_interface_bc26 = {
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
    .socket_init = ryat_socket_init,
    .socket_udp = ryat_socket_udp,
    .socket_tcp = ryat_socket_tcp,
    .socket_shut = ryat_socket_shut,
    .write_data = ryat_write_data,
    .read_cache = ryat_read_cache,
    .get_creg_cell = ryat_get_creg_cell,
    .get_model = ryat_get_model,
};
