/**
 * @ryat.c
 *
 * @copyright This file create by rensuiyi ,all right reserve!
 *
 * @author rensuiyi
 *
 * @date 2018/1/31 16:20:31
 */
#include "includes.h"
#include "ryat.h"


#define RYAT_DEBUG_ENABLE (1)
#if RYAT_DEBUG_ENABLE > 0
#define __ryat_debug(x,...)   printf(x,##__VA_ARGS__)
#else
#define __ryat_debug(x,...)
#endif /* RYAT_DEBUG_ENABLE */

static void ryat_update_type(struct st_ryat_describe *pat, ryat_rx_line_t *pline);

#include "l710/_l710_at.c"
#include "slm750/_slm750_at.c"
#include "g500/_g500_at.c"
#include "u9x07/_u9x07_at.c"
#include "bc26/_bc26_at.c"
#include "nbxx_01/_nbxx_01_at.c"
#include "sim800/_sim800_at.c"
#include "ec20/_ec20_at.c"
#include "slm790/_slm790_at.c"
#include "l620/_l620_at.c"

static struct st_ryat_ret_describe *ryat_ret_tbl[] = {
    [RYAT_MODULE_ID_L710] = (struct st_ryat_ret_describe *)ryat_ret_tbl_l710,
    [RYAT_MODULE_ID_SLM750] = (struct st_ryat_ret_describe *)ryat_ret_tbl_slm750,
    [RYAT_MODULE_ID_G500] = (struct st_ryat_ret_describe *)ryat_ret_tbl_g500,
    [RYAT_MODULE_ID_U9X07] = (struct st_ryat_ret_describe *)ryat_ret_tbl_u9x07,
    [RYAT_MODULE_ID_BC26] = (struct st_ryat_ret_describe *)ryat_ret_tbl_bc26,
    [RYAT_MODULE_ID_NBXX_01] = (struct st_ryat_ret_describe *)ryat_ret_tbl_nbxx_01,
    [RYAT_MODULE_ID_SIM800] = (struct st_ryat_ret_describe *)ryat_ret_tbl_sim800,
    [RYAT_MODULE_ID_EC20] = (struct st_ryat_ret_describe *)ryat_ret_tbl_ec20,
    [RYAT_MODULE_ID_SLM790] = (struct st_ryat_ret_describe *)ryat_ret_tbl_slm790,
    [RYAT_MODULE_ID_L620] = (struct st_ryat_ret_describe *)ryat_ret_tbl_l620,
};

extern struct st_ryat_interface ryat_interface_l710;
extern struct st_ryat_interface ryat_interface_slm750;
extern struct st_ryat_interface ryat_interface_g500;
extern struct st_ryat_interface ryat_interface_u9x07;
extern struct st_ryat_interface ryat_interface_bc26;
extern struct st_ryat_interface ryat_interface_nbxx_01;
extern struct st_ryat_interface ryat_interface_sim800;
extern struct st_ryat_interface ryat_interface_ec20;
extern struct st_ryat_interface ryat_interface_slm790;
extern struct st_ryat_interface ryat_interface_l620;
static struct st_ryat_interface *interface[] = {
    [RYAT_MODULE_ID_L710] = &ryat_interface_l710,
    [RYAT_MODULE_ID_SLM750] = (struct st_ryat_interface *)&ryat_interface_slm750,
    [RYAT_MODULE_ID_G500] = (struct st_ryat_interface *)&ryat_interface_g500,
    [RYAT_MODULE_ID_U9X07] = (struct st_ryat_interface *)&ryat_interface_u9x07,
    [RYAT_MODULE_ID_BC26] = (struct st_ryat_interface *)&ryat_interface_bc26,
    [RYAT_MODULE_ID_NBXX_01] = (struct st_ryat_interface *)&ryat_interface_nbxx_01,
    [RYAT_MODULE_ID_SIM800] = (struct st_ryat_interface *)&ryat_interface_sim800,
    [RYAT_MODULE_ID_EC20] = (struct st_ryat_interface *)&ryat_interface_ec20,
    [RYAT_MODULE_ID_SLM790] = (struct st_ryat_interface *)&ryat_interface_slm790,
    [RYAT_MODULE_ID_L620] = (struct st_ryat_interface *)&ryat_interface_l620,
};
#define RYAT_INTERFACE_NUMBER   (sizeof(interface) / sizeof(interface[0]))

char *ryat_module_model_name[] = {
    [RYAT_MODULE_ID_L710] = "L710",
    [RYAT_MODULE_ID_SLM750] = "SLM750",
    [RYAT_MODULE_ID_G500] = NULL,
    [RYAT_MODULE_ID_U9X07] = NULL,
    [RYAT_MODULE_ID_BC26] = NULL,
    [RYAT_MODULE_ID_NBXX_01] = NULL,
    [RYAT_MODULE_ID_SIM800] = NULL,
    [RYAT_MODULE_ID_EC20] = "EC20",
    [RYAT_MODULE_ID_SLM790] = "SLM790",
    [RYAT_MODULE_ID_L620] = "L620",
};
#define RYAT_MODEL_NAME_NUMBER  (sizeof(ryat_module_model_name) / sizeof(ryat_module_model_name[0]))

void ryat_reset_cache(struct st_ryat_describe* pat) {
  uint16_t i = 0;

  //reset the cache
  for (i = 0; i < RYAT_LINE_MAX; i++) {
    pat->cache.line[i].buf[0] = 0x00;
    pat->cache.line[i].len = 0x00;
    pat->cache.line[i].type = RYAT_RET_NULL;
  }

  //reset the point off the cache
  pat->cache.rxhead = pat->cache.rxtail = 0x00;
}

