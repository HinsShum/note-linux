/**
 * @file atlib/l620/l620_if.c
 *
 * Copyright (C) 2020
 *
 * l620_if.c is free software: you can redistribute it and/or modify
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
#include "l620_if.h"
#include "_l620_at.h"
#include "ryat.h"
#include "ryat_utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*---------- macro ----------*/
#if defined(__CC_ARM)
#define ISDIGIT_RET_VALUE   (NULL)
#else
#define ISDIGIT_RET_VALUE   (0)
#define strlcpy             strncpy
#endif

#define _USING_LWM2M_TO_AEP
#define _MAX_DATA_SIZE      (512)
#define _BUF_SIZE           (_MAX_DATA_SIZE + 20)

/*---------- variable prototype ----------*/
extern struct st_ryat_describe *gp_at;

/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static uint8_t __buf[_BUF_SIZE];

/*---------- function ----------*/
/**
 * @brief get imei from l620
 * @param pbuf: store imei
 *
 * @retval RYAT_E_OK: get imei success;
 *         RYAT_E_ERROR: get imei failed.
 */
static int32_t ryat_get_imei(uint8_t *pbuf)
{
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+CGSN=1\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // AT+CGSN=1
        // +CGSN: 869858032952443
        // OK
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_CGSN);
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
 * @brief get iccid from l620
 * @param pbuf: store iccid buf
 *
 * @retval RYAT_E_OK: get iccid success,
 *         RYAT_E_ERROR: get iccid failed.
 */
