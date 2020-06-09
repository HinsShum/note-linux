/**
 * @sim800_if.c
 *
 * @copyright This file create by rensuiyi ,all right reserve!
 *
 * @author rensuiyi
 *
 * @date 2018/2/4 14:04:01
 */

#include "g500_if.h"
#include "_g500_at.h"
#include "ryat.h"
#include "ryat_utils.h"
#include <ctype.h>
#include <stdlib.h>

extern struct st_ryat_describe* gp_at;

/*---------- marco ----------*/
#if defined(__CC_ARM)
#define ISDIGIT_RET_VALUE   (NULL)
#else
#define ISDIGIT_RET_VALUE   (0)
#define strlcpy             strncpy
#endif

/**
 * @brief get the imei
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param pbuf
 *
 * @return err_t
 */
static err_t ryat_get_imei(uint8_t* pbuf) {
  ryat_cmd_t cmd = {__ms_to_tick(500), "AT+GSN\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    // 865233030013438
    ryat_rx_line_t* prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
    if ((prx != NULL) && (prx->len == 15)) {
      memcpy(pbuf, prx->buf, prx->len);
      return RYAT_E_OK;
    }
  }
  return RYAT_E_ERROR;
}

/**
 * @brief get the iccid ,max delay 2s
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param pbuf
 *
 * @return err_t
 */
static err_t ryat_get_iccid(uint8_t* pbuf) {
  ryat_cmd_t cmd = {__ms_to_tick(2000), "AT+CCID\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    // 15 string +CCID: 898600810906F8048812
    ryat_rx_line_t* prx = ryat_get_line_with_prefix(gp_at, "+CCID");
    if (prx != NULL) {
      char *p = prx->buf;
      for (uint16_t i = 0; (i < RYAT_COMMAND_LINE_LENGTH) && (ISDIGIT_RET_VALUE == isdigit(*++p)); ++i);
      if (p < (prx->buf + RYAT_COMMAND_LINE_LENGTH)) {
        memcpy(pbuf, p, strlen(p));
      }
      return RYAT_E_OK;
    }
  }
  return RYAT_E_ERROR;
}

/**
 * @brief get the imsi,max delay 20s
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param pbuf
 *
 * @return err_t
 */
static err_t ryat_get_imsi(uint8_t* pbuf) {
  ryat_cmd_t cmd = {__ms_to_tick(2000), "AT+CIMI\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    // 15 string 460008184101641
    ryat_rx_line_t* prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
    if ((prx != NULL) && (prx->len == 15)) {
      memcpy(pbuf, prx->buf, prx->len);
      return RYAT_E_OK;
    }
  }
  return RYAT_E_ERROR;
}

/**
 * @brief set the apn with the name
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param apn
 * @param name
 * @param passwd
 *
 * @return err_t
 */
static err_t ryat_gprs_set_apn(char* apn, char* name, char* passwd) {
  char cmdline[64] = {"AT+CGDCONT=1,\"IP\",\""};
  ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_OK | RYAT_RET_ERROR};

  if (apn == NULL) {
    return RYAT_E_OK;
  }

  strcat(cmdline, apn);
  strcat(cmdline, "\"\r\n");

  cmd.cmd = cmdline;
  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }

  /*
   * if has the user name
   */

  if (name != NULL) {
    strcpy(cmdline, "AT+MGAUTH=1,2,\"");
    strcat(cmdline, name);
    strcat(cmdline, "\",\"");
    if (passwd != NULL) {
      strcat(cmdline, passwd);
    }
    strcat(cmdline, "\"\r\n");
    cmd.cmd = cmdline;
    cmd.cmdlen = strlen(cmd.cmd);
    if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
      return RYAT_E_OK;
    }
  }

  return RYAT_E_OK;
}

/**
 * @brief
 *
 *
 * @author rensuiyi (2018/2/9)
 *
 * @param pcell
 *
 * @return err_t
 */
