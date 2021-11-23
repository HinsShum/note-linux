/**
 * @file ymodem\src\ymodem.c
 *
 * Copyright (C) 2021
 *
 * ymodem.c is free software: you can redistribute it and/or modify
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
#include "ymodem.h"
#include "ymodem_crc.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#ifdef YMODEM_CONFIG_FILE
#include YMODEM_CONFIG_FILE
#endif

/*---------- macro ----------*/
#ifndef YMODEM_SAVE_FILE_NAME
#define YMODEM_SAVE_FILE_NAME                   (0)
#endif
#ifndef YMODEM_RECV_TIMEOUT
#define YMODEM_RECV_TIMEOUT                     (100)
#endif
#ifndef YMODEM_ERR_MAX_COUNT
#define YMODEM_ERR_MAX_COUNT                    (50)
#endif
#ifndef YMODEM_FILENAME_SIZE
#define YMODEM_FILENAME_SIZE                    (30)
#endif

/* frame type define
 */
#define SOH                                     (0x01)
#define STX                                     (0x02)
#define EOT                                     (0x04)
#define ACK                                     (0x06)
#define NAK                                     (0x15)
#define CAN                                     (0x18)
#define C                                       (0x43)

/* frame recv state
 */
#define YMODEM_RECV_STATE_OK                    (0)
#define YMODEM_RECV_STATE_TIMEOUT               (1)
#define YMODEM_RECV_STATE_INDEX_ERR             (2)
#define YMODEM_RECV_STATE_FRAME_ERR             (3)
#define YMODEM_RECV_STATE_CANCEL                (4)
#define YMODEM_RECV_STATE_END                   (5)
#define YMODEM_RECV_STATE_STOP                  (6)

/* frame length
 */
#define YMODEM_SOH_FRAME_LENGTH                 (128)
#define YMODEM_STX_FRAME_LENGTH                 (1024)

#define YMODEM_FRAME_HEADER_SIZE                (3)
#define YMODEM_FRAME_TRAILER_SIZE               (2)
#define YMODEM_FRAME_OVERHEAD                   (YMODEM_FRAME_HEADER_SIZE + YMODEM_FRAME_TRAILER_SIZE)

#undef ARRAY_SIZE
#define ARRAY_SIZE(x)                           (sizeof(x) / sizeof(x[0]))

/*---------- variable prototype ----------*/
/*---------- type define ----------*/
typedef struct {
    uint32_t fsize;
#if YMODEM_SAVE_FILE_NAME
    uint8_t fname[YMODEM_FILENAME_SIZE];
#endif
    uint32_t recv_counts;
    uint32_t recv_size;
    uint32_t frame_length;
    uint32_t errs;
    bool cancel;
    bool end;
    ymodem_context_t *ctx;
} ymodem_t;

typedef struct {
    uint8_t type;
    uint8_t index;
    uint8_t index_inverse;
    uint8_t data[0];
} frame_t;

typedef int32_t (*_state_cb_t)(ymodem_t *pymodem);
typedef struct {
    int32_t state;
    _state_cb_t cb;
} _state_t;


/*---------- function prototype ----------*/
static int32_t _state_stop(ymodem_t *pymodem);
static int32_t _state_cancel(ymodem_t *pymodem);
static int32_t _state_timeout(ymodem_t *pymodem);
static int32_t _state_ok(ymodem_t *pymodem);
static int32_t _state_end(ymodem_t *pymodem);

/*---------- variable ----------*/
static ymodem_t _ymodem;
static _state_t _state_array[] = {
    {YMODEM_RECV_STATE_OK, _state_ok},
    {YMODEM_RECV_STATE_TIMEOUT, _state_timeout},
    {YMODEM_RECV_STATE_INDEX_ERR, _state_timeout},
    {YMODEM_RECV_STATE_FRAME_ERR, _state_timeout},
    {YMODEM_RECV_STATE_CANCEL, _state_cancel},
    {YMODEM_RECV_STATE_END, _state_end},
    {YMODEM_RECV_STATE_STOP, _state_stop}
};

