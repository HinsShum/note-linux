/**
 * @file _l710_at.h
 *
 * @brief
 *
 *
 *
 * @copyright This file create by rensuiyi ,all right reserve!
 *
 * @author rensuiyi
 *
 * @date 2018/2/8 14:11:40
 */
#ifndef ___L710_AT_H__
#define ___L710_AT_H__

#ifdef __cpluscplus
extern "c"
{
#endif /* __cpluscplus */

#include "includes.h"
#include "ryat.h"

extern err_t _l710_at_get_data(struct st_ryat_describe* pat,uint8_t ch,uint8_t* pdata, uint16_t len, uint32_t timeout);

#ifdef __cpluscplus
}
#endif /* __cpluscplus */
#endif /* ___L710_AT_H__ */