static int32_t ryat_get_creg_cell(struct st_ryat_cell_describe *pcell) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CGREG=2\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  memset(pcell, 0, sizeof(struct st_ryat_cell_describe));
  /*
   * set the creg=2 first
   */
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }

  /*
   * get the creg status
   */
  cmd.cmd = "AT+CGREG?\r\n";
  cmd.cmdlen = strlen(cmd.cmd);
  cmd.tick = __ms_to_tick(45000);
  cmd.ret = RYAT_RET_OK | RYAT_RET_CME;

  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    ryat_rx_line_t *prx;
    ryat_utils_args_t args;

    prx = ryat_get_line_with_type(gp_at, RYAT_RET_CEREG);
    if (prx != NULL) {
      //+CREG: 0,0,XXX,YYY
      char *p = NULL;
      p = strstr(prx->buf, ":");
      if (p == NULL) {
        return RYAT_E_ERROR;
      }
      ++p;
      if (RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args)) {
        return RYAT_E_ERROR;
      }
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
  cmd.cmd = "AT+COPS=3,2\r\n";
  cmd.cmdlen = strlen(cmd.cmd);
  cmd.tick = __ms_to_tick(1000);
  cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }
  cmd.cmd = "AT+COPS?\r\n";
  cmd.cmdlen = strlen(cmd.cmd);
  cmd.tick = __ms_to_tick(1000);
  cmd.ret = RYAT_RET_OK | RYAT_RET_CME;
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
 * @brief only get the ip address
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param ipbuf
 * @param buflen
 *
 * @return err_t
 */
static err_t ryat_gprs_active(uint8_t* ipbuf, uint16_t buflen) {
  ryat_cmd_t cmd = {__ms_to_tick(30000), "AT+MIPCALL=1\r\n", RYAT_RET_MIPCALL | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_MIPCALL != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }

  gp_at->delay_tick(__ms_to_tick(1000));
  // get the ip
  cmd.cmd = "AT+MIPCALL?\r\n";
  cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
  cmd.tick = __ms_to_tick(3000);
  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    ryat_utils_args_t args;
    ryat_rx_line_t* prx = ryat_get_line_with_type(gp_at, RYAT_RET_MIPCALL);
    if ((prx != NULL) && (prx->len > 12)) {
      ipbuf[buflen - 1] = 0x00;
      if (RYAT_E_OK == ryat_utils_get_args(prx->buf + 12, prx->len - 12, &args)) {
        strncpy((char *)ipbuf, args.pval, buflen - 1);
        return RYAT_E_OK;
      }
    }
  }
  return RYAT_E_ERROR;
}

/**
 * @brief gprs deactivate
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @return err_t
 */
static err_t ryat_gprs_deactivate(void) {
  ryat_cmd_t cmd = {__ms_to_tick(20000), "AT+MIPCALL=0\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    return RYAT_E_OK;
  }
  return RYAT_E_ERROR;
}

/**
 * @brief initial the socket
 *        SET MUX=1 should be called before gprs active
 *
 *
 *
 * @author rensuiyi (2018/2/8)
 *
 * @param void
 *
 * @return err_t
 */
static err_t ryat_socket_init(bool iscached) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+GTSET=\"IPRFMT\",2\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  // iscached
  if (iscached) {
    cmd.cmd = "AT+GTSET=\"IPRFMT\",5\r\n";
  }
  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }
  return RYAT_E_OK;
}

/**
 * @brief connect the udp with 160s timeout
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param domain_ip
 * @param port
 *
 * @return err_t
 */
