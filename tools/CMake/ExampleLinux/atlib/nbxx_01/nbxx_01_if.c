/**
 * nb05_01_if.c
 *
 * Copyright (C) 2018 HinsShum
 *
 * nb05_01_if.c is free software: you can redistribute it and/or modify
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
#include "nbxx_01_if.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ryat_utils.h"
#include "_nbxx_01_at.h"

/*---------- marco ----------*/
#define MAX_NB_BUF_LENGTH (100)

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
    ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_P);
    if (prx != NULL) {
      char *p = prx->buf;

      for (uint16_t i = 0; (i < RYAT_COMMAND_LINE_LENGTH) && (0 == isdigit(*++p)); ++i)
        ;
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
  ryat_cmd_t cmd = {__ms_to_tick(500), "AT+NCCID\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    // 20 string +NCCID:89860317492043342789
    ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_P);
    if (prx != NULL) {
      char *p = prx->buf;

      for (uint16_t i = 0; (i < RYAT_COMMAND_LINE_LENGTH) && (0 == isdigit(*++p)); ++i)
        ;
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
 * ryat_get_creg_cell() - get cell
 * @pcell:
 *
 * retval: int32_t
 */
static int32_t ryat_get_creg_cell(struct st_ryat_cell_describe *pcell) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CEREG=2\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  memset(pcell, 0, sizeof(struct st_ryat_cell_describe));
  /* set the creg=2 first */
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }

  /* get the creg status */
  cmd.cmd = "AT+CEREG?\r\n";
  cmd.cmdlen = strlen(cmd.cmd);
  cmd.tick = __ms_to_tick(45000);
  cmd.ret = RYAT_RET_OK | RYAT_RET_CME;

  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    ryat_rx_line_t *prx;
    ryat_utils_args_t args;

    prx = ryat_get_line_with_type(gp_at, RYAT_RET_CEREG);
    if (prx != NULL) {
      //+CEREG:2,1,XXX,YYY,Z
      char *p = NULL;

      p = strstr(prx->buf, ":");
      if (p == NULL) {
        return RYAT_E_ERROR;
      }
      ++p;
      if (RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
        return RYAT_E_ERROR;
      }
      /* get register state */
      if ((args.pnext == NULL) || (isdigit(args.pnext[0]) == 0) || (args.pnext[0] != '1')) {
        return RYAT_E_ERROR;
      }
      p = strstr(args.pnext, ",");
      if (p == NULL) {
        return RYAT_E_ERROR;
      }
      ++p;
      if (RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args) || args.pnext == NULL) {
        return RYAT_E_ERROR;
      }
      pcell->lac = strtoul(args.pval, NULL, 16);
      p = args.pnext;
      if (RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
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
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    ryat_rx_line_t *prx;
    ryat_utils_args_t args;

    prx = ryat_get_line_with_type(gp_at, RYAT_RET_COPS);
    if (prx != NULL) {
      char *p = prx->buf;

      for (uint8_t i = 0; i < 2; ++i) {
        p = strstr(++p, ",");
        if (p == NULL) {
          return RYAT_E_ERROR;
        }
      }
      if (RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
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
 * ryat_gprs_deactivate() - For this command, the term roaming
 * corresponds to being registered to a VPLMN which is not
 * equivalent to HPLMN or EHPLMN
 * @note: AT+CIPCA=n,attachwithoutPDN
 *        Only n = 3 is supported
 *
 * retval: RYAT_E_ERROR/RYAT_E_OK
 */
static int32_t ryat_gprs_deactivate(void) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CIPCA=3,0\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    return RYAT_E_OK;
  }

  return RYAT_E_ERROR;
}

/**
 * ryat_gprs_set_apn() - set cfun code equal 1, set apn
 * @apn: apn name
 * @name: apn user name
 * @pass: apn user pass
 *
 * retval: RYAT_E_OK/RYAT_E_ERROR
 */
static int32_t ryat_gprs_set_apn(char *apn, char *name, char *pass) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+NCONFIG?\r\n", RYAT_RET_OK};
  ryat_utils_args_t args;
  ryat_rx_line_t *prx = NULL;
  char *p = NULL;

  cmd.cmdlen = strlen(cmd.cmd);
  /* set nconfig */
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }
  for (; gp_at->cache.rxhead < gp_at->cache.rxtail; gp_at->cache.rxhead++) {
    p = strstr(gp_at->cache.line[gp_at->cache.rxhead].buf, "AUTOCONNECT,FALSE");
    if (p != NULL) {
      break;
    }
  }
  if (p == NULL) {
    /* set autoconnect false */
    cmd.cmd = "AT+NCONFIG=AUTOCONNECT,FALSE\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(1000);
    cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
    ryat_set(gp_at, &cmd);
    /* reboot module */
    cmd.cmd = "AT+NRB\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(0);
    cmd.ret = RYAT_RET_OK;
    ryat_set(gp_at, &cmd);

    return RYAT_E_ERROR;
  }

  /* get cfun code */
  cmd.cmd = "AT+CFUN?\r\n";
  cmd.cmdlen = strlen(cmd.cmd);
  cmd.tick = __ms_to_tick(1000);
  cmd.ret = RYAT_RET_OK;
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }
  prx = ryat_get_line_with_type(gp_at, RYAT_RET_P);
  if (prx == NULL) {
    return RYAT_E_ERROR;
  }
  p = strstr(prx->buf, ":");
  if (p == NULL) {
    return RYAT_E_ERROR;
  }
  ++p;
  if (RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args) || 0 == isdigit(args.pval[0])) {
    return RYAT_E_ERROR;
  }
  if (args.pval[0] == '1') {
    return RYAT_E_OK;
  }
  /* set cfun code equal 1 */
  cmd.cmd = "AT+CFUN=1\r\n";
  cmd.cmdlen = strlen(cmd.cmd);
  cmd.tick = __ms_to_tick(10000);
  cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }
  /* set apn */
  char cmdline[64] = {0};

  snprintf(cmdline, sizeof(cmdline), "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", apn);
  cmd.cmd = cmdline;
  cmd.cmdlen = strlen(cmd.cmd);
  cmd.tick = __ms_to_tick(1000);
  cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    cmd.cmd = "AT+CFUN=0\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(1000);
    cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
    ryat_set(gp_at, &cmd);

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
static int32_t ryat_gprs_active(uint8_t *pbuf, uint16_t len) {
  char *p = NULL;
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CGPADDR\r\n", RYAT_RET_OK};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }
  for (; gp_at->cache.rxhead < gp_at->cache.rxtail;) {
    ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "+CGPADDR");

    if (prx == NULL) {
      return RYAT_E_ERROR;
    }
    p = strstr(prx->buf, ",");
    if (p != NULL) {
      ++p;
      strncpy((char *)pbuf, p, len);
      break;
    }
  }

  return (p != NULL) ? RYAT_E_OK : RYAT_E_ERROR;
}

