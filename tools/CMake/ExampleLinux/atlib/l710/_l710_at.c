/**
 * @_l710_at.c
 *
 * @copyright This file create by rensuiyi ,all right reserve!
 *
 * @author rensuiyi
 *
 * @date 2018/2/4 13:07:32
 */
#include "_l710_at.h"
#include "includes.h"
#include "ryat.h"
#include "ryat_if.h"
#include "ryat_utils.h"

/* marcos */
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

struct st_ryat_ret_describe const ryat_ret_tbl_l710[] = {
  { RYAT_RET_OK, "OK", 2 },
  { RYAT_RET_ERROR, "ERROR", 5 },
  { RYAT_RET_CSQ, "+CSQ", 4 },
  { RYAT_RET_GMM, "+GMM", 4},
  { RYAT_RET_COPS, "+COPS", 5},
  { RYAT_RET_CEREG, "+CGREG", 6},
  { RYAT_RET_MIPCALL, "+MIPCALL: ", 10 },
  { RYAT_RET_MIPOPEN, "+MIPOPEN", 8 },
  { RYAT_RET_MIPSEND, "+MIPSEND:", 8 },
  { RYAT_RET_MIPCLOSE, "+MIPCLOSE", 8 },
  { RYAT_RET_MIPREAD, "+MIPREAD", 8 },
  { RYAT_RET_MIPDATA, "+MIPDATA", 8 },
  { RYAT_RET_AT_P, "AT+", 3 },
  { RYAT_RET_P, "+", 1 },
  { RYAT_RET_NULL, NULL, 0}
};

err_t _l710_at_get_data(struct st_ryat_describe* pat, uint8_t channel, uint8_t* pdata, uint16_t len, uint32_t timeout) {
  ryat_rx_line_t* pcurrent;
  int16_t tick = __ms_to_tick(timeout);
  int16_t reallen = 0;
  uint16_t cnt;
  ryat_utils_args_t args;
  char ch;
  bool_t process_ok = false;

  // reset the cache
  ryat_reset_cache(pat);

  pcurrent = &pat->cache.line[0];
  sprintf(pcurrent->buf, "AT+MIPREAD=%d,%d\r\n", channel, len);
  __ryat_debug("AT-W %d:%s", tick, pcurrent->buf);
  // write the command
  pat->port_discard_inpurt();
  pat->port_write_string(pcurrent->buf, strlen(pcurrent->buf));

  // read the data,get the responds line
  while (tick > 0) {
    if (pat->port_read_char(&ch)) {
      if ((ch == '\r') || (ch == '\n') || (ch == 0x00)) {
        // end of the line
        if (pcurrent->len != 0x00) {
          // add zero to the end
          pcurrent->buf[pcurrent->len] = 0x00;
          __ryat_debug("AT-line:%s\r\n", pcurrent->buf);
          // the line over begin a new line
          ryat_update_type(pat, pcurrent);
          __ryat_debug("AT-Type:%08X\r\n", pcurrent->type);
          // get the needed answer,return right now
          if (RYAT_RET_MIPDATA & pcurrent->type) {
            process_ok = true;
            break;
          }
          if (RYAT_RET_OK & pcurrent->type) {
            pat->delay_tick(tick);
            return 0;
          }
          if (RYAT_RET_ERROR & pcurrent->type) {
            pat->delay_tick(tick);
            return RYAT_E_ERROR;
          }
          pcurrent->len = 0;
        }
      } else {
        // fill the data to the line buf
        if (pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
          __ryat_debug("The line is too long\r\n");
          continue;
        }
        pcurrent->buf[pcurrent->len++] = ch;
      }
    } else {
      tick--;
      pat->delay_tick(1);
    }
  }

  if (!process_ok) {
    return RYAT_E_TIMEOUT;
  }

  if (pcurrent->len <= 12) {
    return RYAT_E_ERROR;
  }
  if (RYAT_E_OK != ryat_utils_get_args(pcurrent->buf + 12, pcurrent->len - 12, &args)) {
    return RYAT_E_ERROR;
  }
  reallen = atoi(args.pval);

  // copy the data
  tick = __ms_to_tick(100);
  cnt = 0;
  while ((tick > 0) && (reallen > 0)) {
    if (pat->port_read_char(&ch)) {
      // drop the first '\n'
      if ((cnt == 0) && (ch == '\n')) {
        tick--;
        continue;
      }
      pdata[cnt] = ch;
      cnt++;

      if (cnt == reallen) {
        break;
      }
    } else {
      tick--;
      pat->delay_tick(1);
    }
  }

  // check the last ok
  tick = __ms_to_tick(100);
  pcurrent = &pat->cache.line[0];
  pcurrent->len = 0;
  pcurrent->type = RYAT_RET_NULL;

  process_ok = false;
  while (tick > 0) {
    if (pat->port_read_char(&ch)) {
      if ((ch == '\r') || (ch == '\n') || (ch == 0x00)) {
        // end of the line
        if (pcurrent->len != 0x00) {
          // add zero to the end
          pcurrent->buf[pcurrent->len] = 0x00;
          __ryat_debug("AT-line:%s\r\n", pcurrent->buf);
          // the line over begin a new line
          ryat_update_type(pat, pcurrent);
          __ryat_debug("AT-Type:%08X\r\n", pcurrent->type);
          // get the needed answer,return right now
          if (RYAT_RET_OK & pcurrent->type) {
            process_ok = true;
            break;
          }
          pcurrent->len = 0;
        }
      } else {
        // fill the data to the line buf
        if (pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
          __ryat_debug("The line is too long\r\n");
          continue;
        }
        pcurrent->buf[pcurrent->len++] = ch;
      }
    } else {
      tick--;
      pat->delay_tick(1);
    }
  }

  if (process_ok) {
    return reallen;
  } else {
    return RYAT_E_ERROR;
  }
}
