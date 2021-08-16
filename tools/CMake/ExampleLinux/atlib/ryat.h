/**
 * @file ryat.h
 *
 * @brief
 *      at engine should have 64*10 = 640K cache,and 640K fifo
 *      to receive the longest respond from the device
 *
 *
 * @copyright This file create by rensuiyi ,all right reserve!
 *
 * @author rensuiyi
 *
 * @date 2018/1/31 16:20:42
 */
#ifndef __RY_AT_H__
#define __RY_AT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "includes.h"
#include <limits.h>

/* define module ID */
#define RYAT_MODULE_ID_L710     (0)
#define RYAT_MODULE_ID_SLM750   (1)
#define RYAT_MODULE_ID_G500     (2)
#define RYAT_MODULE_ID_U9X07    (3)
#define RYAT_MODULE_ID_BC26     (4)
#define RYAT_MODULE_ID_NBXX_01  (5)
#define RYAT_MODULE_ID_SIM800   (6)
#define RYAT_MODULE_ID_EC20     (7)
#define RYAT_MODULE_ID_SLM790   (8)
#define RYAT_MODULE_ID_L620     (9)

/*
 * The result type of the AT COMMNAD
 *
 */
typedef enum {
  /* general commands */
  RYAT_RET_NULL         = (1 << 0),
  RYAT_RET_OK           = (1 << 1),
  RYAT_RET_CSQ          = (1 << 2),
  RYAT_RET_ERROR        = (1 << 3),
  RYAT_RET_AT_P         = (1 << 4),
  RYAT_RET_CME          = (1 << 5),
  RYAT_RET_CEREG        = (1 << 6),
  RYAT_RET_COPS         = (1 << 7),
  RYAT_RET_GMM          = (1 << 8),
  RYAT_RET_CCID         = (1 << 9),
  RYAT_RET_CGSN         = (1 << 10),
  /* private commands for module */
  RYAT_RET_MIPCALL      = (1 << 12),
  RYAT_RET_IP           = (1 << 12),
  RYAT_RET_SHUTOK       = (1 << 12),
  RYAT_RET_MIPOPEN      = (1 << 13),
  RYAT_RET_ALREADY_CON  = (1 << 13),
  RYAT_RET_CTM2M_REG    = (1 << 13),
  RYAT_RET_MIPCLOSE     = (1 << 14),
  RYAT_RET_CLOSE_OK     = (1 << 14),
  RYAT_RET_MIPSEND      = (1 << 15),
  RYAT_RET_SENDOK       = (1 << 15),
  RYAT_RET_SEND_OK      = (1 << 15),
  RYAT_RET_MIPREAD      = (1 << 16),
  RYAT_RET_QIRD         = (1 << 16),
  RYAT_RET_NNMI         = (1 << 16),
  RYAT_RET_GET_DATA     = (1 << 16),
  RYAT_RET_MIPDATA      = (1 << 17),
  RYAT_RET_MIPRUDP      = (1 << 17),
  RYAT_RET_SEND_FAILD   = (1 << 17),
  RYAT_RET_CELL_INFO    = (1 << 18),
  RYAT_RET_CON_OK       = (1 << 18),
  RYAT_RET_MIP          = (1 << 19),
  RYAT_RET_CON_FAIL     = (1 << 19),
  RYAT_RET_CTM2M_ERROR  = (1 << 20),
  /* general commands */
  RYAT_RET_P            = (1 << 25),
  RYAT_RET_UNKNOWN      = (1 << 30),
  RYAT_RET_ALL          = (INT_MAX),
} ryat_res_t;

/*
 * AT CMD
 */
typedef struct {
  uint32_t tick;
  const char* cmd;
  uint32_t ret;
  uint16_t cmdlen;
} ryat_cmd_t;

#define RYAT_COMMAND_LINE_LENGTH (64)
#define RYAT_LINE_MAX (10)

// ERROR define
#define RYAT_E_OK          (0)
#define RYAT_E_ERROR       (-1)
#define RYAT_E_BUSY        (-2)
#define RYAT_E_TIMEOUT     (-3)
#define RYAT_E_WRONG_ARGS  (-4)


typedef struct {
  uint32_t type;
  int16_t len;
  char buf[RYAT_COMMAND_LINE_LENGTH];
} ryat_rx_line_t;

typedef struct {
  ryat_rx_line_t line[RYAT_LINE_MAX];
  int16_t rxhead;
  int16_t rxtail;
} ryat_cache_t;

/*
 * AT struct
 *
 */
struct st_ryat_ret_describe {
  ryat_res_t id;
  const char* prefix;
  uint16_t prefixlen;
};

struct st_ryat_cell_describe {
  uint16_t mcc;
  uint8_t mnc;
  uint8_t rf;
  uint32_t cellid;
  uint16_t lac;
};

/* at operate functions */
struct st_ryat_interface {
  int32_t (*test)(void);
  int32_t (*close_echo)(void);
  int32_t (*shutdown)(void);
  int32_t (*get_csq)(uint8_t *csq);
  int32_t (*get_rxlev)(uint8_t *rxlev);
  int32_t (*get_imei)(uint8_t *pbuf);
  int32_t (*get_iccid)(uint8_t *pbuf);
  int32_t (*get_imsi)(uint8_t *pbuf);
  int32_t (*attach)(void);
  int32_t (*is_attached)(void);
  int32_t (*get_apn)(char *apn, uint16_t buflen);
  int32_t (*set_apn)(char *apn, char *name, char *passwd);
  int32_t (*active)(uint8_t *pbuf, uint16_t buflen);
  int32_t (*deactivate)(void);
  int32_t (*socket_init)(bool iscached);
  int32_t (*socket_udp)(char *domain_ip, uint16_t port, uint8_t ch);
  int32_t (*socket_tcp)(char *domain_ip, uint16_t port, uint8_t ch);
  int32_t (*socket_shut)(uint8_t ch);
  int32_t (*write_data)(uint8_t *pdata, uint16_t len, uint8_t ch);
  int32_t (*read_cache)(uint8_t ch, uint8_t *pdata, uint16_t len, uint16_t *remain, uint32_t timeout);
  int32_t (*get_creg_cell)(struct st_ryat_cell_describe *pcell);
  int32_t (*select_radio_access)(uint8_t mode);
  int32_t (*get_radio_access)(uint8_t *mode);
  int32_t (*get_model)(char *buf, uint16_t len);
  void *reserve[6];
};

struct st_ryat_describe {
  ryat_cache_t cache;
  int16_t (*port_write_string)(const char* pdata, int16_t len);
  bool_t (*port_read_char)(char* pchar);
  void (*port_discard_inpurt)(void);
  void (*delay_tick)(int16_t tick);
  /* at operate functions */
  struct st_ryat_ret_describe *pryat_ret_tbl;
  struct st_ryat_interface *method;
};

extern char *ryat_module_model_name[];
extern void ryat_reset_cache(struct st_ryat_describe* pat);
extern ryat_res_t ryat_set(struct st_ryat_describe* pat, ryat_cmd_t* pcmd);
extern ryat_rx_line_t* ryat_get_line(struct st_ryat_describe* pat);
extern ryat_rx_line_t* ryat_get_line_with_type(struct st_ryat_describe* pat, ryat_res_t t);
extern ryat_rx_line_t* ryat_get_line_with_prefix(struct st_ryat_describe* pat, char* prefix);
extern int32_t ryat_init(struct st_ryat_describe* pat);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ry_at_h__ */
