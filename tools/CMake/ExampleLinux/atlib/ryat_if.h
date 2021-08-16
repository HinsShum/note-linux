/**
 * @file ryat_if.h
 *
 * @brief
 *
 *
 *
 * @copyright This file create by rensuiyi ,all right reserve!
 *
 * @author rensuiyi
 *
 * @date 2018/2/4 14:06:30
 */
#ifndef __RYAT_IF_H__
#define __RYAT_IF_H__

#ifdef __cpluscplus
extern "c" {
#endif /* __cpluscplus */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ryat.h"

#define __ms_to_tick(x)             ((x / 10))

#define RYAT_RADIO_ACCESS_MODE_2G   (0)
#define RYAT_RADIO_ACCESS_MODE_4G   (1)

extern int32_t __attribute__((weak)) ryat_test(void);
extern int32_t __attribute__((weak)) ryat_close_echo(void);
extern int32_t __attribute__((weak)) ryat_shutdown(void);
extern int32_t __attribute__((weak)) ryat_get_csq(uint8_t* csq);
extern int32_t __attribute__((weak)) ryat_gprs_attach(void);
extern int32_t __attribute__((weak)) ryat_gprs_is_attached(void);
extern int32_t __attribute__((weak)) ryat_get_cellinfo(struct st_ryat_cell_describe *pcell);
extern int32_t ryat_if_set_at(struct st_ryat_describe* pat);
extern uint8_t ryat_if_get_module_model_type(void);

#ifdef __cpluscplus
}
#endif /* __cpluscplus */
#endif /* __RYAT_IF_H__ */
