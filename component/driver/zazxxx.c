/**
 * @file driver\zazxxx.c
 *
 * Copyright (C) 2021
 *
 * zazxxx.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author HinsShum hinsshum@qq.com
 *
 * @encoding utf-8
 */

/*---------- includes ----------*/
#include "zazxxx.h"
#include "driver.h"
#include "checksum.h"
#include "config/errorno.h"
#include "config/options.h"
#include <string.h>

/*---------- macro ----------*/
/* start code define
 */
#define _START_CODE_COMMAND_PACKET                      (0xAA55)
#define _START_CODE_RESPONSE_COMMAND_PACKET             (0x55AA)
#define _START_CODE_DATA_PACKET                         (0xA55A)
#define _START_CODE_RESPONSE_DATA_PACKET                (0x5AA5)

#define _COMMAND_PACKET_CHECKSUM_LENGTH                 (2)
#define _COMMAND_PACKET_MAX_LENGTH(buf)                 (ARRAY_SIZE(buf) - _COMMAND_PACKET_CHECKSUM_LENGTH)

/* command packet make macros
 */
#define _CMD_PACKET_MAKE(cmd, buf, len)                 _command_packet_make(pdesc, cmd, buf, len)
#define _CMD_DATA_PACKET_MAKE(cmd, buf, len)            _command_data_packet_make(pdesc, cmd, buf, len)
#define _CMD_DATA_TEMP_PACKET_MAKE(cmd, buf, len, blk)  _command_data_template_packet_make(pdesc, cmd, buf, len, blk)

#define _IS_A_PART_OF_PREFIX(byte)                      ((byte == 0xAA) || (byte == 0x55) || (byte == 0xA5) || (byte == 0x5A))

#define _TEMPLATE_MAX_SIZE                              (1024)
#define _TEMPLATE_ID_MIN                                (1)
#define _TEMPLATE_ID_MAX                                (1000)
#define _TEMPLATE_RAM_BUFFER_ID                         (0)
#define _GENERATE_RAM_BUFFER_ID                         (1)
#define _DEFAULT_TIMEOUT_MS                             (500)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t zazxxx_open(driver_t **pdrv);
static void zazxxx_close(driver_t **pdrv);
static int32_t zazxxx_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t zazxxx_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len);

/* private function prototype
 */
