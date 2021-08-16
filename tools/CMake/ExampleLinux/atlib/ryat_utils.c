/**
 * @ryat_utils.c
 *
 * @copyright This file create by rensuiyi ,all right reserve!
 *
 * @author rensuiyi
 *
 * @date 2018/2/7 14:27:12
 */
#include "ryat_utils.h"
#include "ryat.h"
#include <ctype.h>

/**
 * @brief
 *
 *
 * @author rensuiyi (2018/2/8)
 *
 * @param pbuf
 * @param len
 *
 * @return ryat_utils_args_t*
 */
int32_t ryat_utils_get_args(char* pbuf, uint16_t len, ryat_utils_args_t* args) {
  uint16_t i = 0;
  if (pbuf == NULL) {
    return RYAT_E_ERROR;
  }

  memset(args, 0, sizeof(ryat_utils_args_t));
  for (i = 0; i < len; i++) {
    if (NULL == args->pval) {
      if ((pbuf[i] != '"') && (pbuf[i] != ' ') && (pbuf[i] != '\0') && (pbuf[i] != ',') && (pbuf[i] != ':')) {
        args->pval = &pbuf[i];
      }
    } else {
      if ((pbuf[i] == '"') || (pbuf[i] == ' ') || (pbuf[i] == '\0') || (pbuf[i] == ',') || (pbuf[i] == ':')) {
        pbuf[i] = 0x00;
        if (i < len - 1) {
          args->pnext = &(pbuf[i]) + 1;
        }
        break;
      }
    }
  }
  if (args->pval != NULL) {
    args->len = strlen(args->pval);
    return RYAT_E_OK;
  }

  return RYAT_E_ERROR;
}

/**
 * @brief convert hex to hex string
 * @param src: hex address
 * @param dst: hex string start address
 * @param dst_max_len: hex string buffer max length
 *
 * @retval the actual convert hex length
 */
int32_t ryat_utils_hex_to_string(uint8_t *src, uint8_t *dst, uint16_t dst_max_len)
{
    uint16_t src_len = strlen((char *)src);

    if((src_len * 2) > dst_max_len) {
        return RYAT_E_ERROR;
    }
    for(uint16_t i = 0; i < src_len; ++i) {
        char *p = (char *)dst + (i * 2);
        char *c = (char *)src + i;
        sprintf(p, "%02X", *c);
    }
    return src_len;
}

/**
 * @brief convert hex string to hex
 * @param src: hex string address
 * @param dst: hex address
 * @param count: the count of the string
 *
 * @retval the actual convert hex length
 */
int32_t ryat_utils_string_to_hex(uint8_t *src, uint8_t *dst, uint16_t count)
{
    uint16_t i, cnt;

    for(i = 0; i < count; ++i) {
        cnt = 0;
        do {
            uint8_t c = src[(i << 1) + cnt];
            if(!isdigit(c)) {
                c = toupper(c);
                c = (c - 'A') + 10;
            } else {
                c -= '0';
            }
            c &= 0xF;
            dst[i] |= (c << (4 * (1 - cnt)));
        } while(!cnt++);
    }

    return i;
}