static int32_t ryat_get_iccid(uint8_t *pbuf)
{
    ryat_cmd_t cmd = {__ms_to_tick(2000), "AT*MICCID\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // AT*MICCID
        // *MICCID: 89861119259016919150
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
 * @brief get imsi from l620
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
 * @brief get rxlevel from l620
 * @param rxlev: store value
 *
 * @retval RYAT_E_OK: get rxlevel success,
 *         RYAT_E_ERROR: get rxlevel failed.
 */
static int32_t ryat_get_rxlev(uint8_t *rxlev)
{
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+CESQ\r\n", RYAT_RET_OK | RYAT_RET_ERROR | RYAT_RET_CME};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "+CESQ");
        ryat_utils_args_t args = {0};
        /* AT+CESQ
        * +CESQ: 32,0,255,255,4,46
        * OK
        */
        if(prx != NULL) {
            if(RYAT_E_OK == ryat_utils_get_args(prx->buf, prx->len, &args) && args.pnext != NULL &&
               RYAT_E_OK == ryat_utils_get_args(args.pnext, strlen(args.pnext), &args) &&
               ISDIGIT_RET_VALUE != isdigit(args.pval[0])) {
                *rxlev = strtoul(args.pval, NULL, 10);
                return RYAT_E_OK;
            }
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
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CGDCONT?\r\n", RYAT_RET_OK | RYAT_RET_ERROR};
    ryat_rx_line_t *prx = NULL;

    if(apn == NULL) {
        return RYAT_E_OK;
    }
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    prx = ryat_get_line_with_prefix(gp_at, "+CGDCONT");
    if(prx == NULL) {
        snprintf(cmdline, sizeof(cmdline), "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", apn);
        cmd.cmd = cmdline;
        cmd.cmdlen = strlen(cmd.cmd);
        if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
            return RYAT_E_ERROR;
        }
    }
    return RYAT_E_OK;;
}

/**
 * @brief get the L620 ip address
 * @param ipbuf: store ip
 * @param buflen: buf length
 *
 * @retval RYAT_E_OK: get ip address success,
 *         RYAT_E_ERROR: get ip address success.
 */
static int32_t ryat_gprs_active(uint8_t *ipbuf, uint16_t buflen)
{
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+CIICR\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    cmd.cmd = "AT+CIFSR\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
        /* AT+CIFSR
        * 11.79.253.149
        * OK
        */
        if(prx != NULL) {
            strlcpy((char *)ipbuf, prx->buf, buflen);
            return RYAT_E_OK;
        }
    }
    return RYAT_E_ERROR;
}

/**
 * @brief gprs deactivate
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_gprs_deactivate(void)
{
    ryat_cmd_t cmd = {__ms_to_tick(2000), "AT+EGACT=0,1\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

#ifndef _USING_LWM2M_TO_AEP
/**
 * @brief select tcp/ip appliaction mode----Normal mode
 *        select single IP connection
 *        disable getting data from network manually
 * @param nouse: no use 
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_socket_init(bool nouse)
{
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CIPMODE=0\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    cmd.cmd = "AT+CIPMUX=0\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    cmd.cmd = "AT+CIPRXGET=0\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    return RYAT_E_OK;
}

/**
 * @brief connect the tcp
 * @param domain_ip: remote ip
 * @param port: remote port
 * @param ch: nouse
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_socket_tcp(char *domain_ip, uint16_t port, uint8_t ch)
{
    char cmdline[64] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(5000), NULL, RYAT_RET_CON_OK | RYAT_RET_CON_FAIL | RYAT_RET_ERROR};

    snprintf(cmdline, sizeof(cmdline), "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", domain_ip, port);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_CON_OK == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

/**
 * @brief close socket
 * @param ch: no use
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_socket_shut(uint8_t ch)
{
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CIPCLOSE\r\n", RYAT_RET_CLOSE_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_CLOSE_OK == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

/**
 * @brief write data to l620
 * @param string: string
 * @param len: string length
 * @param ch: no use
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_write_string(uint8_t *string, uint16_t len, uint8_t ch)
{
    char cmdbuf[32] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_UNKNOWN | RYAT_RET_ERROR};
    
    snprintf(cmdbuf, sizeof(cmdbuf), "AT+CIPSEND=%d\r\n", strlen((char *)string));
    cmd.cmd = cmdbuf;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_UNKNOWN == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, ">");
        
        if(prx == NULL) {
            return RYAT_E_ERROR;
        }
    }
    cmd.cmd = (char *)string;
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(2000);
    cmd.ret = RYAT_RET_ERROR | RYAT_RET_SENDOK;
    if(RYAT_RET_SEND_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    return RYAT_E_OK;
}

static int32_t ryat_read_cache(uint8_t ch, uint8_t *pdata, uint16_t len, uint16_t *remain, uint32_t timeout)
{
    return _l620_at_get_data(gp_at, ch, pdata, len, timeout);
}
#else
/**
 * @brief do nothing
 * @param nouse: no use 
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_socket_init_lwm2m(bool nouse)
{
    return RYAT_E_OK;
}

/**
 * @brief initialize lwm2m parameters, register to the ct-wing
 * @param domain_ip: remote ip
 * @param port: remote port
 * @param ch: nouse
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_socket_udp_lwm2m(char *domain_ip, uint16_t port, uint8_t ch)
{
    char cmdline[64] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(5000), NULL, RYAT_RET_CTM2M_ERROR | RYAT_RET_OK};
    ryat_rx_line_t *prx = NULL;
    char *p = NULL;
    int32_t status = -1;

    snprintf(cmdline, sizeof(cmdline), "AT+CTM2MINIT=%s,%d,86400,0,0\r\n", 
                                       domain_ip, port);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    snprintf(cmdline, sizeof(cmdline), "AT+CTM2MREG\r\n");
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.ret = RYAT_RET_CTM2M_REG | RYAT_RET_CTM2M_ERROR;
    if(RYAT_RET_CTM2M_REG != ryat_set(gp_at, &cmd)) {
        return  RYAT_E_ERROR;
    }
    prx = ryat_get_line_with_type(gp_at, RYAT_RET_CTM2M_REG);
    if(!prx) {
        return RYAT_E_ERROR;
    }
    p = strrchr(prx->buf, ',');
    if(!p) {
        return RYAT_E_ERROR;
    }
    status = strtoul(++p, NULL, 10);

    return status == 0 ? RYAT_E_OK : RYAT_E_ERROR;
}

/**
 * @brief deregister from ct-wing
 * @param ch: no use
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_socket_shut_lwm2m(uint8_t ch)
{
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CTM2MDEREG\r\n", RYAT_RET_CTM2M_ERROR | RYAT_RET_OK};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

/**
 * @brief write data to l620
 * @param string: string
 * @param len: string length
 * @param ch: no use
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_write_string(uint8_t *string, uint16_t len, uint8_t ch)
{
    ryat_cmd_t cmd = {__ms_to_tick(2000), NULL, RYAT_RET_OK | RYAT_RET_CTM2M_ERROR};
    uint16_t cnt = 0;

    memset(__buf, 0, sizeof(__buf));
    cnt = sprintf((char *)__buf, "AT+CTM2MSEND=");
    if(0 >= ryat_utils_hex_to_string(string, __buf + cnt, _MAX_DATA_SIZE)) {
        return RYAT_E_ERROR;
    }
    strcat((char *)__buf, "\r\n");
    cmd.cmd = (char *)__buf;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    return RYAT_E_OK;
}

static int32_t ryat_read_cache(uint8_t ch, uint8_t *pdata, uint16_t len, uint16_t *remain, uint32_t timeout)
{
    return _l620_at_get_data_lwm2m(gp_at, ch, pdata, len, timeout);
}
#endif

/**
 * @brief get module model name
 * @param buf: buf to store module model name
 * @param buf_len: buf length
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_get_model(char *buf, uint16_t buf_len)
{
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+CGMM\r\n", RYAT_RET_OK};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
        ryat_utils_args_t args = {0};

        if(prx == NULL || RYAT_E_OK != ryat_utils_get_args(prx->buf, prx->len, &args)) {
            return RYAT_E_ERROR;
        }
        strlcpy(buf, args.pval, buf_len);
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

struct st_ryat_interface const ryat_interface_l620 = {
    .test = ryat_test,
    .close_echo = ryat_close_echo,
    .shutdown = ryat_shutdown,
    .get_csq = ryat_get_csq,
    .get_rxlev = ryat_get_rxlev,
    .get_imei = ryat_get_imei,
    .get_iccid = ryat_get_iccid,
    .get_imsi = ryat_get_imsi,
    .attach = ryat_gprs_attach,
    .is_attached = ryat_gprs_is_attached,
    .set_apn = ryat_gprs_set_apn,
    .active = ryat_gprs_active,
    .deactivate = ryat_gprs_deactivate,
#ifndef _USING_LWM2M_TO_AEP
    .socket_init = ryat_socket_init,
    .socket_tcp = ryat_socket_tcp,
    .socket_shut = ryat_socket_shut,
#else
    .socket_init = ryat_socket_init_lwm2m,
    .socket_udp = ryat_socket_udp_lwm2m,
    .socket_shut = ryat_socket_shut_lwm2m,
#endif
    .write_data = ryat_write_string,
    .read_cache = ryat_read_cache,
    .get_creg_cell = ryat_get_cellinfo,
    .get_model = ryat_get_model,
};
