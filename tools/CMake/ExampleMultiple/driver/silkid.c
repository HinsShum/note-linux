/**
 * @file driver\silkid.c
 *
 * Copyright (C) 2021
 *
 * silkid.c is free software: you can redistribute it and/or modify
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
#include "silkid.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"
#include <string.h>

/*---------- macro ----------*/
#define _SILKID_PACKET_START_CODE                       (0x70)
#define _SILKID_PACKET_END_CODE                         (0x0A)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t silkid_open(driver_t **pdrv);
static void silkid_close(driver_t **pdrv);
static int32_t silkid_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t silkid_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len);

/*---------- type define ----------*/
typedef enum {
    SILKID_RECV_STATUS_IDLE,
    SILKID_RECV_STATUS_BUSY
} silkid_recv_status_en;

typedef enum {
    SILKID_CMD_MD_SYS_WP = 0x11,
    SILKID_CMD_MD_SYS_SP = 0x12,
    SILKID_CMD_MD_SYS_RP = 0x13,
    SILKID_CMD_MD_SYS_STATUS = 0x14,
    SILKID_CMD_MD_ENROLL_SCAN = 0x15,
    SILKID_CMD_MD_ENROLL_TMP = 0x17,
    SILKID_CMD_MD_VERIFY_SCAN = 0x18,
    SILKID_CMD_MD_VERIFY_TMP = 0x20,
    SILKID_CMD_MD_DEL_TMP = 0x26,
    SILKID_CMD_MD_DEL_ALL_TMP = 0x27,
    SILKID_CMD_MD_IDENTIFY_FREE = 0x2F,
    SILKID_CMD_MD_WU_FILE_X = 0x42,
    SILKID_CMD_MD_RU_FILE_X = 0x43,
    SILKID_CMD_MD_SET_TIME = 0x4A,
    SILKID_CMD_MD_GET_TIME = 0x4B,
    SILKID_CMD_MD_UPDATE_FW = 0x72,
    SILKID_CMD_MD_CANCEL_OP = 0x78,
    SILKID_CMD_MD_ENROLL_IMAGE_X = 0x80,
    SILKID_CMD_MD_IDENTIFY_IMAGE_X = 0x81,
    SILKID_CMD_MD_SCAN_IMAGE_X = 0x83,
    SILKID_CMD_MD_READ_TMP_X = 0x89,
    SILKID_CMD_MD_SLEEP = 0x98,
    SILKID_CMD_MD_WAKEUP = 0x99,
    SILKID_CMD_MD_DEL_ALOG = 0x9E,
    SILKID_CMD_MD_LOAD_USER_X = 0xA0,
    SILKID_CMD_MD_LOAD_TMP_X = 0xA2,
    SILKID_CMD_MD_LOAD_LOG_X = 0xA4,
    SILKID_CMD_MD_WT_FILE_X = 0xAA,
    SILKID_CMD_MD_RT_FILE_X = 0xAB,
    SILKID_CMD_MD_PROFILE_WRITE = 0xAC,
    SILKID_CMD_MD_PROFILE_READ = 0xAD,
    SILKID_CMD_MD_PROFILE_DELETE = 0xAE,
    SILKID_CMD_MD_DISCONNECT = 0xCC,
    SILKID_CMD_MD_CORRECTING_SWITCH = 0xCD,
    SILKID_CMD_MD_RESET = 0xD0,
    SILKID_CMD_MD_ADD_USER = 0xF1,
    SILKID_CMD_MD_READ_USER = 0xF2,
    SILKID_CMD_MD_DELETE_USER = 0xF3,
    SILKID_CMD_MD_DEL_ALL_USER = 0xF5,
    SILKID_CMD_MD_DEL_DB = 0xF8,
    SILKID_CMD_MD_DISABLEDEVICE = 0xFA,
    SILKID_CMD_MD_ENABLEDEVICE = 0xFB,
    SILKID_CMD_MD_SCAN_TEMPLATE = 0xFC
} silkid_command_en;

