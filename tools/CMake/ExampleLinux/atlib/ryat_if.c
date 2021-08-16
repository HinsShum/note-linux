/**
 * @ryat_if.c
 *
 * @copyright This file create by rensuiyi ,all right reserve!
 *
 * @author rensuiyi
 *
 * @date 2018/2/9 10:51:29
 */
#include "ryat.h"
#include "ryat_if.h"
#include "ctype.h"
#include "ryat_utils.h"

/* marcos */
#if defined(__CC_ARM)
#define ISDIGIT_RET_VALUE   (NULL)
#else
#define ISDIGIT_RET_VALUE   (0)
#endif

struct st_ryat_describe *gp_at;
static uint8_t module_model_type = 0xFF;

/**
 * @brief test the at command
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param void
 *
 * @return err_t
 */
int32_t __attribute__((weak)) ryat_test(void) {
  ryat_cmd_t cmd = {__ms_to_tick(500), "AT\r\n", RYAT_RET_OK};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    return RYAT_E_OK;
  }
  return RYAT_E_ERROR;
}

/**
 * @brief close the echo
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @return err_t
 */
int32_t __attribute__((weak)) ryat_close_echo(void) {
  ryat_cmd_t cmd = {__ms_to_tick(500), "ATE0\r\n", RYAT_RET_OK};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    return RYAT_E_OK;
  }
  return RYAT_E_ERROR;
}

/**
 * ryat_shutdown() - shutdown module
 *
 * retval: SL_ERROR/SL_EOK
 */
int32_t __attribute__((weak)) ryat_shutdown(void)
{
    ryat_cmd_t cmd = {
        __ms_to_tick(5000), "AT+CFUN=0\r\n", RYAT_RET_OK | RYAT_RET_ERROR
    };

    cmd.cmdlen = strlen(cmd.cmd);
    if(RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
        return RYAT_E_OK;
    }

    return RYAT_E_ERROR;
}

/**
 * @brief get the csq
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param csq
 *
 * @return err_t
 */
int32_t __attribute__((weak)) ryat_get_csq(uint8_t *csq) {
  ryat_cmd_t cmd = {__ms_to_tick(500), "AT+CSQ\r\n", RYAT_RET_OK};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    // respond +CSQ: xx,xx
    char *p = NULL;
    ryat_utils_args_t args;
    ryat_rx_line_t *prx = ryat_get_line_with_type(gp_at, RYAT_RET_CSQ);
    if (prx == NULL) {
      return RYAT_E_ERROR;
    }
    p = strstr(prx->buf, ":");
    ++p;
    if (RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args) || ISDIGIT_RET_VALUE == isdigit(args.pval[0])) {
      return RYAT_E_ERROR;
    }
    *csq = atoi(args.pval);
    return RYAT_E_OK;
  }
return RYAT_E_ERROR;
}

/**
 * @brief attach the gprs ,max delay is 10S
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @return err_t
 */
int32_t __attribute__((weak)) ryat_gprs_attach(void) {
  ryat_cmd_t cmd = {__ms_to_tick(30000), "AT+CGATT=1\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

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
 * @param void
 *
 * @return err_t
 */
int32_t __attribute__((weak)) ryat_gprs_is_attached(void) {
  ryat_cmd_t cmd = {__ms_to_tick(10000), "AT+CGATT?\r\n", RYAT_RET_OK | RYAT_RET_ERROR};

  cmd.cmdlen = strlen(cmd.cmd);
  if (RYAT_RET_OK == ryat_set(gp_at, &cmd)) {
    char *p = NULL;
    ryat_utils_args_t args;
    ryat_rx_line_t *prx = ryat_get_line_with_prefix(gp_at, "+CGATT");
    if (prx == NULL) {
      return RYAT_E_ERROR;
    }
    p = strstr(prx->buf, ":");
    ++p;
    if (RYAT_E_OK != ryat_utils_get_args(p, strlen(p), &args) || 
        ISDIGIT_RET_VALUE == isdigit(args.pval[0])) {
      return RYAT_E_ERROR;
    }
    return atoi(args.pval);
  }
  return RYAT_E_ERROR;
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
int32_t __attribute__((weak)) ryat_get_cellinfo(struct st_ryat_cell_describe *pcell) {
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

    prx = ryat_get_line_with_prefix(gp_at, "+CGREG");
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
  /* CLOSE CGREG unsolicited result code */
  cmd.cmd = "AT+CGREG=0\r\n";
  cmd.cmdlen = strlen(cmd.cmd);
  cmd.tick = __ms_to_tick(1000);
  cmd.ret = RYAT_RET_OK | RYAT_RET_ERROR;
  if(RYAT_RET_OK != ryat_set(gp_at, &cmd)) {
    return RYAT_E_ERROR;
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
 * @brief get module model type
 *
 * @retval type
 */
uint8_t ryat_if_get_module_model_type(void)
{
    return module_model_type;
}

/**
 * @brief set the at commander
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param pat
 *
 * @return err_t
 */
int32_t ryat_if_set_at(struct st_ryat_describe *pat) {
  int32_t retval = RYAT_E_ERROR;

  gp_at = pat;
  retval = ryat_init(pat);
  if(retval >= 0) {
        module_model_type = (uint8_t)retval;
        retval = RYAT_E_OK;
  }
  
  return retval;
}