static int32_t _ioctl_get_comport(zazxxx_describe_t *pdesc, void *agrs);
static int32_t _ioctl_set_irq_handler(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_direction_choose(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_get_baudrate(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_set_baudrate(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_connect(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_get_image(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_enroll_by_template(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_get_enroll_counts(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_delete_one_template(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_delete_all_template(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_generate_template(zazxxx_describe_t *pdesc, void *args);
static int32_t _ioctl_search_template(zazxxx_describe_t *pdesc, void *args);

/*---------- type define ----------*/
typedef enum {
    STATE_RX_IDLE,
    STATE_RX_RECV_FIXED_PART,
    STATE_RX_RECV_DATA_PART,
    STATE_RX_WAIT_EOF
} _recv_state;

typedef enum {
    CMD_CONNECTION = 0x0001,
    CMD_SET_PARAM = 0x0002,
    CMD_GET_PARAM = 0x0003,
    CMD_GET_DEVICE_INFO = 0x0004,
    CMD_ENTER_IAP_MODE = 0x0005,
    CMD_SET_MODULE_SN = 0x0008,
    CMD_GET_MODULE_SN = 0x0009,
    CMD_ENETR_STANDY_STATE = 0x000C,
    CMD_GET_IMAGE = 0x0020,
    CMD_FINGER_DETECT = 0x0021,
    CMD_UP_IMAGE = 0x0022,
    CMD_DOWN_IMAGE = 0x0023,
    CMD_SLED_CTRL = 0x0024,
    CMD_STORE_CHAR = 0x0040,
    CMD_LOAD_CHAR = 0x0041,
    CMD_UP_CHAR = 0x0042,
    CMD_DOWN_CHAR = 0x0043,
    CMD_DEL_CHAR = 0x0044,
    CMD_GET_EMPTY_ID = 0x0045,
    CMD_GET_STATUS = 0x0046,
    CMD_GET_BROKEN_ID = 0x0047,
    CMD_GET_ENROLL_COUNT = 0x0048,
    CMD_GET_ENROLLED_ID_LIST = 0x0049,
    CMD_GENERATE = 0x0060,
    CMD_MERGE = 0x0061,
    CMD_MATCH = 0x0062,
    CMD_SEARCH = 0x0063,
    CMD_VERIFY = 0x0064
} _command_en;

typedef enum {
    ERR_SUCCESS = 0x00,
    ERR_FAILE = 0x01,
    ERR_VERIFY = 0x10,
    ERR_IDENTIFY = 0x11,
    ERR_TMPL_EMPTY = 0x12,
    ERR_TMPL_NOT_EMPTY = 0x13,
    ERR_ALL_TMPL_EMPTY = 0x14,
    ERR_EMPTY_ID_NOEXITS = 0x15,
    ERR_BROKEN_ID_NOEXTES = 0x16,
    ERR_INVALID_TMPL_DATA = 0x17,
    ERR_DUPLICATION_ID = 0x18,
    ERR_BAD_QUALITY = 0x19,
    ERR_MERGE_FAIL = 0x1A,
    ERR_MEMORY = 0x1C,
    ERR_INVALID_TMPL_NO = 0x1D,
    ERR_INVALID_PARAM = 0x22,
    ERR_GEN_COUNT = 0x25,
    ERR_INVALID_BUFFER_ID = 0x26,
    ERR_FP_NOT_DETECTED = 0x28,
    ERR_FP_SMALL_AREA = 0x30,
    ERR_FP_WET_FINGER = 0x31,
    ERR_FP_DRY_FINGER = 0x32
} _result_code_en;

typedef enum {
    PARA_TYPE_DEVICE_ID = 0,
    PARA_TYPE_DUP_FP_CHECK,
    PARA_TYPE_BAUDRATE,
    PARA_TYPE_AUTOMATIC_LEARN,
    PARA_TYPE_DUP_ID_CHECK,
    PARA_TYPE_GENSCORE,
    PARA_TYPE_MATSCORE,
    PARA_TYPE_FP_CHECCORE,
    PARA_TYPE_STDMODE
} zazxxx_parameter_type_en;

typedef struct {
    uint16_t prefix;
    uint8_t src;
    uint8_t dst;
    uint16_t cmd;
    uint16_t length;
    uint8_t data[0];
} _command_packet_t;

typedef struct {
    uint16_t prefix;
    uint8_t src;
    uint8_t dst;
    uint16_t rcm;
    uint16_t length;
    uint16_t ret;
    uint8_t data[0];
} _command_response_packet_t;

typedef int32_t (*ioctl_cb_func_t)(zazxxx_describe_t *pdesc, void *args);

typedef struct {
    uint32_t ioctl_cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable ----------*/
DRIVER_DEFINED(zazxxx, zazxxx_open, zazxxx_close, NULL, NULL, zazxxx_ioctl, zazxxx_irq_handler);

/* ioctl callback function array
 */
ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_ZAZXXX_GET_COMPORT, _ioctl_get_comport},
    {IOCTL_ZAZXXX_SET_IRQ_HANDLER, _ioctl_set_irq_handler},
    {IOCTL_ZAZXXX_DIRECTION_CHOOSE, _ioctl_direction_choose},
    {IOCTL_ZAZXXX_GET_BAUDRATE, _ioctl_get_baudrate},
    {IOCTL_ZAZXXX_SET_BAUDRATE, _ioctl_set_baudrate},
    {IOCTL_ZAZXXX_CONNECT, _ioctl_connect},
    {IOCTL_ZAZXXX_DELETE_ALL_TEMPLATE, _ioctl_delete_all_template},
    {IOCTL_ZAZXXX_DELETE_ONE_TEMPLATE, _ioctl_delete_one_template},
    {IOCTL_ZAZXXX_ENROLL_BY_TEMPLATE, _ioctl_enroll_by_template},
    {IOCTL_ZAZXXX_GET_ENROLL_NUMBERS, _ioctl_get_enroll_counts},
    {IOCTL_ZAZXXX_GET_IMAGE, _ioctl_get_image},
    {IOCTL_ZAZXXX_GENERATE_TEMPLATE, _ioctl_generate_template},
    {IOCTL_ZAZXXX_SEARCH_TEMPLATE, _ioctl_search_template}
};

/*---------- function ----------*/
static void _command_packet_clear(zazxxx_describe_t *pdesc)
{
    pdesc->_private.offset = 0;
    pdesc->_private.state = RECV_STATE_IDLE;
}

static void _command_packet_calc_checksum(uint8_t *pbuf, uint16_t *plength)
{
    uint16_t checksum = checksum_sum16(pbuf, *plength);

    memcpy(&pbuf[*plength], &checksum, sizeof(checksum));
    *plength += sizeof(checksum);
}

static int32_t _command_packet_make(zazxxx_describe_t *pdesc, uint16_t command, 
                                    uint8_t *pdata, uint16_t length)
{
    _command_packet_t *pcmd = (_command_packet_t *)pdesc->_private.buffer;
    int32_t retval = CY_E_NO_MEMORY;

    do {
        _command_packet_clear(pdesc);
        pdesc->_private.offset = offsetof(_command_packet_t, data) + 16;
        if(pdesc->_private.offset > _COMMAND_PACKET_MAX_LENGTH(pdesc->_private.buffer)) {
            __debug_warn("No memory to store data for make zazxxx command packet\n");
            break;
        }
        pcmd->prefix = _START_CODE_COMMAND_PACKET;
        pcmd->src = 0;
        pcmd->dst = 0;
        pcmd->cmd = command;
        pcmd->length = length;
        memcpy(pcmd->data, pdata, length);
        _command_packet_calc_checksum(pdesc->_private.buffer, &pdesc->_private.offset);
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _command_data_packet_make(zazxxx_describe_t *pdesc, uint16_t command, 
                                         uint8_t *pdata, uint16_t length)
{
    _command_packet_t *pcmd = (_command_packet_t *)pdesc->_private.buffer;
    int32_t retval = CY_E_NO_MEMORY;

    do {
        _command_packet_clear(pdesc);
        pdesc->_private.offset = offsetof(_command_packet_t, data) + length;
        if(pdesc->_private.offset > _COMMAND_PACKET_MAX_LENGTH(pdesc->_private.buffer)) {
            __debug_warn("No memory to store data for make zazxxx command data packet\n");
            break;
        }
        pcmd->prefix = _START_CODE_DATA_PACKET;
        pcmd->src = 0;
        pcmd->dst = 0;
        pcmd->cmd = command;
        pcmd->length = length;
        memcpy(pcmd->data, pdata, length);
        _command_packet_calc_checksum(pdesc->_private.buffer, &pdesc->_private.offset);
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _command_data_template_packet_make(zazxxx_describe_t *pdesc, uint16_t command, uint8_t *pdata,
                                                  uint16_t length, uint16_t block_number)
{
    _command_packet_t *pcmd = (_command_packet_t *)pdesc->_private.buffer;
    int32_t retval = CY_E_NO_MEMORY;

    do {
        _command_packet_clear(pdesc);
        pdesc->_private.offset = offsetof(_command_packet_t, data) + 514;
        if(pdesc->_private.offset > _COMMAND_PACKET_MAX_LENGTH(pdesc->_private.buffer)) {
            __debug_warn("No memory to store template data for make zazxxx command data packet\n");
            break;
        }
        if(length > 512) {
            __debug_warn("Template data packet length can not over than 512 bytes\n");
            break;
        }
        pcmd->prefix = _START_CODE_DATA_PACKET;
        pcmd->src = 0;
        pcmd->dst = 0;
        pcmd->cmd = command;
        pcmd->length = 514;
        memset(pcmd->data, 0, pcmd->length);
        memcpy(pcmd->data, &block_number, sizeof(block_number));
        if(pdata) {
            memcpy(&pcmd->data[sizeof(block_number)], pdata, length);
        }
        _command_packet_calc_checksum(pdesc->_private.buffer, &pdesc->_private.offset);
        retval = CY_EOK;
    } while(0);

    return retval;
}

static void _command_packet_send(zazxxx_describe_t *pdesc)
{
    if(pdesc->serial.dir_change) {
        pdesc->serial.dir_change(SERIAL_DIRECTION_TX);
    }
    pdesc->serial.write(pdesc->_private.buffer, pdesc->_private.offset);
    if(pdesc->serial.dir_change) {
        pdesc->serial.dir_change(SERIAL_DIRECTION_RX);
    }
    _command_packet_clear(pdesc);
}

static int32_t _command_send(zazxxx_describe_t *pdesc, _command_en cmd, uint8_t *pdata, uint16_t length, int32_t timeout_ms)
{
    int32_t retval = CY_E_NO_MEMORY;
    _command_response_packet_t *presp = NULL;

    do {
        if(CY_EOK != (retval = _CMD_PACKET_MAKE(cmd, pdata, length))) {
            break;
        }
        _command_packet_send(pdesc);
        retval = CY_E_TIME_OUT;
        do {
            if(pdesc->_private.state == RECV_STATE_BUSY) {
                retval = CY_EOK;
                break;
            }
            __delay_ms(1);
        } while(timeout_ms-- > 0);
        if(retval == CY_E_TIME_OUT) {
            __debug_warn("Get command(%04X) response timeout\n", cmd);
            break;
        }
        presp = (_command_response_packet_t *)pdesc->_private.buffer;
        if(presp->rcm != cmd) {
            retval = CY_ERROR;
            __debug_error("Resp RCM(%04X) is not equal to command(%04X)\n", presp->rcm, cmd);
            break;
        }
        retval = (presp->ret ? CY_ERROR : CY_EOK);
        __debug_message("Send command(%04X) result code:%04X\n", cmd, presp->ret);
    } while(0);

    return retval;
}

static int32_t _command_template_send(zazxxx_describe_t *pdesc, _command_en cmd, uint8_t *pdata, 
                                      uint16_t length, uint8_t block, int32_t timeout_ms)
{
    int32_t retval = CY_E_NO_MEMORY;
    _command_response_packet_t *presp = NULL;

    do {
        if(CY_EOK != (retval = _CMD_DATA_TEMP_PACKET_MAKE(cmd, pdata, length, block))) {
            break;
        }
        _command_packet_send(pdesc);
        retval = CY_E_TIME_OUT;
        do {
            if(pdesc->_private.state == RECV_STATE_BUSY) {
                retval = CY_EOK;
                break;
            }
            __delay_ms(1);
        } while(timeout_ms-- > 0);
        if(retval == CY_E_TIME_OUT) {
            __debug_warn("Get command(%04X) response timeout\n", cmd);
            break;
        }
        presp = (_command_response_packet_t *)pdesc->_private.buffer;
        if(presp->rcm != cmd) {
            retval = CY_ERROR;
            __debug_error("Resp RCM(%04X) is not equal to command(%04X)\n", presp->rcm, cmd);
            break;
        }
        retval = (presp->ret ? CY_ERROR : CY_EOK);
        __debug_message("Send command(%04X) result code:%04X\n", cmd, presp->ret);
    } while(0);

    return retval;
}

static int32_t zazxxx_open(driver_t **pdrv)
{
    zazxxx_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->serial.init) {
        retval = (pdesc->serial.init() ? CY_EOK : CY_ERROR);
    }

    return retval;
}

static void zazxxx_close(driver_t **pdrv)
{
    zazxxx_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->serial.deinit) {
        pdesc->serial.deinit();
    }
}

static ioctl_cb_func_t _ioctl_cb_func_find(uint32_t cmd)
{
    ioctl_cb_func_t cb = NULL;

    for(uint32_t i = 0; i < ARRAY_SIZE(ioctl_cb_array); ++i) {
        if(ioctl_cb_array[i].ioctl_cmd == cmd) {
            cb = ioctl_cb_array[i].cb;
            break;
        }
    }

    return cb;
}

static int32_t _ioctl_get_comport(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint8_t *pcomport = (uint8_t *)args;

    do {
        if(!args) {
            __debug_warn("Args is NULL, can not store zazxxx driver's comport\n");
            break;
        }
        *pcomport = pdesc->serial.comport;
        retval = CY_EOK;
    } while(0);
    
    return retval;
}

static int32_t _ioctl_set_irq_handler(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;

    do {
        if(!args) {
            __debug_warn("Args is NULL, can not set zazxxx driver's irq handler\n");
            break;
        }
        pdesc->serial.irq_handler = (int32_t (*)(uint32_t, void *, uint32_t))args;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_direction_choose(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    serial_direction_en *pdir = (serial_direction_en *)args;

    do {
        if(!args) {
            __debug_warn("Args is NULL, can not choose direction in zazxxx driver\n");
            break;
        }
        if(!pdesc->serial.dir_change) {
            __debug_warn("zazxxx driver not support choose direction\n");
            break;
        }
        if(*pdir > SERIAL_DIRECTION_NRX_NTX) {
            __debug_warn("Args(%02X) for choose direction is error\n", *pdir);
            break;
        }
        pdesc->serial.dir_change(*pdir);
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_get_baudrate(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *pbaudrate = (uint32_t *)args;

    do {
        if(!args) {
            __debug_warn("Args is NULL, can not get baudrate in zazxxx driver\n");
            break;
        }
        *pbaudrate = pdesc->serial.baudrate;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _ioctl_set_baudrate(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *pbaudrate = (uint32_t *)args;

    do {
        if(!args) {
            __debug_warn("Args is NULL, can not set baudrate in zazxxx driver\n");
            break;
        }
        retval = CY_EOK;
        if(pdesc->serial.baudrate == *pbaudrate) {
            __debug_message("Baudrate not changed");
            break;
        }
        pdesc->serial.baudrate = *pbaudrate;
        if(pdesc->serial.deinit) {
            pdesc->serial.deinit();
        }
        if(pdesc->serial.init) {
            retval = (pdesc->serial.init() ? CY_EOK : CY_ERROR);
        }
    } while(0);

    return retval;
}

static int32_t _ioctl_connect(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_EOK;
    int32_t timeout_ms = _DEFAULT_TIMEOUT_MS;
    _command_response_packet_t *presp = NULL;

    do {
        if(args) {
            timeout_ms = *(int32_t *)args;
        }
        retval = _command_send(pdesc, CMD_CONNECTION, NULL, 0, timeout_ms);
    } while(0);

    return retval;
}

static int32_t _ioctl_get_image(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_EOK;
    int32_t timeout_ms = _DEFAULT_TIMEOUT_MS;
    _command_response_packet_t *presp = NULL;
    
    do {
        if(args) {
            timeout_ms = *(int32_t *)args;
        }
        if(CY_EOK != (retval = _command_send(pdesc, CMD_GET_IMAGE, NULL, 0, timeout_ms))) {
            break;
        }
        presp = (_command_response_packet_t *)pdesc->_private.buffer;
        __debug_message("Get fp image score: %d\n", presp->data[0]);
    } while(0);

    return retval;
}

static int32_t __ioctl_download_template(zazxxx_describe_t *pdesc, uint8_t *pdata, uint16_t length, 
                                         uint16_t ram_buffer_id, int32_t timeout_ms)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint8_t buf[4] = {0};
    uint8_t *p = NULL;
    uint16_t size = 0;

    do {
        if(!pdata) {
            __debug_error("Template data is NULL, can not download template to zazxxx driver\n");
            break;
        }
        if(length > _TEMPLATE_MAX_SIZE) {
            __debug_error("Template size(%d) is too large\n", length);
            break;
        }
        memcpy(buf, &length, sizeof(length));
        memcpy(&buf[2], &ram_buffer_id, sizeof(ram_buffer_id));
        if(CY_EOK != (retval = _command_send(pdesc, CMD_DOWN_CHAR, buf, ARRAY_SIZE(buf), timeout_ms))) {
            break;
        }
        /* send first template packet */
        p = pdata;
        size = (length > 512 ? 512 : length);
        if(CY_EOK != (retval = _command_template_send(pdesc, CMD_DOWN_CHAR, p, size, 0, timeout_ms))) {
            __debug_error("Send template block(%d) failed\n", 0);
            break;
        }
        /* send second template packet */
        if(length > size) {
            p = pdata + size;
            size = length - size;
        } else {
            p = NULL;
            size = 0;
        }
        if(CY_EOK != (retval = _command_template_send(pdesc, CMD_DOWN_CHAR, p, size, 1, timeout_ms))) {
            __debug_error("Send template block(%d) failed\n", 1);
            break;
        }
    } while(0);

    return retval;
}

static int32_t __ioctl_store_template(zazxxx_describe_t *pdesc, uint16_t template_id, 
                                      uint16_t ram_buffer_id, int32_t timeout_ms)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint8_t buf[4] = {0};

    do {
        memcpy(buf, &template_id, sizeof(template_id));
        memcpy(buf + sizeof(template_id), &ram_buffer_id, sizeof(ram_buffer_id));
        retval = _command_send(pdesc, CMD_STORE_CHAR, buf, ARRAY_SIZE(buf), timeout_ms);
    } while(0);

    return retval;
}

static int32_t _ioctl_enroll_by_template(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    zazxxx_template_args_t *ptemp = (zazxxx_template_args_t *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, can not enroll template\n");
            break;
        }
        if(CY_EOK != (retval = __ioctl_download_template(pdesc, ptemp->pdata, ptemp->size, 
                                                         _TEMPLATE_RAM_BUFFER_ID, ptemp->timeout_ms))) {
            break;
        }
        retval = __ioctl_store_template(pdesc, ptemp->template_id, _TEMPLATE_RAM_BUFFER_ID, ptemp->timeout_ms);
    } while(0);

    return retval;
}

static int32_t __ioctl_delete_template(zazxxx_describe_t *pdesc, uint16_t start, uint16_t end, int32_t timeout_ms)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint8_t buf[4] = {0};
    
    memcpy(buf, &start, sizeof(start));
    memcpy(buf + sizeof(start), &end, sizeof(end));
    retval = _command_send(pdesc, CMD_DEL_CHAR, buf, ARRAY_SIZE(buf), timeout_ms);

    return retval;
}

static int32_t _ioctl_delete_one_template(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint16_t *ptmpl_id = (uint16_t *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, delete template failed\n");
            break;
        }
        retval = __ioctl_delete_template(pdesc, *ptmpl_id, *ptmpl_id, _DEFAULT_TIMEOUT_MS);
    } while(0);

    return retval;
}

static int32_t _ioctl_delete_all_template(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_EOK;
    int32_t timeout_ms = _DEFAULT_TIMEOUT_MS;

    if(args) {
        timeout_ms = *(int32_t *)args;
    }

    retval = __ioctl_delete_template(pdesc, _TEMPLATE_ID_MIN, _TEMPLATE_ID_MAX, timeout_ms);

    return retval;
}

static int32_t __ioctl_get_enroll_counts(zazxxx_describe_t *pdesc, uint16_t start, uint16_t end, int32_t timeout_ms)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint8_t buf[4] = {0};
    _command_response_packet_t *presp = NULL;

    do {
        memcpy(buf, &start, sizeof(start));
        memcpy(buf + sizeof(start), &end, sizeof(end));
        if(CY_EOK != (retval = _command_send(pdesc, CMD_GET_ENROLL_COUNT, buf, ARRAY_SIZE(buf), timeout_ms))) {
            break;
        }
        presp = (_command_response_packet_t *)pdesc->_private.buffer;
        retval = (((uint16_t)presp->data[1]) << 8) | presp->data[0];
        __debug_message("Enroll counts:%d\n", retval);
    } while(0);

    return retval;
}

static int32_t _ioctl_get_enroll_counts(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *pcounts = (uint32_t *)args;

    do {
        if(!args) {
            __debug_error("Args is NULL, can not get enroll counts\n");
            break;
        }
        retval = __ioctl_get_enroll_counts(pdesc, _TEMPLATE_ID_MIN, _TEMPLATE_ID_MAX, _DEFAULT_TIMEOUT_MS);
        if(retval < CY_EOK) {
            break;
        }
        *pcounts = (uint32_t)retval;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t inline __ioctl_generate_template(zazxxx_describe_t *pdesc, uint16_t ram_buffer_id, int32_t timeout_ms)
{
    return  _command_send(pdesc, CMD_GENERATE, (uint8_t *)&ram_buffer_id, sizeof(ram_buffer_id), timeout_ms);
}

static int32_t _ioctl_generate_template(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_EOK;
    int32_t timeout_ms = _DEFAULT_TIMEOUT_MS;

    if(args) {
        timeout_ms = *(int32_t *)args;
    }
    retval = __ioctl_generate_template(pdesc, _GENERATE_RAM_BUFFER_ID, timeout_ms);

    return retval;
}

static int32_t __ioctl_search_template(zazxxx_describe_t *pdesc, uint16_t start, uint16_t end, 
                                       uint16_t ram_buffer_id, int32_t timeout_ms)
{
    uint8_t buf[6] = {0};

    memcpy(buf, &ram_buffer_id, sizeof(ram_buffer_id));
    memcpy(buf + sizeof(ram_buffer_id), &start, sizeof(start));
    memcpy(buf + sizeof(ram_buffer_id) + sizeof(start), &end, sizeof(end));

    return _command_send(pdesc, CMD_SEARCH, buf, ARRAY_SIZE(buf), timeout_ms);
}

static int32_t _ioctl_search_template(zazxxx_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    zazxxx_search_args_t *psearch = (zazxxx_search_args_t *)args;
    _command_response_packet_t *presp = NULL;

    do {
        if(!args) {
            __debug_error("Args is NULL, can not search template\n");
            break;
        }
        if(CY_EOK != (retval = __ioctl_search_template(pdesc, _TEMPLATE_ID_MIN, _TEMPLATE_ID_MAX, 
                                                       _GENERATE_RAM_BUFFER_ID, psearch->timeout_ms))) {
            break;
        }
        presp = (_command_response_packet_t *)pdesc->_private.buffer;
        memcpy(&psearch->template_id, presp->data, sizeof(psearch->template_id));
        psearch->score = presp->data[3];
        __debug_message("Template id:%d, score:%d\n", psearch->template_id, psearch->score);
    } while(0);

    return retval;
}

#if 0
static int32_t __ioctl_get_parameter(zazxxx_describe_t *pdesc, zazxxx_parameter_type_en type, 
                                     uint32_t *para, int32_t timeout_ms)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint8_t buf = (uint8_t)type;
    _command_response_packet_t *presp = NULL;

    do {
        if(!para) {
            __debug_warn("Para is NULL, can not to store parameters\n");
            break;
        }
        presp = (_command_response_packet_t *)pdesc->_private.buffer;
        if(CY_EOK != (retval = _command_send(pdesc, CMD_GET_PARAM, &buf, sizeof(buf), timeout_ms))) {
            break;
        }
        memcpy(para, presp->data, sizeof(*para));
        __debug_message("Get type(%02X) parameters: %d\n", buf, *para);
    } while(0);

    return retval;
}

static int32_t __ioctl_set_parameter(zazxxx_describe_t *pdesc, zazxxx_parameter_type_en type, 
                                     uint32_t para, int32_t timeout_ms)
{
    int32_t retval = CY_EOK;
    uint8_t buf[5] = {0};

    do {
        buf[0] = (uint8_t)type;
        memcpy(&buf[1], &para, sizeof(para));
        retval = _command_send(pdesc, CMD_SET_PARAM, buf, ARRAY_SIZE(buf), timeout_ms);
    } while(0);

    return retval;
}
#endif

static int32_t zazxxx_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    zazxxx_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_warn("No device bind to zazxxx driver module\n");
            break;
        }
        if(NULL == (cb = _ioctl_cb_func_find(cmd))) {
            __debug_warn("Not support this ioctl command(%02X) in zazxxx driver\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}

static int32_t _receive_fsm(zazxxx_describe_t *pdesc, uint8_t byte)
{
    static _recv_state state = STATE_RX_IDLE;
    static uint16_t check_length = 0;
    uint16_t prefix = 0;
    uint16_t checksum = 0;

    if(!pdesc->_private.offset) {
        state = STATE_RX_IDLE;
    }
    switch(state) {
        case STATE_RX_RECV_FIXED_PART:
            pdesc->_private.buffer[pdesc->_private.offset++] = byte;
            if(pdesc->_private.offset >= offsetof(_command_response_packet_t, data)) {
                memcpy(&prefix, pdesc->_private.buffer, sizeof(prefix));
                if(prefix == _START_CODE_RESPONSE_COMMAND_PACKET) {
                    check_length = offsetof(_command_response_packet_t, data) + 14;
                } else {
                    memcpy(&check_length, &pdesc->_private.buffer[offsetof(_command_response_packet_t, length)], 
                           sizeof(check_length));
                    check_length += offsetof(_command_response_packet_t, ret);
                }
                if(check_length > _COMMAND_PACKET_MAX_LENGTH(pdesc->_private.buffer)) {
                    state = STATE_RX_IDLE;
                    pdesc->_private.offset = 0;
                    check_length = 0;
                } else if(check_length == offsetof(_command_response_packet_t, data)) {
                    check_length += _COMMAND_PACKET_CHECKSUM_LENGTH;
                    state = STATE_RX_WAIT_EOF;
                } else {
                    state = STATE_RX_RECV_DATA_PART;
                }
            }
            break;
        case STATE_RX_RECV_DATA_PART:
            pdesc->_private.buffer[pdesc->_private.offset++] = byte;
            if(pdesc->_private.offset >= check_length) {
                state = STATE_RX_WAIT_EOF;
                check_length += _COMMAND_PACKET_CHECKSUM_LENGTH;
            }
            break;
        case STATE_RX_WAIT_EOF:
            pdesc->_private.buffer[pdesc->_private.offset++] = byte;
            if(pdesc->_private.offset >= check_length) {
                state = STATE_RX_IDLE;
                /* check checksum */
                memcpy(&checksum, &pdesc->_private.buffer[check_length - 2], sizeof(checksum));
                if(checksum == checksum_sum16(pdesc->_private.buffer, check_length - 2)) {
                    pdesc->_private.state = RECV_STATE_BUSY;
                }
            }
            break;
        case STATE_RX_IDLE:
            if(_IS_A_PART_OF_PREFIX(byte)) {
                pdesc->_private.buffer[pdesc->_private.offset++] = byte;
                if(pdesc->_private.offset == sizeof(prefix)) {
                    memcpy(&prefix, pdesc->_private.buffer, sizeof(prefix));
                    if(prefix == _START_CODE_RESPONSE_COMMAND_PACKET ||
                       prefix == _START_CODE_RESPONSE_DATA_PACKET) {
                        state = STATE_RX_RECV_FIXED_PART;
                    } else {
                        pdesc->_private.offset = 0;
                    }
                }
            } else {
                pdesc->_private.offset = 0;
            }
            break;
    }

    return CY_EOK;
}

static int32_t zazxxx_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len)
{
    zazxxx_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;
    uint8_t *pdata = (uint8_t *)args;

    assert(pdrv);
    assert(args || !len);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    do {
        if(pdesc->_private.state == RECV_STATE_BUSY) {
            break;
        }
        for(uint16_t i = 0; i < len; ++i) {
            _receive_fsm(pdesc, pdata[i]);
            if(pdesc->_private.state == RECV_STATE_BUSY) {
#if defined(__linux__) || defined(__WIN32)
                for(uint16_t i = 0; i < pdesc->_private.offset; ++i) {
                    __debug_cont("%02X ", pdesc->_private.buffer[i]);
                }
                __debug_cont("\n");
#endif
                break;
            }
        }
        if(pdesc->_private.state == RECV_STATE_BUSY && pdesc->serial.irq_handler) {
            retval = pdesc->serial.irq_handler(irq_handler, NULL, 0);
        }
    } while(0);

    return retval;
}
