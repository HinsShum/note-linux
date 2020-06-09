/**
 * @file _sim800_at.h
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
#ifndef ___SIM800_AT_H__
#define ___SIM800_AT_H__

#ifdef __cpluscplus
extern "c"
{
#endif /* __cpluscplus */

#include "includes.h"
#include "ryat.h"

extern int32_t _sim800_at_get_data(struct st_ryat_describe* pat,uint8_t* pdata, uint16_t len, uint16_t* remain, uint32_t timeout);

#ifdef __cpluscplus
}
#endif /* __cpluscplus */
#endif /* ___SIM800_AT_H__ */



