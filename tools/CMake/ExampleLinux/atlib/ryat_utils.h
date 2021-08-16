/**
 * @file ryat_utils.h
 *
 * @brief
 *
 *
 *
 * @copyright This file create by rensuiyi ,all right reserve!
 *
 * @author rensuiyi
 *
 * @date 2018/2/7 14:28:08
 */
#ifndef __RYAT_UTILS_H__
#define __RYAT_UTILS_H__

#ifdef __cpluscplus
extern "c"
{
#endif /* __cpluscplus */
#include "includes.h"

typedef struct {
  char* pval;
  char* pnext;
  uint16_t len;
}ryat_utils_args_t;


extern int32_t  ryat_utils_get_args(char* pbuf, uint16_t len, ryat_utils_args_t *args);
extern int32_t ryat_utils_hex_to_string(uint8_t *src, uint8_t *dst, uint16_t dst_max_len);
extern int32_t ryat_utils_string_to_hex(uint8_t *src, uint8_t *dst, uint16_t count);

#ifdef __cpluscplus
}
#endif /* __cpluscplus */
#endif /* __RYAT_UTILS_H__ */