typedef enum {
    SILKID_SYS_PARA_LED = 0x31,
    SILKID_SYS_PARA_LOG = 0x36,
    SILKID_SYS_PARA_MODE = 0x50,
    SILKID_SYS_PARA_TEMP = 0x51,
    SILKID_SYS_PARA_TIMEOUT = 0x62,
    SILKID_SYS_PARA_ID = 0x6D,
    SILKID_SYS_PARA_VER = 0x6E,
    SILKID_SYS_PARA_SERIAL = 0x6F,
    SILKID_SYS_PARA_BUADRATE = 0x71,
    SILKID_SYS_PARA_FINGERPRINT_NUMBERS = 0x73,
    SILKID_SYS_PARA_FINGERPRINT_CAPACITY = 0x74,
    SILKID_SYS_PARA_USER_CAPACITY = 0x79,
    SILKID_SYS_PARA_USER_NUMBERS = 0x7A,
    SILKID_SYS_PARA_LOG_CAPACITY = 0x7B,
    SILKID_SYS_PARA_LOG_NUMBERS = 0x7C,
    SILKID_SYS_PARA_AUTO_RESP = 0x82,
    SILKID_SYS_PARA_BUILD = 0x89,
    SILKID_SYS_PARA_DETECT = 0x94,
    SILKID_SYS_PARA_MESSAGE = 0x96,
    SILKID_SYS_PARA_SIMILARITY = 0x97,
    SILKID_SYS_PARA_DEBUG = 0x98,
    SILKID_SYS_PARA_1_N = 0xA0,
    SILKID_SYS_PARA_1_1 = 0xA1,
    SILKID_SYS_PARA_DEFAULT = 0xA2,
    SILKID_SYS_PARA_IDLE = 0xA3,
    SILKID_SYS_PARA_SLEEP_MODE = 0xA5,
    SILKID_SYS_PARA_IMAGE = 0xA6,
    SILKID_SYS_PARA_USB = 0xA7,
    SILKID_SYS_PARA_TYPE = 0xA8,
    SILKID_SYS_PARA_ICON = 0xA9,
    SILKID_SYS_PARA_ALGORITHM = 0xAA,
    SILKID_SYS_PARA_AUTO_DEL_LOG = 0xAD,
    SILKID_SYS_PARA_IMAGE_QUALITY = 0xF0
} silkid_sys_para_en;

typedef enum {
    SILKID_FLAG_SUCCESS = 0x61,
    SILKID_FLAG_FAILED = 0x63
} silkid_flag_en;

typedef struct {
    uint8_t start;
    uint8_t command;
    uint8_t param[4];
    uint8_t size[4];
    uint8_t flag;
    uint8_t checksum;
    uint8_t end;
} __attribute__((aligned(1))) silkid_general_packet_t;

typedef silkid_general_packet_t silkid_response_packet_t;

/*---------- variable ----------*/
DRIVER_DEFINED(silkid, silkid_open, silkid_close, NULL, NULL, silkid_ioctl, silkid_irq_handler);

/*---------- function ----------*/
static uint32_t __silkid_checksum(const uint8_t *pdata, uint32_t len)
{
    uint32_t sum = 0;

    while(len-- > 0) {
        sum += *pdata++;
    }

    return sum;
}

static void __silkid_reset_normal_buffer(silkid_describe_t *pdesc)
{
    pdesc->normal.offset = 0;
    pdesc->normal.status = SILKID_RECV_STATUS_IDLE;
    pdesc->normal.iswrite = false;
}

static int32_t silkid_open(driver_t **pdrv)
{
    silkid_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->serial.ops.init) {
        retval = (pdesc->serial.ops.init() ? CY_EOK : CY_ERROR);
    }

    return retval;
}

static void silkid_close(driver_t **pdrv)
{
    silkid_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->serial.ops.deinit) {
        pdesc->serial.ops.deinit();
    }
}

static int32_t __silkid_write(silkid_describe_t *pdesc)
{
    pdesc->normal.iswrite = true;
    if(pdesc->serial.ops.dir_change) {
        pdesc->serial.ops.dir_change(SERIAL_DIRECTION_TX);
    }
    pdesc->serial.ops.write(pdesc->normal.buf, pdesc->normal.offset);
    __silkid_reset_normal_buffer(pdesc);
    if(pdesc->serial.ops.dir_change) {
        pdesc->serial.ops.dir_change(SERIAL_DIRECTION_RX);
    }

    return CY_EOK;
}

static int32_t __silkid_write_with_data(silkid_describe_t *pdesc, uint8_t *pbuf, uint32_t len)
{
    uint32_t sum = 0;
    int32_t retval = CY_EOK;

    sum = __silkid_checksum(pbuf, len);
    pdesc->normal.iswrite = true;
    if(pdesc->serial.ops.dir_change) {
        pdesc->serial.ops.dir_change(SERIAL_DIRECTION_TX);
    }
    pdesc->serial.ops.write(pdesc->normal.buf, pdesc->normal.offset);
    __silkid_reset_normal_buffer(pdesc);
    pdesc->serial.ops.write(pbuf, len);
    pdesc->serial.ops.write((uint8_t *)&sum, sizeof(sum));
    if(pdesc->serial.ops.dir_change) {
        pdesc->serial.ops.dir_change(SERIAL_DIRECTION_RX);
    }

    return CY_EOK;
}