/**
 * ryat_socket_udp() - configure ncdp, use coap protocol
 * @domain_ip: ipdomain
 * @port: coap port, it must be 5683 when use Neul Hi2110
 * @ch: no use
 *
 * retval: RYAT_E_OK/RYAT_E_ERROR
 */
static int32_t ryat_socket_udp(char *domain_ip, uint16_t port, uint8_t ch) {
  char cmdline[64];
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+NCDP?\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "+NCDP");

    if (prx != NULL) {
      snprintf(cmdline, sizeof(cmdline), "%s,%u", domain_ip, port);
      if (strstr(prx->buf, cmdline) != NULL) {
        return RYAT_E_OK;
      }
    }
  }
  /* set ncdp server */
  snprintf(cmdline, sizeof(cmdline), "AT+NCDP=%s,%u\r\n", domain_ip, port);
  cmd.cmd = cmdline;
  cmd.cmdlen = strlen(cmd.cmd);
  ryat_set(gp_at, &cmd);
  /* reboot */
  cmd.cmd = "AT+NRB\r\n";
  cmd.cmdlen = strlen(cmd.cmd);
  ryat_set(gp_at, &cmd);

  return RYAT_E_ERROR;
}

/**
 * ryat_write_data() - write data to module
 * @pdata: data
 * @len: data length
 * @ch: no use
 *
 * retval:
 */
static int32_t ryat_write_data(uint8_t *pdata, uint16_t len, uint8_t ch) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+NNMI?\r\n", RYAT_RET_OK};
  uint8_t buf[(MAX_NB_BUF_LENGTH << 1)] = {0};
  ryat_rx_line_t *prx = NULL;

  if (len > MAX_NB_BUF_LENGTH) {
    return RYAT_E_WRONG_ARGS;
  }
  /* set NNMI */
  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }
  prx = ryat_get_line_with_type(gp_at, RYAT_RET_NNMI);
  if (prx == NULL) {
    return RYAT_E_ERROR;
  }
  if (strstr(prx->buf, "2") == NULL) {
    cmd.cmd = "AT+NNMI=2\r\n";
    cmd.cmdlen = strlen(cmd.cmd);
    cmd.tick = __ms_to_tick(1000);
    cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
    if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
      return RYAT_E_ERROR;
    }
  }
  /* send data */
  snprintf((char *)buf, sizeof(buf), "AT+NMGS=%u,", len);
  // hex to hex string format
  for (uint8_t i = 0; i < len; ++i) {
    char *p = (char *)buf + strlen((char *)buf);
    uint8_t *c = pdata + i;
    sprintf(p, "%02X", *c);
  }
  strcat((char *)buf, "\r\n");
  cmd.cmd = (char *)buf;
  cmd.cmdlen = strlen(cmd.cmd);
  cmd.tick = __ms_to_tick(5000);
  cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    return RYAT_E_OK;
  }

  return RYAT_E_ERROR;
}

/**
 * ryat_read_cache() - get data from module
 * @pdata: store data
 * @len: buf length
 * @remain: remain data length
 *
 * retval: RYAT_E_ERROR/data length
 */
static int32_t ryat_read_cache(uint8_t ch, uint8_t *pdata, uint16_t len, uint16_t *remain, uint32_t timeout) {
  return _nbxx_01_at_get_data(gp_at, pdata, len, remain, timeout);
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
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "Hi211X");
        ryat_utils_args_t args = {0};

        if(prx == NULL || RYAT_E_OK != ryat_utils_get_args(prx->buf, prx->len, &args)) {
            return RYAT_E_ERROR;
        }
        strlcpy(buf, args.pval, buf_len);
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

struct st_ryat_interface const ryat_interface_nbxx_01 = {
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
    .write_data = ryat_write_data,
    .read_cache = ryat_read_cache,
    .get_creg_cell = ryat_get_creg_cell,
    .get_model = ryat_get_model,
};
