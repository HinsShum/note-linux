/**
 * @file driver\d100.c
 *
 * Copyright (C) 2021
 *
 * d100.c is free software: you can redistribute it and/or modify
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
#include "d100.h"
#include "driver.h"
#include "utils.h"
#include "checksum.h"
#include "config/errorno.h"
#include "config/options.h"
#include <string.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _open(driver_t **pdrv);
static void _close(driver_t **pdrv);
static int32_t _write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t length);
static int32_t _read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t length);
static int32_t _ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t _irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);

static int32_t __ioctl_get_comport(d100_describe_t *pdesc, void *args);
static int32_t __ioctl_set_irq(d100_describe_t *pdesc, void *args);
static int32_t __ioctl_get_baudrate(d100_describe_t *pdesc, void *args);
static int32_t __ioctl_set_baudrate(d100_describe_t *pdesc, void *args);
static int32_t __ioctl_clear_status(d100_describe_t *pdesc, void *args);
static int32_t __ioctl_power_on(d100_describe_t *pdesc, void *args);
static int32_t __ioctl_power_off(d100_describe_t *pdesc, void *args);

static int32_t __write_version(d100_describe_t *pdesc, uint8_t *buf, uint32_t length);
static int32_t __write_reset(d100_describe_t *pdesc, uint8_t *buf, uint32_t length);
static int32_t __write_del_a_user(d100_describe_t *pdesc, uint8_t *buf, uint32_t length);
static int32_t __write_del_all_users(d100_describe_t *pdesc, uint8_t *buf, uint32_t length);
static int32_t __write_get_a_user_info(d100_describe_t *pdesc, uint8_t *buf, uint32_t length);
static int32_t __write_get_all_users_id(d100_describe_t *pdesc, uint8_t *buf, uint32_t length);
static int32_t __write_verify(d100_describe_t *pdesc, uint8_t *buf, uint32_t length);
static int32_t __write_enroll_single(d100_describe_t *pdesc, uint8_t *buf, uint32_t length);
static int32_t __write_power_down(d100_describe_t *pdesc, uint8_t *buf, uint32_t length);

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(d100_describe_t *pdesc, void *args);
typedef struct {
    uint32_t cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

typedef int32_t (*write_cb_func_t)(d100_describe_t *pdesc, uint8_t *buf, uint32_t length);
typedef struct {
    uint32_t msgid;
    write_cb_func_t cb;
} write_cb_t;

typedef enum {
    STATE_RECV_SYNC_WORD,
    STATE_RECV_MSGID,
    STATE_RECV_SIZE,
    STATE_RECV_DATA,
    STATE_RECV_PARITY
} d100_recv_state_en_t;

/*---------- variable ----------*/
DRIVER_DEFINED(d100, _open, _close, _write, _read, _ioctl, _irq_handler);
static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_D100_GET_COMPORT, __ioctl_get_comport},
    {IOCTL_D100_SET_IRQ_HANDLER, __ioctl_set_irq},
    {IOCTL_D100_GET_BAUDRATE, __ioctl_get_baudrate},
    {IOCTL_D100_SET_BAUDRATE, __ioctl_set_baudrate},
    {IOCTL_D100_CLEAR_STATUS, __ioctl_clear_status},
    {IOCTL_DEVICE_POWER_ON, __ioctl_power_on},
    {IOCTL_DEVICE_POWER_OFF, __ioctl_power_off}
};
static write_cb_t write_cb_array[] = {
    {D100_MSGID_VERSION, __write_version},
    {D100_MSGID_RESET, __write_reset},
    {D100_MSGID_DELUSER, __write_del_a_user},
    {D100_MSGID_DELALL, __write_del_all_users},
    {D100_MSGID_GETUSERINFO, __write_get_a_user_info},
    {D100_MSGID_GET_ALL_USERID, __write_get_all_users_id},
    {D100_MSGID_VERIFY, __write_verify},
    {D100_MSGID_ENROLL_SINGLE, __write_enroll_single},
    {D100_MSGID_POWERDOWN, __write_power_down}
};