static void __silkid_make_general_packet(silkid_describe_t *pdesc, silkid_command_en cmd, 
                                            uint32_t param, uint32_t size, uint8_t flag)
{
    silkid_general_packet_t *pack = (silkid_general_packet_t *)pdesc->normal.buf;

    pack->start = _SILKID_PACKET_START_CODE;
    pack->command = cmd;
    memcpy(pack->param, &param, ARRAY_SIZE(pack->param));
    memcpy(pack->size, &size, ARRAY_SIZE(pack->size));
    pack->flag = flag;
    pack->checksum = __silkid_checksum(&pack->start, offsetof(silkid_general_packet_t, checksum)) & 0xFF;
    pack->end = _SILKID_PACKET_END_CODE;
    pdesc->normal.offset = sizeof(*pack);
}

static int32_t silkid_connect(silkid_describe_t *pdesc)
{
    int32_t retval = CY_ERROR;
    silkid_response_packet_t *pack = (silkid_response_packet_t *)pdesc->normal.buf;
    uint32_t timeoutms = 100;

    __silkid_make_general_packet(pdesc, SILKID_CMD_MD_SYS_STATUS, 0, 0, 0);
    __silkid_write(pdesc);
    while(timeoutms-- > 0) {
        if(SILKID_RECV_STATUS_BUSY == pdesc->normal.status) {
            break;
        }
        __delay_ms(1);
    }
    if(SILKID_RECV_STATUS_BUSY != pdesc->normal.status) {
        retval = CY_E_TIME_OUT;
    } else if(SILKID_FLAG_SUCCESS == pack->flag) {
        retval = CY_EOK;
    }
    __silkid_reset_normal_buffer(pdesc);

    return retval;
}

static int32_t silkid_disconnect(silkid_describe_t *pdesc)
{
    int32_t retval = CY_ERROR;
    silkid_response_packet_t *pack = (silkid_response_packet_t *)pdesc->normal.buf;
    uint32_t timeoutms = 100;

    __silkid_make_general_packet(pdesc, SILKID_CMD_MD_DISCONNECT, 0, 0, 0);
    __silkid_write(pdesc);
    while(timeoutms-- > 0) {
        if(SILKID_RECV_STATUS_BUSY == pdesc->normal.status) {
            break;
        }
        __delay_ms(1);
    }
    if(SILKID_RECV_STATUS_BUSY != pdesc->normal.status) {
        retval = CY_E_TIME_OUT;
    } else if(SILKID_FLAG_SUCCESS == pack->flag) {
        retval = CY_EOK;
    }
    __silkid_reset_normal_buffer(pdesc);

    return retval;
}

static int32_t silkid_delete_all_user_template(silkid_describe_t *pdesc)
{
    int32_t retval = CY_EOK;

    __silkid_make_general_packet(pdesc, SILKID_CMD_MD_DEL_ALL_TMP, 0, 0, 0);
    /* send packet to module */
    __silkid_write(pdesc);

    return retval;
}

static int32_t silkid_delete_template(silkid_describe_t *pdesc, uint32_t id)
{
    int32_t retval = CY_ERROR;
    silkid_response_packet_t *pack = (silkid_response_packet_t *)pdesc->normal.buf;
    uint32_t timeoutms = 200;

    __silkid_make_general_packet(pdesc, SILKID_CMD_MD_DEL_TMP, id, 0, 0x77);
    __silkid_write(pdesc);
    while(timeoutms-- > 0) {
        if(SILKID_RECV_STATUS_BUSY == pdesc->normal.status) {
            break;
        }
        __delay_ms(1);
    }
    if(SILKID_RECV_STATUS_BUSY != pdesc->normal.status) {
        retval = CY_E_TIME_OUT;
    } else if(SILKID_FLAG_SUCCESS == pack->flag) {
        retval = CY_EOK;
    }
    __silkid_reset_normal_buffer(pdesc);

    return retval;
}

static int32_t silkid_delete_user_template(silkid_describe_t *pdesc, uint32_t id)
{
    int32_t retval = CY_EOK;
    silkid_response_packet_t *pack = (silkid_response_packet_t *)pdesc->normal.buf;
    uint32_t timeoutms = 200;

    __silkid_make_general_packet(pdesc, SILKID_CMD_MD_DEL_TMP, id, 0, 0x00);
    __silkid_write(pdesc);
    while(timeoutms-- > 0) {
        if(SILKID_RECV_STATUS_BUSY == pdesc->normal.status) {
            break;
        }
        __delay_ms(1);
    }
    if(SILKID_RECV_STATUS_BUSY != pdesc->normal.status) {
        retval = CY_E_TIME_OUT;
    } else if(SILKID_FLAG_SUCCESS == pack->flag) {
        retval = CY_EOK;
    }
    __silkid_reset_normal_buffer(pdesc);

    return retval;
}