static err_t ryat_socket_udp(char* domain_ip, uint16_t port, uint8_t ch) {
  char cmdline[64];
  ryat_res_t ret;
  ryat_cmd_t cmd = {__ms_to_tick(30000), NULL, RYAT_RET_ERROR | RYAT_RET_CME | RYAT_RET_MIPOPEN};

  sprintf(cmdline, "AT+MIPOPEN=%d,,\"%s\",%d,1\r\n", ch, domain_ip, port);

  cmd.cmd = cmdline;
  cmd.cmdlen = strlen(cmd.cmd);
  ret = ryat_set(gp_at, &cmd);
  if (RYAT_RET_MIPOPEN == ret) {
    return RYAT_E_OK;
  }
  return RYAT_E_ERROR;
}

/**
 * @brief
 *
 *
 * @author rensuiyi (2018/2/8)
 *
 * @param pdata
 * @param len
 *
 * @return err_t
 */
static err_t ryat_write_data(uint8_t* pdata, uint16_t len, uint8_t ch) {
  char cmdbuf[32];
  ryat_res_t result;
  ryat_cmd_t cmd = {__ms_to_tick(100), NULL, RYAT_RET_ERROR};
  sprintf(cmdbuf, "AT+MIPSEND=%d,%d\r\n", ch, len);

  cmd.cmd = cmdbuf;
  cmd.cmdlen = strlen(cmd.cmd);
  result = ryat_set(gp_at, &cmd);
  if ((RYAT_RET_CME == result) || (result == RYAT_RET_ERROR)) {
    return RYAT_E_ERROR;
  }

  cmd.cmd = (char *)pdata;
  cmd.ret = RYAT_RET_CME | RYAT_RET_ERROR | RYAT_RET_MIPSEND;
  cmd.tick = __ms_to_tick(20000);
  cmd.cmdlen = len;

  if (RYAT_RET_MIPSEND == ryat_set(gp_at, &cmd)) {
    return len;
  }
  return RYAT_E_ERROR;
}

/**
 * @brief connect the tcp with 160s timeout
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param domain_ip
 * @param port
 *
 * @return err_t
 */
static err_t ryat_socket_tcp(char* domain_ip, uint16_t port, uint8_t ch) {
  char cmdline[64];
  ryat_res_t ret;
  ryat_cmd_t cmd = {__ms_to_tick(30000), NULL, RYAT_RET_ERROR | RYAT_RET_CME | RYAT_RET_MIPOPEN};

  sprintf(cmdline, "AT+MIPOPEN=%d,,\"%s\",%d,0\r\n", ch, domain_ip, port);

  cmd.cmd = cmdline;
  cmd.cmdlen = strlen(cmd.cmd);
  ret = ryat_set(gp_at, &cmd);
  if (RYAT_RET_MIPOPEN == ret) {
    return RYAT_E_OK;
  }
  return RYAT_E_ERROR;
}

/**
 * @brief close the connecting socket or the connected socket
 *
 *
 * @author rensuiyi (2018/2/8)
 *
 * @return err_t
 */
static err_t ryat_socket_shut(uint8_t ch) {
  char cmdline[64];
  ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_MIPCLOSE | RYAT_RET_ERROR};

  sprintf(cmdline, "AT+MIPCLOSE=%d\r\n", ch);

  cmd.cmd = cmdline;
  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_MIPCLOSE == ryat_set(gp_at, &cmd)) {
    return RYAT_E_OK;
  }
  return RYAT_E_ERROR;
}

/**
 * @brief read the data from the cache
 *
 *
 * @author rensuiyi (2018/2/8)
 *
 * @param pdata
 * @param len
 * @param remain
 *
 * @return err_t
 */
static err_t ryat_read_cache(uint8_t ch, uint8_t* pdata, uint16_t len, uint16_t* remain, uint32_t timeout) {
  return _g500_at_get_data(gp_at, ch, pdata, len, timeout);
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
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "G500");
        ryat_utils_args_t args = {0};

        if(prx == NULL || RYAT_E_OK != ryat_utils_get_args(prx->buf, prx->len, &args)) {
            return RYAT_E_ERROR;
        }
        strlcpy(buf, args.pval, buf_len);
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

struct st_ryat_interface const ryat_interface_g500 = {
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