static void ryat_update_type(struct st_ryat_describe *pat, ryat_rx_line_t *pline)
{
  for(struct st_ryat_ret_describe *pret_tbl = pat->pryat_ret_tbl; pret_tbl->prefix != NULL; ++pret_tbl) {
    if (!strncmp(pline->buf, pret_tbl->prefix, pret_tbl->prefixlen)) {
      //match
      pline->type = pret_tbl->id;
      return;
    }
  }
  pline->type = RYAT_RET_UNKNOWN;
}

/**
 * @brief
 *
 *
 * @author rensuiyi (2018/1/31)
 *
 * @param pat
 * @param pcmd
 *
 * @return ryat_res_t
 */
ryat_res_t ryat_set(struct st_ryat_describe* pat, ryat_cmd_t* pcmd) {
  ryat_rx_line_t* pcurrent;
  char ch;
  __ryat_debug("AT-W %d:%s", pcmd->tick, pcmd->cmd);
  //discard input buffer
  pat->port_discard_inpurt();
  //write the data to the modem
  pat->port_write_string(pcmd->cmd, pcmd->cmdlen);

  /*
   * read the data from the modem and check the result
   */
  //reset the cache
  ryat_reset_cache(pat);
  pcurrent = &pat->cache.line[0];
  while (pcmd->tick > 0) {
    //read char and add to the buffer
    if (pat->port_read_char(&ch)) {
      if ((ch == '\r') || (ch == '\n') || (ch == 0x00)) {
        //end of the line
        if (pcurrent->len != 0x00) {
          // add zero to the end
          pcurrent->buf[pcurrent->len] = 0x00;
          __ryat_debug("AT-line:%s\r\n", pcurrent->buf);
          //the line over begin a new line
          ryat_update_type(pat, pcurrent);
          __ryat_debug("AT-Type:%08X\r\n", pcurrent->type);
          pat->cache.rxtail++;
          //get the needed answer,return right now
          if (pcmd->ret & pcurrent->type) {
            return (ryat_res_t)pcurrent->type;
          }
          //else cache the middle data
          if (pat->cache.rxtail >= RYAT_LINE_MAX) {
            __ryat_debug("Too more lines to read\r\n");
            //drop the last buffer in order to read the right answer
            pat->cache.rxtail = RYAT_LINE_MAX;
            pcurrent->type = RYAT_RET_NULL;
            pcurrent->len = 0x00;
          } else {
            pcurrent++;
          }
        }
      } else {
        //fill the data to the line buf
        if (pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
          __ryat_debug("The line is too long\r\n");
          continue;
        }
        pcurrent->buf[pcurrent->len++] = ch;
      }
    } else {
      pcmd->tick--;
      pat->delay_tick(1);
    }
  }
  return RYAT_RET_NULL;
}


/**
 * @brief
 *
 *
 * @author rensuiyi (2018/1/31)
 *
 * @param pat
 *
 * @return ryat_rx_line_t*
 */
ryat_rx_line_t* ryat_get_line(struct st_ryat_describe* pat) {
  return NULL;
}


/**
 * @brief
 *
 *
 * @author rensuiyi (2018/1/31)
 *
 * @param pat
 * @param prefix
 *
 * @return ryat_rx_line_t*
 */
ryat_rx_line_t* ryat_get_line_with_prefix(struct st_ryat_describe* pat, char* prefix) {
  uint16_t i = 0;
  uint16_t len = strlen(prefix);
  for (i = pat->cache.rxhead; i < pat->cache.rxtail; i++) {
    if (!strncmp(prefix, pat->cache.line[i].buf,len)) {
      pat->cache.rxhead = i + 1;
      return &pat->cache.line[i];
    }
  }
  return NULL;
}


/**
 *
 *
 */
ryat_rx_line_t* ryat_get_like_line(struct st_ryat_describe* pat, char* prefix) {
  return NULL;
}


/**
 * @brief
 *
 *
 * @author rensuiyi (2018/2/7)
 *
 * @param pat
 * @param t
 *
 * @return ryat_rx_line_t*
 */
ryat_rx_line_t* ryat_get_line_with_type(struct st_ryat_describe* pat, ryat_res_t t) {
  uint16_t i = 0;

  for (i = pat->cache.rxhead; i < pat->cache.rxtail; i++) {
    if (pat->cache.line[i].type == t) {
      pat->cache.rxhead = i+1;
      return &pat->cache.line[i];
    }
  }
  return NULL;
}

/**
 *
 *
 * @author rensuiyi (2018/2/4)
 *
 * @param pat
 *
 * @return err_t
 */
int32_t ryat_init(struct st_ryat_describe* pat) {
    char model_name[15] = {0};
    int32_t retval = RYAT_E_ERROR;

    ryat_reset_cache(pat);
    for(uint8_t i = 0; i < RYAT_INTERFACE_NUMBER; ++i) {
        if(ryat_ret_tbl[i] != NULL && interface[i] != NULL && ryat_module_model_name[i] != NULL) {
            pat->pryat_ret_tbl = ryat_ret_tbl[i];
            pat->method = interface[i];
            /* test and close echo */
            for(uint8_t j = 0; j < 3; ++j) {
                if(RYAT_E_OK == pat->method->test() && RYAT_E_OK == pat->method->close_echo()) {
                    break;
                }
            }
            if(RYAT_E_OK == (retval = pat->method->get_model(model_name, sizeof(model_name)))) {
                // judge the model name
                if(ryat_module_model_name[i] != NULL && strstr(model_name, ryat_module_model_name[i]) != NULL) {
                    retval = i;
                    break;
                }
                retval = RYAT_E_ERROR;
            }
        }
    }

    return retval;
}
