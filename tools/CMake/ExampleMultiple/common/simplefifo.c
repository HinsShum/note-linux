/**
 * @file common/simplefifo.c
 *
 * Copyright (C) 2021
 *
 * simplefifo.c is free software: you can redistribute it and/or modify
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
#include "simplefifo.h"
#include "config/options.h"
#include "config/errorno.h"

/*---------- macro ----------*/
#ifndef CONFIG_SIMPLEFIFO_DYNAMIC_COUNT
#define CONFIG_SIMPLEFIFO_DYNAMIC_COUNT             (5)
#endif

#define SIMPLEFIFO_MAGIC                            (0x769394AA)

#define STATE_IDLE                                  (0)
#define STATE_BUSY                                  (1)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
typedef struct {
    uint16_t max_size;
    uint16_t head;
    uint16_t tail;
    uint8_t *pdata;
} simplefifo_t;

typedef struct {
    simplefifo_t fifo;
    uint32_t magic;
    bool state;
} simplefifo_dynamic_t;

/*---------- variable ----------*/
static simplefifo_dynamic_t array[CONFIG_SIMPLEFIFO_DYNAMIC_COUNT];
static bool initialize = false;

/*---------- function ----------*/
void *simplefifo_new(void)
{
    void *handler = NULL;

    if(!initialize) {
        for(uint32_t i = 0; i < CONFIG_SIMPLEFIFO_DYNAMIC_COUNT; ++i) {
            array[i].magic = SIMPLEFIFO_MAGIC;
        }
        initialize = true;
    }
    for(uint32_t i = 0; i < CONFIG_SIMPLEFIFO_DYNAMIC_COUNT; ++i) {
        if(array[i].state == STATE_IDLE) {
            array[i].state = STATE_BUSY;
            handler = &array[i];
        }
    }

    return handler;
}

void simplefifo_delete(void *handler)
{
    simplefifo_dynamic_t *pfifo = (simplefifo_dynamic_t *)handler;

    assert(handler);
    if(pfifo->magic == SIMPLEFIFO_MAGIC) {
        pfifo->state = STATE_IDLE;
    }
}

int32_t simplefifo_init(void *handler, uint8_t *pbuf, uint16_t size)
{
    simplefifo_dynamic_t *pfifo = (simplefifo_dynamic_t *)handler;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(handler);
    if(pfifo->magic == SIMPLEFIFO_MAGIC) {
        pfifo->fifo.head = 0;
        pfifo->fifo.tail = 0;
        pfifo->fifo.max_size = size;
        pfifo->fifo.pdata = pbuf;
        retval = CY_EOK;
    }

    return retval;
}

uint16_t simplefifo_get_size(void *handler)
{
    simplefifo_dynamic_t *pfifo = (simplefifo_dynamic_t *)handler;
    uint16_t size = 0;

    assert(handler);
    if(pfifo->magic == SIMPLEFIFO_MAGIC) {
        size = (pfifo->fifo.head + pfifo->fifo.max_size - pfifo->fifo.tail) % pfifo->fifo.max_size;
    }

    return size;
}

void simplefifo_reset(void *handler)
{
    simplefifo_dynamic_t *pfifo = (simplefifo_dynamic_t *)handler;

    assert(handler);
    if(pfifo->magic == SIMPLEFIFO_MAGIC) {
        pfifo->fifo.head = 0;
        pfifo->fifo.tail = 0;
    }
}

uint16_t simplefifo_write(void *handler, const uint8_t *pbuf, uint16_t length)
{
    uint16_t full_tail = 0, len = 0;
    simplefifo_dynamic_t *pfifo = (simplefifo_dynamic_t *)handler;

    assert(handler);
    if(pfifo->magic == SIMPLEFIFO_MAGIC) {
        full_tail = (pfifo->fifo.tail + pfifo->fifo.max_size - 1) % pfifo->fifo.max_size;
        while(pfifo->fifo.head != full_tail) {
            if(len >= length) {
                break;
            }
            pfifo->fifo.pdata[pfifo->fifo.head] = pbuf[len++];
            pfifo->fifo.head = (pfifo->fifo.head + 1) % pfifo->fifo.max_size;
        }
    }

    return len;
}

uint16_t simplefifo_read(void *handler, uint8_t *pbuf, uint16_t length)
{
    uint16_t len = 0;
    simplefifo_dynamic_t *pfifo = (simplefifo_dynamic_t *)handler;

    assert(handler);
    if(pfifo->magic == SIMPLEFIFO_MAGIC) {
        while(pfifo->fifo.head != pfifo->fifo.tail) {
            if(len >= length) {
                break;
            }
            pbuf[len++] = pfifo->fifo.pdata[pfifo->fifo.tail];
            pfifo->fifo.tail = (pfifo->fifo.tail + 1) % pfifo->fifo.max_size;
        }
    }

    return len;
}

uint16_t simplefifo_get_avaiable(void *handler)
 {
     uint16_t full_tail = 0, avaiable = 0;
     simplefifo_dynamic_t *pfifo = (simplefifo_dynamic_t *)handler;

     assert(handler);
     if(pfifo->magic == SIMPLEFIFO_MAGIC) {
         full_tail = (pfifo->fifo.tail + pfifo->fifo.max_size - 1) % pfifo->fifo.max_size;
         avaiable = (full_tail + pfifo->fifo.max_size - pfifo->fifo.head) % pfifo->fifo.max_size;
     }

     return avaiable;
 }
