/**
 * @file atlib/ec20/ec20_if.c
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
#include "ec20_if.h"
#include "_ec20_at.h"
#include "ryat.h"
#include "ryat_utils.h"
#include <ctype.h>
#include <stdlib.h>

/*---------- macro ----------*/
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
 * @brief get imei from ec20
 * @param pbuf: store imei
 *
 * @retval RYAT_E_OK: get imei success;
 *         RYAT_E_ERROR: get imei failed.
 */
static int32_t ryat_get_imei(uint8_t *pbuf)
{
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+CGSN\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // AT+CGSN
        // 863879041484851
        // OK
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
        if(prx != NULL && prx->len == 15) {
            memcpy(pbuf, prx->buf, prx->len);
            return RYAT_E_OK;
        }
    }
    return RYAT_E_ERROR;
}

/**
 * @brief get iccid from ec20
 * @param pbuf: store iccid buf
 *
 * @retval RYAT_E_OK: get iccid success,
 *         RYAT_E_ERROR: get iccid failed.
 */
static int32_t ryat_get_iccid(uint8_t *pbuf)
{
    ryat_cmd_t cmd = {__ms_to_tick(2000), "AT+QCCID\r\n", RYAT_RET_OK | RYAT_RET_CME};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // AT+QCCID
        // +QCCID: 89860439101880931012
        // OK
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_CCID);
        if(prx != NULL) {
            char *p = prx->buf;
            for(uint8_t i = 0; (i < RYAT_COMMAND_LINE_LENGTH) && (ISDIGIT_RET_VALUE == isdigit(*++p)); ++i) {
            }
            if(p < (prx->buf + RYAT_COMMAND_LINE_LENGTH)) {
                memcpy(pbuf, p, strlen(p));
                return RYAT_E_OK;
            }
        }
    }
    return RYAT_E_ERROR;
}

/**
 * @brief get imsi from ec20
 * @param pbuf: store imsi buf
 *
 * @retval RYAT_E_OK: get imsi success,
 *         RYAT_E_ERROR: get imsi failed.
 */
static int32_t ryat_get_imsi(uint8_t *pbuf)
{
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+CIMI\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // AT+CIMI
        // 460045929801012
        // OK
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
        if(prx != NULL && prx->len == 15) {
            memcpy(pbuf, prx->buf, prx->len);
            return RYAT_E_OK;
        }
    }
    return RYAT_E_ERROR;
}

/**
 * @brief set apn
 * @param apn: apn name
 * @param name: no use
 * @param passwd: no use
 *
 * @retval RYAT_E_OK: set apn success,
 *         RYAT_E_RROR: set apn failed.
 */
static int32_t ryat_gprs_set_apn(char *apn, char *name, char *passwd)
{
    char cmdline[64] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_OK | RYAT_RET_ERROR};

    if(apn == NULL) {
        return RYAT_E_OK;
    }

    snprintf(cmdline, sizeof(cmdline), "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", apn);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

/**
 * @brief get the slm750 ip address
 * @param ipbuf: store ip
 * @param buflen: buf length
 *
 * @retval RYAT_E_OK: get ip address success,
 *         RYAT_E_ERROR: get ip address success.
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
        cmd.tick = __ms_to_tick(5000);
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
 * @brief gprs deactivate
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
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
 * ryat_socket_init() - set data format
 * @ch: no use
 *
 * retval: RYAT_E_OK/RYAT_E_ERROR
 */
static int32_t ryat_socket_init(bool ishex)
{
    char cmdline[32] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_OK | RYAT_RET_ERROR};

    snprintf(cmdline, sizeof(cmdline), "AT+QICFG=\"dataformat\",%d,%d\r\n", ishex, ishex);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    cmd.cmd = "AT+QICFG=\"viewmode\",1\r\n";
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
    char cmdline[32] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_OK | RYAT_RET_ERROR};

    snprintf(cmdline, sizeof(cmdline), "AT+QICLOSE=%d\r\n", ch);
    cmd.cmd = cmdline;
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
    ryat_cmd_t cmd = {__ms_to_tick(2000), NULL, RYAT_RET_MIPOPEN | RYAT_RET_ERROR};

    snprintf(cmdline, sizeof(cmdline), "AT+QIOPEN=1,%d,\"%s\",\"%s\",%d,0,1\r\n", ch, service_type, domain_ip, port);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_MIPOPEN != ryat_set(gp_at, &cmd)) {
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

    snprintf(cmdline, sizeof(cmdline), "AT+QISEND=%d,%d\r\n", ch, len);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);

    return  _ec20_at_send_data(gp_at, &cmd, ch, pdata, len);
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
    return _ec20_at_get_data(gp_at, ch, pdata, len, timeout);
}

/**
 * @brief get network mode
 * @param mode: store mode value
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_gprs_get_radio_access(uint8_t *mode)
{
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+QCFG=\"nwscanmode\"\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // AT+QCFG="nwscanmode"
        // +QCFG: "nwscanmode",0
        // OK
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "+QCFG");
        ryat_utils_args_t args = {0};

        if(prx != NULL) {
            if(RYAT_E_OK == ryat_utils_get_args(prx->buf, prx->len, &args) &&
               RYAT_E_OK == ryat_utils_get_args(args.pnext, strlen(args.pnext), &args) &&
               RYAT_E_OK == ryat_utils_get_args(args.pnext, strlen(args.pnext), &args) &&
               ISDIGIT_RET_VALUE != isdigit(args.pval[0])) {
                *mode = (atoi(args.pval) == 1) ? RYAT_RADIO_ACCESS_MODE_2G : RYAT_RADIO_ACCESS_MODE_4G;
                return RYAT_E_OK;
            }
        }
    }

    return RYAT_E_ERROR;
}

/**
 * @brief change radio access mode
 * @param mode: RYAT_RADIO_ACCESS_MODE_2G
 *              RYAT_RADIO_ACCESS_MODE_4G
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_gprs_select_radio_access(uint8_t mode)
{
    uint8_t set_mode = 0;
    char cmdline[32] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_OK | RYAT_RET_ERROR};

    switch(mode) {
        case RYAT_RADIO_ACCESS_MODE_2G:
            set_mode = 1;
            break;
        case RYAT_RADIO_ACCESS_MODE_4G:
            set_mode = 3;
            break;
        default: return RYAT_E_WRONG_ARGS;
    }
    /* set_mode: 1 --> 2G, 3 --> 4G */
    snprintf(cmdline, sizeof(cmdline), "AT+QCFG=\"nwscanmode\",%d\r\n", set_mode);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    return RYAT_E_OK;
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
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "EC20");
        ryat_utils_args_t args = {0};

        if(prx == NULL || RYAT_E_OK != ryat_utils_get_args(prx->buf, prx->len, &args)) {
            return RYAT_E_ERROR;
        }
        strlcpy(buf, args.pval, buf_len);
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

struct st_ryat_interface const ryat_interface_ec20 = {
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
    .socket_tcp = ryat_socket_tcp,
    .socket_udp = ryat_socket_udp,
    .socket_shut = ryat_socket_shut,
    .write_data = ryat_write_data,
    .read_cache = ryat_read_cache,
    .get_creg_cell = ryat_get_cellinfo,
    .select_radio_access = ryat_gprs_select_radio_access,
    .get_radio_access = ryat_gprs_get_radio_access,
    .get_model = ryat_get_model,
};