/*---------- function ----------*/
static void _reset(ymodem_t *pymodem)
{
    pymodem->errs = 0;
    pymodem->recv_counts = 0;
    pymodem->recv_size = 0;
    pymodem->frame_length = 0;
    pymodem->fsize = 0;
}

static void _force_cancel(ymodem_context_t *ctx)
{
    ctx->putc((uint8_t)CAN);
    ctx->putc((uint8_t)CAN);
}

static int32_t _recv_frame(ymodem_t *pymodem)
{
    uint8_t ch = 0;
    uint32_t data_length = 0;
    int32_t retval = YMODEM_RECV_STATE_OK;
    frame_t *frame = NULL;

    do {
        pymodem->frame_length = 0;
        if(!pymodem->ctx->get_char(&ch, YMODEM_RECV_TIMEOUT)) {
            retval = YMODEM_RECV_STATE_TIMEOUT;
            break;
        }
        ch = toupper(ch);
        if(ch == CAN) {
            if(!pymodem->cancel) {
                retval = YMODEM_RECV_STATE_CANCEL;
            } else {
                retval = YMODEM_RECV_STATE_STOP;
            }
            pymodem->cancel = !pymodem->cancel;
            break;
        }
        pymodem->cancel = false;
        if(ch == EOT) {
            retval = YMODEM_RECV_STATE_END;
            pymodem->end = !pymodem->end;
            break;
        }
        pymodem->end = false;
        if(ch == SOH) {
            data_length = YMODEM_SOH_FRAME_LENGTH;
        } else if(ch == STX) {
            data_length = YMODEM_STX_FRAME_LENGTH;
        } else {
            retval = YMODEM_RECV_STATE_STOP;
            break;
        }
        if((data_length + YMODEM_FRAME_OVERHEAD) > pymodem->ctx->size) {
            _force_cancel(pymodem->ctx);
            retval = YMODEM_RECV_STATE_STOP;
            break;
        }
        pymodem->ctx->pbuf[pymodem->frame_length++] = ch;
        while(pymodem->frame_length < (data_length + YMODEM_FRAME_OVERHEAD)) {
            if(!pymodem->ctx->get_char(&ch, YMODEM_RECV_TIMEOUT)) {
                retval = YMODEM_RECV_STATE_TIMEOUT;
                break;
            }
            pymodem->ctx->pbuf[pymodem->frame_length++] = ch;
        }
        if(retval != YMODEM_RECV_STATE_OK) {
            break;
        }
        frame = (frame_t *)pymodem->ctx->pbuf;
        if(frame->index != ((frame->index_inverse ^ 0xFF) & 0xFF)) {
            retval = YMODEM_RECV_STATE_INDEX_ERR;
            break;
        }
        /* calc crc16-xmodem */
        if(ymodem_crc16_xmodem(frame->data, data_length + YMODEM_FRAME_TRAILER_SIZE) != 0) {
            retval = YMODEM_RECV_STATE_FRAME_ERR;
            break;
        }
        pymodem->frame_length = data_length;
    } while(0);

    return retval;
}

static int32_t _state_stop(ymodem_t *pymodem)
{
    return YMODEM_ERR_FAILED;
}

static int32_t _state_cancel(ymodem_t *pymodem)
{
    return YMODEM_ERR_OK;
}

static int32_t _state_timeout(ymodem_t *pymodem)
{
    int32_t retval = YMODEM_ERR_OK;

    if(!pymodem->recv_counts && !pymodem->end) {
        pymodem->ctx->putc(C);
    } else {
        if(++pymodem->errs > YMODEM_ERR_MAX_COUNT) {
            retval = YMODEM_ERR_TIMEOUT;
            _force_cancel(pymodem->ctx);
        } else {
            pymodem->ctx->putc(NAK);
        }
    }

    return retval;
}