/*---------- function ----------*/
static int32_t _open(driver_t **pdrv)
{
    d100_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("D100 device has not describe field\n");
            break;
        }
        retval = CY_EOK;
        pingpong_buffer_init(&pdesc->pingpong, &pdesc->buffer1, &pdesc->buffer2);
        pingpong_buffer_get_write_buf(&pdesc->pingpong, (void **)&pdesc->cur_buffer);
        if(pdesc->gpio.ops.init) {
            if(!pdesc->gpio.ops.init()) {
                retval = CY_ERROR;
                break;
            }
        }
        if(pdesc->serial.ops.init) {
            if(!pdesc->serial.ops.init()) {
                retval = CY_ERROR;
            }
        }
    } while(0);

    return retval;
}

static void _close(driver_t **pdrv)
{
    d100_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc) {
        if(pdesc->serial.ops.deinit) {
            pdesc->serial.ops.deinit();
        }
        if(pdesc->gpio.ops.deinit) {
            pdesc->gpio.ops.deinit();
        }
    }
}

static void __msg_init(d100_msg_t *pmsg, uint8_t msgid, uint16_t size)
{
    pmsg->sync = utils_htons(D100_PACKAGE_SYNC_WORD);
    pmsg->msgid = msgid;
    size = utils_htons(size);
    memcpy(pmsg->size, &size, sizeof(size));
}

static write_cb_func_t __write_cb_func_find(uint32_t msgid)
{
    write_cb_func_t cb = NULL;

    for(uint32_t i = 0; i < ARRAY_SIZE(write_cb_array); ++i) {
        if(write_cb_array[i].msgid == msgid) {
            cb = write_cb_array[i].cb;
            break;
        }
    }

    return cb;
}

static int32_t _write(driver_t **pdrv, void *buf, uint32_t msgid, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_describe_t *pdesc = NULL;
    write_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("D100 device has no describe field\n");
            break;
        }
        if(NULL == (cb = __write_cb_func_find(msgid))) {
            __debug_error("D100 driver not support this msgid(%02X)\n", msgid);
            break;
        }
        retval = cb(pdesc, (uint8_t *)buf, length);
    } while(0);

    return retval;
}

