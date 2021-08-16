/**
 * @sim800_if.c
 *
 * @copyright This file create by rensuiyi ,all right reserve!
 *
 * @author rensuiyi
 *
 * @date 2018/2/4 14:04:01
 */
#include "sim800_if.h"
#include "_sim800_at.h"
#include "ryat.h"
#include "ryat_utils.h"

extern struct st_ryat_describe* gp_at;

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
static int32_t ryat_get_imei(uint8_t* pbuf) {
  ryat_cmd_t cmd = {__ms_to_tick(500), "AT+GSN\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    // 15 string 865233030013438
    ryat_rx_line_t* prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
    if ((prx != NULL) && (prx->len == 15)) {
      memcpy(pbuf, prx->buf, 15);
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
static int32_t ryat_get_iccid(uint8_t* pbuf) {
  ryat_cmd_t cmd = {__ms_to_tick(2000), "AT+ICCID\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    // 15 string +ICCID: 898600810906F8048812
    ryat_rx_line_t* prx = ryat_get_line_with_type(gp_at, RYAT_RET_P);
    if ((prx != NULL) && (prx->len == 28)) {
      memcpy(pbuf, prx->buf + 8, 20);
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
static int32_t ryat_get_imsi(uint8_t* pbuf) {
  ryat_cmd_t cmd = {__ms_to_tick(20000), "AT+CIMI\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    // 15 string 460008184101641
    ryat_rx_line_t* prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
    if ((prx != NULL) && (prx->len == 15)) {
      memcpy(pbuf, prx->buf, 15);
      return RYAT_E_OK;
    }
  }
  return RYAT_E_ERROR;
}

/**
 * @brief
 *
 *
 * @author rensuiyi (2018/2/8)
 *
 * @param apn
 * @param buflen
 *
 * @return err_t
 */
static int32_t ryat_gprs_get_apn(char* apn, uint16_t buflen) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CGNAPN\r\n", RYAT_RET_OK};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    ryat_rx_line_t* prx = NULL;
    ryat_utils_args_t args;

    prx = ryat_get_line_with_prefix(gp_at, "+CGNAPN:");
    if ((prx != NULL) && (prx->len >= 11)) {
      if (RYAT_E_OK == ryat_utils_get_args(prx->buf + 11, prx->len - 11, &args)) {
        if (buflen + 1 < args.len) {
          return RYAT_E_WRONG_ARGS;
        }
        strncpy(apn, args.pval, buflen);
        return RYAT_E_OK;
      }
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
static int32_t ryat_gprs_set_apn(char* apn, char* name, char* passwd) {
  char cmdline[64] = {"AT+CSTT=\""};
  ryat_cmd_t cmd = {__ms_to_tick(1000), NULL, RYAT_RET_OK | RYAT_RET_ERROR};

  if (apn == NULL) {
    return RYAT_E_OK;
  }

  // AT+CSTT="apn","username","password"
  strcat(cmdline, apn);
  strcat(cmdline, "\",\"");
  if (name != NULL) {
    strcat(cmdline, name);
  }
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
  return RYAT_E_ERROR;
}

/**
 * @brief
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param ipbuf
 * @param buflen
 *
 * @return err_t
 */
static int32_t ryat_gprs_active(uint8_t* ipbuf, uint16_t buflen) {
  ryat_cmd_t cmd = {__ms_to_tick(85000), "AT+CIICR\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    // net ok ,get ip addr next
    cmd.cmd = "AT+CIFSR\r\n";
    cmd.tick = __ms_to_tick(500);
    cmd.ret = RYAT_RET_OK | RYAT_RET_UNKNOWN;
    cmd.cmdlen = strlen(cmd.cmd);
    if (RYAT_RET_UNKNOWN == ryat_set(gp_at, &cmd)) {
      ryat_rx_line_t* prx = ryat_get_line_with_type(gp_at, RYAT_RET_UNKNOWN);
      if (prx != NULL) {
        ipbuf[buflen - 1] = 0x00;
        strncpy((char*)ipbuf, prx->buf, buflen - 1);
      }
    }
    return RYAT_E_OK;
  }
  return RYAT_E_ERROR;
}

/**
 * @brief gprs disactive
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @return err_t
 */
static int32_t ryat_gprs_deactivate(void) {
  ryat_cmd_t cmd = {__ms_to_tick(65000), "AT+CIPSHUT\r\n", RYAT_RET_SHUTOK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_SHUTOK == ryat_set(gp_at, &cmd)) {
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
static int32_t ryat_socket_init(bool iscached) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CIPMUX=0\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

#if 0
  //cipmux = 0
  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }
#endif

  // cip header
  cmd.cmd = "AT+CIPHEAD=1\r\n";
  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }

  // iscached
  if (iscached) {
    cmd.cmd = "AT+CIPRXGET=1\r\n";
  } else {
    cmd.cmd = "AT+CIPRXGET=0\r\n";
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
static int32_t ryat_socket_udp(char* domain_ip, uint16_t port, uint8_t ch) {
  char cmdline[64];
  ryat_res_t ret;
  ryat_cmd_t cmd = {__ms_to_tick(160000), NULL, RYAT_RET_ERROR | RYAT_RET_CME | RYAT_RET_ALREADY_CON | RYAT_RET_CON_FAIL | RYAT_RET_CON_OK};

  sprintf(cmdline, "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n", domain_ip, port);

  cmd.cmd = cmdline;
  cmd.cmdlen = strlen(cmd.cmd);
  ret = ryat_set(gp_at, &cmd);
  if ((RYAT_RET_CON_OK == ret) || (RYAT_RET_ALREADY_CON == ret)) {
    return RYAT_E_OK;
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
static int32_t ryat_socket_tcp(char* domain_ip, uint16_t port, uint8_t ch) {
  char cmdline[64];
  ryat_res_t ret;
  ryat_cmd_t cmd = {__ms_to_tick(160000), NULL, RYAT_RET_ERROR | RYAT_RET_CME | RYAT_RET_ALREADY_CON | RYAT_RET_CON_FAIL | RYAT_RET_CON_OK};

  sprintf(cmdline, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", domain_ip, port);

  cmd.cmd = cmdline;
  cmd.cmdlen = strlen(cmd.cmd);
  ret = ryat_set(gp_at, &cmd);
  if ((RYAT_RET_CON_OK == ret) || (RYAT_RET_ALREADY_CON == ret)) {
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
static int32_t ryat_socket_shut(uint8_t ch) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CIPCLOSE\r\n", RYAT_RET_CLOSE_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_CLOSE_OK == ryat_set(gp_at, &cmd)) {
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
static int32_t ryat_write_data(uint8_t* pdata, uint16_t len, uint8_t ch) {
  char cmdbuf[32];
  ryat_cmd_t cmd = {__ms_to_tick(100), NULL, RYAT_RET_CME};
  sprintf(cmdbuf, "AT+CIPSEND=%d\r\n", len);

  cmd.cmd = cmdbuf;
  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_CME == ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
  }

  cmd.cmd = (char*)pdata;
  cmd.cmdlen = len;
  cmd.ret = RYAT_RET_CME | RYAT_RET_SEND_FAILD | RYAT_RET_SEND_OK;
  cmd.tick = __ms_to_tick(20000);

  if (RYAT_RET_SEND_OK == ryat_set(gp_at, &cmd)) {
    return len;
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
static int32_t ryat_read_cache(uint8_t ch, uint8_t* pdata, uint16_t len, uint16_t* remain, uint32_t timeout) {
  return _sim800_at_get_data(gp_at, pdata, len, remain, timeout);
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
static err_t ryat_get_creg_cell(struct st_ryat_cell_describe* pcell) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CREG=2\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

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
  cmd.cmd = "AT+CREG?\r\n";
  cmd.tick = __ms_to_tick(45000);
  cmd.ret = RYAT_RET_OK | RYAT_RET_CME;
  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    ryat_rx_line_t* prx;
    ryat_utils_args_t args;

    prx = ryat_get_line_with_prefix(gp_at, "+CREG");
    if ((prx != NULL) && (prx->len > 24)) {
      //+CREG: 0,0,XXX,YYY
      if (RYAT_E_OK != ryat_utils_get_args(prx->buf + 11, prx->len - 11, &args)) {
        return RYAT_E_ERROR;
      }
      pcell->lac = strtoul(args.pval, NULL, 16);

      if (RYAT_E_OK != ryat_utils_get_args(prx->buf + 17, prx->len - 17, &args)) {
        return RYAT_E_ERROR;
      }
      pcell->cellid = strtoul(args.pval, NULL, 16);
      return RYAT_E_OK;
    }
  }
  return RYAT_E_ERROR;
}

/**
 * @brief change the if mode
 *
 *
 * @author rensuiyi (2018/2/8)
 *
 * @param mode
 *
 * @return err_t
 */
static int32_t sim800_if_switch_mode(uint8_t mode) {
  ryat_cmd_t cmd = {__ms_to_tick(1000), "AT+CNMP=2\r\n", RYAT_RET_OK | RYAT_RET_CME};

  if (mode == RYAT_RADIO_ACCESS_MODE_2G) {
    cmd.cmd = "AT+CNMP=13\r\n";
  }
  if (mode == RYAT_RADIO_ACCESS_MODE_4G) {
    cmd.cmd = "AT+CNMP=38\r\n";
  }
  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    return RYAT_E_OK;
  }
  return RYAT_E_ERROR;
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
        ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "SIM800");
        ryat_utils_args_t args = {0};

        if(prx == NULL || RYAT_E_OK != ryat_utils_get_args(prx->buf, prx->len, &args)) {
            return RYAT_E_ERROR;
        }
        strlcpy(buf, args.pval, buf_len);
        return RYAT_E_OK;
    }
    return RYAT_E_ERROR;
}

struct st_ryat_interface const ryat_interface_sim800 = {
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
    .get_apn = ryat_gprs_get_apn,
    .active = ryat_gprs_active,
    .deactivate = ryat_gprs_deactivate,
    .socket_init = ryat_socket_init,
    .socket_udp = ryat_socket_udp,
    .socket_tcp = ryat_socket_tcp,
    .socket_shut = ryat_socket_shut,
    .write_data = ryat_write_data,
    .read_cache = ryat_read_cache,
    .get_creg_cell = ryat_get_creg_cell,
    .select_radio_access = sim800_if_switch_mode,
    .get_model = ryat_get_model,
};