static int32_t _state_ok(ymodem_t *pymodem)
{
    int32_t retval = YMODEM_ERR_OK;
    frame_t *frame = (frame_t *)pymodem->ctx->pbuf;
    uint8_t *filename = NULL, *filesize = NULL;
    uint8_t *p = NULL;

    do {
        if(frame->index == 0 && pymodem->recv_counts == 1) {
            /* skip repeate start frame */
            break;
        }
        if(frame->index != (pymodem->recv_counts & 0xFF)) {
            _force_cancel(pymodem->ctx);
            retval = YMODEM_ERR_INDEX;
            break;
        }
        if(!pymodem->recv_counts) {
            filename = frame->data;
            if(!isprint(filename[0])) {
                /* no file name, transfer over */
                pymodem->ctx->putc(ACK);
                retval = YMODEM_ERR_OVER;
                break;
            }
            /* start transfer */
            _reset(pymodem);
            filesize = filename + strlen((char *)filename) + 1;
#if YMODEM_SAVE_FILE_NAME
            strncpy((char *)pymodem->fname, (char *)filename, YMODEM_FILENAME_SIZE);
            filename = pymodem->fname;
#endif
            if((p = (uint8_t *)strstr((char *)filesize, " ")) == NULL) {
                _force_cancel(pymodem->ctx);
                retval = YMODEM_ERR_FAILED;
                break;
            }
            *p = '\0';
            pymodem->fsize = strtoul((char *)filesize, NULL, 10);
            if(pymodem->ctx->file_verify) {
                if(!pymodem->ctx->file_verify((char *)filename, pymodem->fsize)) {
                    _force_cancel(pymodem->ctx);
                    retval = YMODEM_ERR_VERIFY;
                    break;
                }
            }
            pymodem->ctx->putc(ACK);
            pymodem->ctx->putc(C);
            pymodem->recv_counts++;
            break;
        }
        /* data frame */
        if((pymodem->recv_size + pymodem->frame_length) >= pymodem->fsize) {
            pymodem->frame_length = pymodem->fsize - pymodem->recv_size;
            pymodem->recv_counts = 0;
        } else {
            pymodem->recv_counts++;
        }
        if(pymodem->ctx->frame_save) {
            if(!pymodem->ctx->frame_save(pymodem->recv_size, frame->data, pymodem->frame_length)) {
                _force_cancel(pymodem->ctx);
                retval = YMODEM_ERR_SAVE;
                break;
            }
        }
        pymodem->recv_size += pymodem->frame_length;
        pymodem->ctx->putc(ACK);
    } while(0);

    return retval;
}

static int32_t _state_end(ymodem_t *pymodem)
{
    if(pymodem->end) {
        pymodem->ctx->putc(NAK);
    } else {
        pymodem->ctx->putc(ACK);
        pymodem->ctx->putc(C);
    }

    return YMODEM_ERR_OK;
}

int32_t ymodem_recv_file(void)
{
    int32_t retval = YMODEM_ERR_FAILED;
    int32_t state = YMODEM_RECV_STATE_OK;

    do {
        /* check context */
        if(!_ymodem.ctx || !_ymodem.ctx->pbuf || !_ymodem.ctx->get_char || !_ymodem.ctx->putc) {
            break;
        }
        _reset(&_ymodem);
        do {
            retval = YMODEM_ERR_UNKNOWN;
            state = _recv_frame(&_ymodem);
            for(uint8_t i = 0; i < ARRAY_SIZE(_state_array); ++i) {
                if(state == _state_array[i].state) {
                    retval = _state_array[i].cb(&_ymodem);
                    break;
                }
            }
        } while(retval == YMODEM_ERR_OK);
    } while(0);

    return retval;
}

int32_t ymodem_init(ymodem_context_t *ctx)
{
    int32_t retval = YMODEM_ERR_FAILED;

    if(ctx) {
        _ymodem.ctx = ctx;
        retval = YMODEM_ERR_OK;
    }

    return retval;
}