static int32_t silkid_enroll_template(silkid_describe_t *pdesc, silkid_enroll_template_t *enroll)
{
    int32_t retval = CY_ERROR;
    uint16_t finger_id = (enroll->id >> 16) & 0xFFFF;
    uint16_t user_id = enroll->id & 0xFFFF;
    silkid_response_packet_t *pack = (silkid_response_packet_t *)pdesc->normal.buf;
    uint32_t size = 0, timeoutms = 200;

    enroll->result = SILKID_ENROLL_RESULT_FAILE;
    do {
        if(!user_id || finger_id > 10) {
            retval = CY_E_WRONG_ARGS;
            break;
        }
        __silkid_make_general_packet(pdesc, SILKID_CMD_MD_ENROLL_TMP, enroll->id, enroll->size + 4, 0);
        __silkid_write_with_data(pdesc, enroll->template, enroll->size);
        while(timeoutms-- > 0) {
            if(SILKID_RECV_STATUS_BUSY == pdesc->normal.status) {
                break;
            }
            __delay_ms(1);
        }
        if(SILKID_RECV_STATUS_BUSY != pdesc->normal.status) {
            retval = CY_E_TIME_OUT;
            break;
        }
        if(SILKID_FLAG_SUCCESS == pack->flag) {
            enroll->result = SILKID_ENROLL_RESULT_SUCCESS;
            memcpy(&retval, pack->param, ARRAY_SIZE(pack->param));
        } else if(0x6D == pack->flag) {
            enroll->result = SILKID_ENROLL_RESULT_FULL;
        } else if(0x6E == pack->flag) {
            enroll->result = SILKID_ENROLL_RESULT_ID_REPEAT;
        } else {
            enroll->result = SILKID_ENROLL_RESULT_FAILE;
        }
    } while(0);
    __silkid_reset_normal_buffer(pdesc);

    return retval;
}

static int32_t silkid_get_enroll_numbers(silkid_describe_t *pdesc, uint32_t *numbers)
{
    int32_t retval = CY_ERROR;
    silkid_response_packet_t *pack = (silkid_response_packet_t *)pdesc->normal.buf;
    uint32_t timeoutms = 100;

    __silkid_make_general_packet(pdesc, SILKID_CMD_MD_SYS_RP, 0, 0, SILKID_SYS_PARA_FINGERPRINT_NUMBERS);
    __silkid_write(pdesc);
    while(timeoutms-- > 0) {
        if(SILKID_RECV_STATUS_BUSY == pdesc->normal.status) {
            break;
        }
        __delay_ms(1);
    }
    if(SILKID_RECV_STATUS_BUSY != pdesc->normal.status) {
        retval = CY_E_TIME_OUT;
    } else if(SILKID_FLAG_SUCCESS == pack->flag) {
        memcpy(numbers, pack->param, ARRAY_SIZE(pack->param));
        retval = CY_EOK;
    }
    __silkid_reset_normal_buffer(pdesc);

    return retval;
}

