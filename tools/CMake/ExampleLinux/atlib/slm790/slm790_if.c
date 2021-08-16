/**
 * @file atlib/slm790/slm790_if.c
 *
 * Copyright (C) 2019
 *
 * slm750_if.c is free software: you can redistribute it and/or modify
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
#include "slm790_if.h"
#include "_slm790_at.h"
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
 * @brief get imei from slm790
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
 * @brief get iccid from slm790
 * @param pbuf: store iccid buf
 *
 * @retval RYAT_E_OK: get iccid success,
 *         RYAT_E_ERROR: get iccid failed.
 */
static int32_t ryat_get_iccid(uint8_t *pbuf)
{
    ryat_cmd_t cmd = {__ms_to_tick(2000), "AT+ICCID\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        // AT+ICCID
        // +ICCID: 89860439101880931012
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
 * @brief get imsi from slm790
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
 * @brief get network mode
 * @param mode: store mode value
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_gprs_get_radio_access(uint8_t *mode)
{
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+MODODR?\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "+MODODR");
        ryat_utils_args_t args = {0};
        
        if(prx != NULL) {
            if(RYAT_E_OK == ryat_utils_get_args(prx->buf, prx->len, &args) &&
               RYAT_E_OK == ryat_utils_get_args(args.pnext, strlen(args.pnext), &args) &&
               ISDIGIT_RET_VALUE != isdigit(args.pval[0])) {
                *mode = (atoi(args.pval) == 3) ? RYAT_RADIO_ACCESS_MODE_2G : RYAT_RADIO_ACCESS_MODE_4G;
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
    char cmdline[20] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_OK | RYAT_RET_ERROR};

    switch(mode) {
        case RYAT_RADIO_ACCESS_MODE_2G:
            set_mode = 3;
            break;
        case RYAT_RADIO_ACCESS_MODE_4G:
            set_mode = 5;
            break;
        default: return RYAT_E_ERROR;
    }
    /* set_mode: 3 --> 2G, 5 --> 4G */
    snprintf(cmdline, sizeof(cmdline), "AT+MODODR=%d\r\n", set_mode);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    return RYAT_E_OK;
}

/**
 * @brief get mnc, mcc, lac and cellid
 * @param pcell: cell info structure
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_get_creg_cell(struct st_ryat_cell_describe *pcell)
{
    uint8_t lac_index = 0, cellid_index = 0, i = 0;
    ryat_cmd_t cmd = {__ms_to_tick(1000), "AT^MONSC\r\n", RYAT_RET_OK | RYAT_RET_ERROR};
    ryat_rx_line_t *prx = NULL;
    ryat_utils_args_t args = {0};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }
    /* AT^MONSC
     * ^MONSC: LTE,460,00,38950,94EF884,142,57B2,-96,-8,-67
     * or
     * ^MONSC: GSM,460,00,1,12,49,2D54,57B2,-68,,
     * OK
     */
    if(NULL == (prx = ryat_get_line_with_prefix(gp_at, "^MONSC"))) {
        return RYAT_E_ERROR;
    }
    if(RYAT_E_OK != ryat_utils_get_args(prx->buf, prx->len, &args) ||
       RYAT_E_OK != ryat_utils_get_args(args.pnext, strlen(args.pnext), &args)) {
        return RYAT_E_ERROR;
    }
    /* judge LTE or GSM */
    if(!strncmp(args.pval, "LTE", strlen("LTE"))) {
        cellid_index = 5;
        lac_index = 7;
    } else if(!strncmp(args.pval, "GSM", strlen("GSM"))) {
        cellid_index = 7;
        lac_index = 8;
    } else {
        return RYAT_E_ERROR;
    }
    i += 2;
    /* mcc */
    if(RYAT_E_OK != ryat_utils_get_args(args.pnext, strlen(args.pnext), &args)) {
        return RYAT_E_ERROR;
    }
    pcell->mcc = strtoul(args.pval, NULL, 10);
    i++;
    /* mnc */
    if(RYAT_E_OK != ryat_utils_get_args(args.pnext, strlen(args.pnext), &args)) {
        return RYAT_E_ERROR;
    }
    pcell->mnc = strtoul(args.pval, NULL, 10);
    do {
        i++;
        if(RYAT_E_OK != ryat_utils_get_args(args.pnext, strlen(args.pnext), &args)) {
            return RYAT_E_ERROR;
        }
    } while(i < cellid_index);
    /* cellid */
    pcell->cellid = strtoul(args.pval, NULL, 16);
    do {
        i++;
        if(RYAT_E_OK != ryat_utils_get_args(args.pnext, strlen(args.pnext), &args)) {
            return RYAT_E_ERROR;
        }
    } while(i < lac_index);
    /* lac */
    pcell->lac = strtoul(args.pval, NULL, 16);

    return RYAT_E_OK;
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
 * @brief get the slm790 ip address
 * @param ipbuf: store ip
 * @param buflen: buf length
 *
 * @retval RYAT_E_OK: get ip address success,
 *         RYAT_E_ERROR: get ip address success.
 */
static int32_t ryat_gprs_active(uint8_t *ipbuf, uint16_t buflen)
{
    ryat_cmd_t cmd = {__ms_to_tick(30000), "AT+MIPCALL=1\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_MIPCALL);
        if(prx != NULL) {
            // AT+MIPCALL=1
            // +MIPCALL: 1,10.2.113.184
            // OK
            char *p = strstr(prx->buf, ",");
            if(p != NULL) {
                strlcpy((char *)ipbuf, ++p, buflen);
                return RYAT_E_OK;
            }
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
    ryat_cmd_t cmd = {__ms_to_tick(2000), "AT+MIPCALL=0\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

/**
 * @brief open or close hex mode
 * @param ishex: true: open hex mode
 *               false: close hex mode
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_socket_init(bool ishex)
{
    char cmdline[16] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_OK | RYAT_RET_ERROR};

    snprintf(cmdline, sizeof(cmdline), "AT+MIPHEX=%d\r\n", ishex);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

/**
 * @brief connect the udp
 * @param domain_ip: remote ip
 * @param port: remote port
 * @param ch: socket id
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_socket_udp(char *domain_ip, uint16_t port, uint8_t ch)
{
    char cmdline[64] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_ERROR | RYAT_RET_OK};
    ryat_rx_line_t *prx = NULL;
    ryat_utils_args_t args = {0};
    bool is_ip = true;
    uint8_t domain_ip_len = strlen(domain_ip);

    for(uint8_t i = 0; i < domain_ip_len; ++i) {
        if(ISDIGIT_RET_VALUE == isdigit(domain_ip[i]) && domain_ip[i] != '.') {
            is_ip = false;
            break;
        }
    }
    // dns
    // AT+MIPDNSR="niot.dcgc.sluan.org"
    // +MIPDNSR:121.40.233.196
    // OK
    if(is_ip == false) {
        sprintf(cmdline, "AT+MIPDNSR=\"%s\"\r\n", domain_ip);
        cmd.cmd = cmdline;
        cmd.cmdlen = strlen(cmd.cmd);
        if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
            return RYAT_E_ERROR;
        }
        prx = ryat_get_line_with_prefix(gp_at, "+MIPDNSR");
        if(prx == NULL || RYAT_E_OK != ryat_utils_get_args(prx->buf, prx->len, &args) ||
           RYAT_E_OK != ryat_utils_get_args(args.pnext, strlen(args.pnext), &args)) {
            return RYAT_E_ERROR;
        }
        domain_ip = args.pval;
    }

    sprintf(cmdline, "AT+MIPOPEN=%d,0,\"%s\",%d,1\r\n", ch, domain_ip, port);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(10000);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_MIPOPEN);
        if(prx != NULL) {
            char *p = strstr(prx->buf, ",");
            if(p != NULL && p[1] == '1') {
                return RYAT_E_OK;
            }
        }
    }
    return RYAT_E_ERROR;
}

/**
 * @brief write data to slm790
 * @param pdata: data
 * @param len: data length
 * @param ch: socket id
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_write_data(uint8_t *pdata, uint16_t len, uint8_t ch)
{
    char cmdbuf[32] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(200), NULL, RYAT_RET_ERROR};
    
    sprintf(cmdbuf, "AT+MIPTPS=3,%d,0,%d\r\n", ch, len);
    cmd.cmd = cmdbuf;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_ERROR == ryat_set(gp_at, &cmd)) {
        return RYAT_E_ERROR;
    }

    cmd.cmd = (char *)pdata;
    cmd.cmdlen = len;
    cmd.ret = RYAT_RET_ERROR | RYAT_RET_OK | RYAT_RET_CME;
    cmd.tick = __ms_to_tick(20000);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        return len;
    }
    return RYAT_E_ERROR;
}

/**
 * @brief close socket
 * @param ch: socket id
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_socket_shut(uint8_t ch)
{
    char cmdline[64] = {0};
    ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_MIPCLOSE | RYAT_RET_ERROR};

    sprintf(cmdline, "AT+MIPCLOSE=%d\r\n", ch);
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_MIPCLOSE == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

static int32_t ryat_read_cache(uint8_t ch, uint8_t *pdata, uint16_t len, uint16_t *remain, uint32_t timeout)
{
    return _slm790_at_get_data(gp_at, ch, pdata, len, timeout);
}

/**
 * @brief get module model name
 * @param buf: buf to store module model name
 * @param buf_len: buf length
 *
 * @retval RYAT_E_OK or RYAT_E_ERROR
 */
static int32_t ryat_get_model(char *buf, uint16_t buf_len)
{
    ryat_cmd_t cmd = {__ms_to_tick(500), "AT+GMM\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_GMM);
        ryat_utils_args_t args = {0};

        if(prx == NULL || RYAT_E_OK != ryat_utils_get_args(prx->buf, prx->len, &args) ||
           args.pnext == NULL || RYAT_E_OK != ryat_utils_get_args(args.pnext, strlen(args.pnext), &args)) {
            return RYAT_E_ERROR;
        }
        strlcpy(buf, args.pval, buf_len);
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

struct st_ryat_interface const ryat_interface_slm790 = {
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
    .socket_shut = ryat_socket_shut,
    .write_data = ryat_write_data,
    .read_cache = ryat_read_cache,
    .get_creg_cell = ryat_get_creg_cell,
    .select_radio_access = ryat_gprs_select_radio_access,
    .get_radio_access = ryat_gprs_get_radio_access,
    .get_model = ryat_get_model,
};