static int32_t __write_version(d100_describe_t *pdesc, uint8_t *buf, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_msg_t msg = {0};
    uint8_t xor = 0;

    do {
        if(buf || length) {
            __debug_error("Args format error for d100 get version\n");
            break;
        }
        if(pdesc->status.state != STATE_IDLE) {
            __debug_warn("D100 is busy\n");
            retval = CY_E_BUSY;
            break;
        }
        __msg_init(&msg, (uint8_t)D100_MSGID_VERSION, length);
        xor = checksum_xor(&msg.msgid, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE - sizeof(msg.sync));
        pdesc->serial.ops.write((uint8_t *)&msg, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE);
        pdesc->serial.ops.write(&xor, D100_PACKAGE_PARITY_SIZE);
        pdesc->status.msgid = (uint8_t)D100_MSGID_VERSION;
        pdesc->status.state = STATE_WAIT_RESP;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t __write_reset(d100_describe_t *pdesc, uint8_t *buf, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_msg_t msg = {0};
    uint8_t xor = 0;

    do {
        if(buf || length) {
            __debug_error("Args format error for d100 reset\n");
            break;
        }
        __msg_init(&msg, (uint8_t)D100_MSGID_RESET, length);
        xor = checksum_xor(&msg.msgid, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE - sizeof(msg.sync));
        pdesc->serial.ops.write((uint8_t *)&msg, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE);
        pdesc->serial.ops.write(&xor, D100_PACKAGE_PARITY_SIZE);
        pdesc->status.msgid = (uint8_t)D100_MSGID_RESET;
        pdesc->status.state = STATE_WAIT_RESP;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t __write_del_a_user(d100_describe_t *pdesc, uint8_t *buf, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_msg_t msg = {0};
    uint8_t xor[2] = {0};

    do {
        if(!buf || length != sizeof(d100_user_id_t)) {
            __debug_error("Args format error for d100 del a user\n");
            break;
        }
        if(pdesc->status.state != STATE_IDLE) {
            __debug_warn("D100 is busy\n");
            retval = CY_E_BUSY;
            break;
        }
        __msg_init(&msg, (uint8_t)D100_MSGID_DELUSER, length);
        xor[0] = checksum_xor(&msg.msgid, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE - sizeof(msg.sync));
        xor[1] = checksum_xor(buf, length);
        xor[0] = checksum_xor(xor, ARRAY_SIZE(xor));
        pdesc->serial.ops.write((uint8_t *)&msg, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE);
        pdesc->serial.ops.write(buf, length);
        pdesc->serial.ops.write(&xor[0], D100_PACKAGE_PARITY_SIZE);
        pdesc->status.msgid = (uint8_t)D100_MSGID_DELUSER;
        pdesc->status.state = STATE_WAIT_RESP;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t __write_del_all_users(d100_describe_t *pdesc, uint8_t *buf, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_msg_t msg = {0};
    uint8_t xor = 0;

    do {
        if(buf || length) {
            __debug_error("Args format error for d100 del all users\n");
            break;
        }
        if(pdesc->status.state != STATE_IDLE) {
            __debug_warn("D100 is busy\n");
            retval = CY_E_BUSY;
            break;
        }
        __msg_init(&msg, (uint8_t)D100_MSGID_DELALL, length);
        xor = checksum_xor(&msg.msgid, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE - sizeof(msg.sync));
        pdesc->serial.ops.write((uint8_t *)&msg, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE);
        pdesc->serial.ops.write(&xor, D100_PACKAGE_PARITY_SIZE);
        pdesc->status.msgid = (uint8_t)D100_MSGID_DELALL;
        pdesc->status.state = STATE_WAIT_RESP;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t __write_get_a_user_info(d100_describe_t *pdesc, uint8_t *buf, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_msg_t msg = {0};
    uint8_t xor[2] = {0};

    do {
        if(!buf || length != sizeof(d100_user_id_t)) {
            __debug_error("Args format error for d100 get user information\n");
            break;
        }
        if(pdesc->status.state != STATE_IDLE) {
            __debug_warn("D100 is busy\n");
            retval = CY_E_BUSY;
            break;
        }
        __msg_init(&msg, (uint8_t)D100_MSGID_GETUSERINFO, length);
        xor[0] = checksum_xor(&msg.msgid, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE - sizeof(msg.sync));
        xor[1] = checksum_xor(buf, length);
        xor[0] = checksum_xor(xor, ARRAY_SIZE(xor));
        pdesc->serial.ops.write((uint8_t *)&msg, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE);
        pdesc->serial.ops.write(buf, length);
        pdesc->serial.ops.write(&xor[0], D100_PACKAGE_PARITY_SIZE);
        pdesc->status.msgid = (uint8_t)D100_MSGID_GETUSERINFO;
        pdesc->status.state = STATE_WAIT_RESP;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t __write_get_all_users_id(d100_describe_t *pdesc, uint8_t *buf, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_msg_t msg = {0};
    uint8_t xor = 0;

    do {
        if(buf || length) {
            __debug_error("Args format error for d100 get all users id\n");
            break;
        }
        if(pdesc->status.state != STATE_IDLE) {
            __debug_warn("D100 is busy\n");
            retval = CY_E_BUSY;
            break;
        }
        __msg_init(&msg, (uint8_t)D100_MSGID_GET_ALL_USERID, length);
        xor = checksum_xor(&msg.msgid, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE - sizeof(msg.sync));
        pdesc->serial.ops.write((uint8_t *)&msg, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE);
        pdesc->serial.ops.write(&xor, D100_PACKAGE_PARITY_SIZE);
        pdesc->status.msgid = (uint8_t)D100_MSGID_GET_ALL_USERID;
        pdesc->status.state = STATE_WAIT_RESP;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t __write_verify(d100_describe_t *pdesc, uint8_t *buf, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_msg_t msg = {0};
    uint8_t xor[2] = {0};

    do {
        if(!buf || length != sizeof(d100_verify_t)) {
            __debug_error("Args format error for d100 verify\n");
            break;
        }
        if(pdesc->status.state != STATE_IDLE) {
            __debug_warn("D100 is busy\n");
            retval = CY_E_BUSY;
            break;
        }
        __msg_init(&msg, (uint8_t)D100_MSGID_VERIFY, length);
        xor[0] = checksum_xor(&msg.msgid, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE - sizeof(msg.sync));
        xor[1] = checksum_xor(buf, length);
        xor[0] = checksum_xor(xor, ARRAY_SIZE(xor));
        pdesc->serial.ops.write((uint8_t *)&msg, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE);
        pdesc->serial.ops.write(buf, length);
        pdesc->serial.ops.write(&xor[0], D100_PACKAGE_PARITY_SIZE);
        pdesc->status.msgid = (uint8_t)D100_MSGID_VERIFY;
        pdesc->status.state = STATE_WAIT_RESP;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t __write_enroll_single(d100_describe_t *pdesc, uint8_t *buf, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_msg_t msg = {0};
    uint8_t xor[2] = {0};

    do {
        if(!buf || length != sizeof(d100_enroll_t)) {
            __debug_error("Args format error for d100 enroll single face\n");
            break;
        }
        if(pdesc->status.state != STATE_IDLE) {
            __debug_warn("D100 is busy\n");
            retval = CY_E_BUSY;
            break;
        }
        __msg_init(&msg, (uint8_t)D100_MSGID_ENROLL_SINGLE, length);
        xor[0] = checksum_xor(&msg.msgid, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE - sizeof(msg.sync));
        xor[1] = checksum_xor(buf, length);
        xor[0] = checksum_xor(xor, ARRAY_SIZE(xor));
        pdesc->serial.ops.write((uint8_t *)&msg, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE);
        pdesc->serial.ops.write(buf, length);
        pdesc->serial.ops.write(&xor[0], D100_PACKAGE_PARITY_SIZE);
        pdesc->status.msgid = (uint8_t)D100_MSGID_ENROLL_SINGLE;
        pdesc->status.state = STATE_WAIT_RESP;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t __write_power_down(d100_describe_t *pdesc, uint8_t *buf, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_msg_t msg = {0};
    uint8_t xor = 0;

    do {
        if(buf || length) {
            __debug_error("Args format error for d100 power down\n");
            break;
        }
        if(pdesc->status.state != STATE_IDLE) {
            __debug_warn("D100 is busy\n");
            retval = CY_E_BUSY;
            break;
        }
        __msg_init(&msg, (uint8_t)D100_MSGID_POWERDOWN, length);
        xor = checksum_xor(&msg.msgid, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE - sizeof(msg.sync));
        pdesc->serial.ops.write((uint8_t *)&msg, D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE);
        pdesc->serial.ops.write(&xor, D100_PACKAGE_PARITY_SIZE);
        pdesc->status.msgid = (uint8_t)D100_MSGID_POWERDOWN;
        pdesc->status.state = STATE_WAIT_RESP;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t _read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t length)
{
    int32_t retval = CY_E_WRONG_ARGS;
    d100_describe_t *pdesc = NULL;
    d100_buffer_t *buffer = NULL;
    d100_msg_t *pmsg = NULL;
    d100_read_context_t *ctx = buf;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("D100 device has no describe field\n");
            break;
        }
        if(!buf) {
            __debug_error("No buffer to store the d100 resp context\n");
            break;
        }
        if(!pingpong_buffer_get_read_buf(&pdesc->pingpong, (void **)&buffer)) {
            retval = CY_E_NO_MEMORY;
            break;
        }
        retval = CY_EOK;
        pmsg = (d100_msg_t *)buffer->buf;
        ctx->msgid = pmsg->msgid;
        ctx->size = (pmsg->size[0] << 8) | pmsg->size[1];
        memcpy(ctx->buf, pmsg->data, ctx->size);
        pingpong_buffer_set_read_done(&pdesc->pingpong);
        if(pmsg->msgid == D100_MSGID_REPLAY && pdesc->status.state == STATE_WAIT_RESP && pmsg->data[0] == pdesc->status.msgid) {
            pdesc->status.state = STATE_IDLE;
            pdesc->status.msgid = 0;
        }
    } while(0);

    return retval;
}

static d100_recv_state_en_t __recv_fsm_sync_word(d100_buffer_t *buffer, uint8_t byte)
{
    d100_recv_state_en_t state = STATE_RECV_SYNC_WORD;
    uint8_t sync_word_hi = (D100_PACKAGE_SYNC_WORD >> 8) & 0xFF;
    uint8_t sync_word_lo = (D100_PACKAGE_SYNC_WORD & 0xFF);

    do {
        if(byte != sync_word_hi && byte != sync_word_lo) {
            buffer->offset = 0;
            break;
        }
        buffer->buf[buffer->offset++] = byte;
        if(byte == sync_word_lo) {
            state = STATE_RECV_MSGID;
        }
    } while(0);

    return state;
}

static d100_recv_state_en_t __recv_fsm_msgid(d100_buffer_t *buffer, uint8_t byte)
{
    buffer->buf[buffer->offset++] = byte;

    return STATE_RECV_SIZE;
}

static d100_recv_state_en_t __recv_fsm_size(d100_buffer_t *buffer, uint8_t byte)
{
    d100_recv_state_en_t state = STATE_RECV_SIZE;
    uint16_t expected_size = 0;
    d100_msg_t *pmsg = NULL;

    do {
        buffer->buf[buffer->offset++] = byte;
        if(buffer->offset == (D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE)) {
            /* parse expected size */
            pmsg = (d100_msg_t *)buffer->buf;
            expected_size = (pmsg->size[0] << 8) | pmsg->size[1];
            if(expected_size > ARRAY_SIZE(buffer->buf)) {
                /* drop this package */
                state = STATE_RECV_SYNC_WORD;
                buffer->offset = 0;
                buffer->expected_length = 0;
                break;
            } else if(expected_size == 0) {
                state = STATE_RECV_PARITY;
                buffer->expected_length = 0;
                break;
            }
            buffer->expected_length = expected_size;
            state = STATE_RECV_DATA;
        }
    } while(0);

    return state;
}

static d100_recv_state_en_t __recv_fsm_data(d100_buffer_t *buffer, uint8_t byte)
{
    d100_recv_state_en_t state = STATE_RECV_DATA;

    buffer->buf[buffer->offset++] = byte;
    if(buffer->offset >= (D100_PACKAGE_SIZE_MINIMUM - D100_PACKAGE_PARITY_SIZE + buffer->expected_length)) {
        state = STATE_RECV_PARITY;
    }

    return state;
}

static d100_recv_state_en_t __recv_fsm_parity(d100_buffer_t *buffer, uint8_t byte)
{
    buffer->buf[buffer->offset++] = byte;

    return STATE_RECV_SYNC_WORD;
}

static int32_t __recv_fsm_check(d100_buffer_t *buffer)
{
    d100_msg_t *pmsg = (d100_msg_t *)buffer->buf;
    uint8_t xor = 0;
    int32_t retval = CY_EOK;

    xor = checksum_xor(&pmsg->msgid, buffer->offset - sizeof(pmsg->sync) - D100_PACKAGE_PARITY_SIZE);
    if(xor == buffer->buf[buffer->offset - 1]) {
        retval = CY_E_BUSY;
    }

    return retval;
}

static int32_t __recv_fsm_in_irq(d100_buffer_t *buffer, uint8_t byte)
{
    int32_t retval = CY_EOK;
    static d100_recv_state_en_t state = STATE_RECV_SYNC_WORD;

    switch(state) {
        case STATE_RECV_SYNC_WORD:
            state = __recv_fsm_sync_word(buffer, byte);
            break;
        case STATE_RECV_MSGID:
            state = __recv_fsm_msgid(buffer, byte);
            break;
        case STATE_RECV_SIZE:
            state = __recv_fsm_size(buffer, byte);
            break;
        case STATE_RECV_DATA:
            state = __recv_fsm_data(buffer, byte);
            break;
        case STATE_RECV_PARITY:
            state = __recv_fsm_parity(buffer, byte);
            retval = __recv_fsm_check(buffer);
            break;
        default:
            state = STATE_RECV_SYNC_WORD;
            buffer->offset = 0;
            buffer->expected_length = 0;
            break;
    }

    return retval;
}

static int32_t _irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length)
{
    d100_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            break;
        }
        for(uint32_t i = 0; i < length; ++i) {
            if(CY_E_BUSY == __recv_fsm_in_irq(pdesc->cur_buffer, ((uint8_t *)args)[i])) {
                /* get a complete package */
                pingpong_buffer_set_write_done(&pdesc->pingpong);
                pingpong_buffer_get_write_buf(&pdesc->pingpong, (void **)&pdesc->cur_buffer);
                pdesc->cur_buffer->offset = 0;
                pdesc->cur_buffer->expected_length = 0;
                if(pdesc->serial.ops.irq_handler) {
                    retval = pdesc->serial.ops.irq_handler(irq_handler, NULL, 0);
                }
            }
        }
    } while(0);

    return retval;
}

static ioctl_cb_func_t __ioctl_cb_func_find(uint32_t cmd)
{
    ioctl_cb_func_t cb = NULL;

    for(uint32_t i = 0; i < ARRAY_SIZE(ioctl_cb_array); ++i) {
        if(ioctl_cb_array[i].cmd == cmd) {
            cb = ioctl_cb_array[i].cb;
            break;
        }
    }

    return cb;
}

static int32_t _ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    d100_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("D100 device has no descirbe field\n");
            break;
        }
        if(NULL == (cb = __ioctl_cb_func_find(cmd))) {
            __debug_error("D100 driver not support this command(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}

static int32_t __ioctl_get_comport(d100_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint8_t *comport = (uint8_t *)args;

    do {
        if(!args) {
            __debug_error("No buffer to store the comport of the D100\n");
            break;
        }
        *comport = pdesc->serial.comport;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t __ioctl_set_irq(d100_describe_t *pdesc, void *args)
{
    int32_t (*irq)(uint32_t, void *, uint32_t) = (int32_t (*)(uint32_t, void *, uint32_t))args;
    
    pdesc->serial.ops.irq_handler = irq;

    return CY_EOK;
}

static int32_t __ioctl_get_baudrate(d100_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *baud = (uint32_t *)args;

    do {
        if(!args) {
            __debug_error("No buffer to store the baudrate\n");
            break;
        }
        *baud = pdesc->serial.baudrate;
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t __ioctl_set_baudrate(d100_describe_t *pdesc, void *args)
{
    int32_t retval = CY_E_WRONG_ARGS;
    uint32_t *baud = (uint32_t *)args;

    do {
        if(!args) {
            __debug_error("No baud specified to change the d100 baud\n");
            break;
        }
        retval = CY_EOK;
        if(*baud == pdesc->serial.baudrate) {
            break;
        }
        pdesc->serial.baudrate = *baud;
        if(pdesc->serial.ops.deinit) {
            pdesc->serial.ops.deinit();
        }
        if(pdesc->serial.ops.init) {
            if(!pdesc->serial.ops.init()) {
                retval = CY_ERROR;
            }
        }
    } while(0);

    return retval;
}

static int32_t __ioctl_clear_status(d100_describe_t *pdesc, void *args)
{
    pdesc->status.state = STATE_IDLE;

    return CY_EOK;
}

static int32_t __ioctl_power_on(d100_describe_t *pdesc, void *args)
{
    if(pdesc->gpio.ops.set) {
        pdesc->gpio.ops.set(true);
    }

    return CY_EOK;
}

static int32_t __ioctl_power_off(d100_describe_t *pdesc, void *args)
{
    if(pdesc->gpio.ops.set) {
        pdesc->gpio.ops.set(false);
    }

    return CY_EOK;
}