static int32_t silkid_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    silkid_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_SILKID_GET_COMPORT:
            if(pdesc) {
                *(uint8_t *)args = pdesc->serial.comport;
            }
            break;
        case IOCTL_SILKID_SET_IRQ_HANDLER:
            if(pdesc) {
                pdesc->serial.ops.irq_handler = (int32_t (*)(uint32_t, void *, uint32_t))args;
            }
            break;
        case IOCTL_SILKID_DIRECTION_CHOOSE:
            if(pdesc && pdesc->serial.ops.dir_change && args) {
                serial_direction_en dir = *(serial_direction_en *)args;
                if(dir > SERIAL_DIRECTION_NRX_NTX) {
                    retval = CY_E_WRONG_ARGS;
                } else {
                    pdesc->serial.ops.dir_change(dir);
                }
            }
            break;
        case IOCTL_SILKID_GET_BAUDRATE:
            if(pdesc && args) {
                *(uint32_t *)args = pdesc->serial.baudrate;
            }
            break;
        case IOCTL_SILKID_SET_BAUDRATE:
            if(pdesc && args) {
                uint32_t baud = *(uint32_t *)args;
                if(pdesc->serial.baudrate != baud) {
                    pdesc->serial.baudrate = baud;
                    if(pdesc->serial.ops.deinit) {
                        pdesc->serial.ops.deinit();
                    }
                    if(pdesc->serial.ops.init) {
                        retval = (pdesc->serial.ops.init() ? CY_EOK : CY_ERROR);
                    }
                }
            }
            break;
        case IOCTL_SILKID_CONNECT:
            if(pdesc) {
                retval = silkid_connect(pdesc);
            }
            break;
        case IOCTL_SILKID_DISCONNECT:
            if(pdesc) {
                retval = silkid_disconnect(pdesc);
            }
            break;
        case IOCTL_SILKID_DELETE_ALL_TEMPLATE:
            if(pdesc) {
                retval = silkid_delete_all_user_template(pdesc);
            }
            break;
        case IOCTL_SILKID_DELETE_TEMPLATE:
            if(pdesc && args) {
                retval = silkid_delete_template(pdesc, *(uint32_t *)args);
            }
            break;
        case IOCTL_SILKID_DELETE_USER_TEMPLATE:
            if(pdesc && args) {
                retval = silkid_delete_user_template(pdesc, *(uint32_t *)args);
            }
            break;
        case IOCTL_SILKID_ENROLL_TEMPLATE:
            if(pdesc && args) {
                retval = silkid_enroll_template(pdesc, args);
            }
            break;
        case IOCTL_SILKID_GET_ENROLL_NUMBERS:
            if(pdesc && args) {
                retval = silkid_get_enroll_numbers(pdesc, (uint32_t *)args);
            }
            break;
        default:
            retval = CY_E_WRONG_ARGS;
            break;
    }

    return retval;
}

static int32_t silkid_irq_recv_pack(silkid_describe_t *pdesc, uint8_t ch)
{
    int32_t retval = CY_EOK;
    uint32_t sum = 0;
    silkid_response_packet_t *pack = (silkid_response_packet_t *)pdesc->normal.buf;

    do {
        if(SILKID_RECV_STATUS_BUSY == pdesc->normal.status) {
            if(pdesc->normal.iswrite) {
                retval = CY_E_BUSY;
                break;
            }
            __silkid_reset_normal_buffer(pdesc);
        }
        /* recv start code */
        if(0 == pdesc->normal.offset) {
            if(_SILKID_PACKET_START_CODE != ch) {
                break;
            }
        }
        pdesc->normal.buf[pdesc->normal.offset++] = ch;
        if(pdesc->normal.offset == sizeof(*pack)) {
            /* check packet end code */
            if(_SILKID_PACKET_END_CODE != ch) {
                retval = CY_ERROR;
                break;
            }
            /* check sum */
            if(pack->checksum != (__silkid_checksum((uint8_t *)pack, 
                                  offsetof(silkid_response_packet_t, checksum)) & 0xFF)) {
                retval = CY_E_WRONG_CRC;
                break;
            }
            pdesc->normal.status = SILKID_RECV_STATUS_BUSY;
        }
    } while(0);
    if(CY_ERROR == retval || CY_E_WRONG_CRC == retval) {
        __silkid_reset_normal_buffer(pdesc);
    }

    return retval;
}

static int32_t silkid_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len)
{
    silkid_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;
    uint8_t *pdata = (uint8_t *)args;
    silkid_response_packet_t *pack = NULL;
    silkid_irq_handler_args_t irq_args = {
        .id = 0,
        .msg = SILKID_MESSAGE_NONE
    };

    assert(pdrv);
    assert(args || !len);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    pack = (silkid_response_packet_t *)pdesc->normal.buf;
    while(len-- > 0) {
        silkid_irq_recv_pack(pdesc, *pdata++);
    }
    do {
        if(SILKID_RECV_STATUS_BUSY == pdesc->normal.status) {
            if(SILKID_CMD_MD_IDENTIFY_FREE == pack->command) {
                memcpy(&irq_args.id, pack->param, sizeof(irq_args.id));
                irq_args.msg = (SILKID_FLAG_SUCCESS == pack->flag ? SILKID_MESSAGE_VERIFY_SUCCESS : SILKID_MESSAGE_VERIFY_FAILE);
            } else if(SILKID_CMD_MD_DEL_ALL_TMP == pack->command) {
                irq_args.msg = (SILKID_FLAG_SUCCESS == pack->flag ? SILKID_MESSAGE_CLEAR_TEMPLATE_SUCCESS : SILKID_MESSAGE_CLEAR_TEMPLATE_FAILE);
            } else {
                break;
            }
            if(pdesc->serial.ops.irq_handler) {
                retval = pdesc->serial.ops.irq_handler(irq_handler, &irq_args, sizeof(irq_args));
            }
            __silkid_reset_normal_buffer(pdesc);
        }
    } while(0);

    return retval;
}
