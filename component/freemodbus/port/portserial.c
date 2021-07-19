/**
 * @file components\freemodbus\port\portserial.c
 *
 * Copyright (C) 2021
 *
 * portserial.c is free software: you can redistribute it and/or modify
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
#include "mb.h"
#include "mbport.h"
#include "platform.h"
#include "serial.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static __IO UCHAR data;
static __IO BOOL data_valid = false;
static __IO BOOL __transmitted = true;

/*---------- function ----------*/
static void __xMBPortSerialStoreByte(UCHAR byte)
{
    data = byte;
    data_valid = true;
    pxMBFrameCBByteReceived();
}

static int32_t __xMBPortSerialIrqHandler(uint32_t irq_handler, void *args, uint32_t len)
{
    UCHAR *p = (UCHAR *)args;

    assert(args);
    for(uint32_t i = 0; i < len; ++i) {
        __xMBPortSerialStoreByte(p[i]);
    }

    return CY_EOK;
}

static void xMBPortSerialPoll(void)
{
    while(!__transmitted) {
        pxMBFrameCBTransmitterEmpty();
    }
}

void vMBPortSerialEnable(BOOL rx_enable, BOOL tx_enable)
{
    serial_direction_en dir = SERIAL_DIRECTION_NRX_NTX;

    assert(!rx_enable || !tx_enable);

    __transmitted = true;
    if(rx_enable) {
        dir = SERIAL_DIRECTION_RX;
    } else if(tx_enable) {
        dir = SERIAL_DIRECTION_TX;
        __transmitted = false;
    }
    device_ioctl(g_platform.handler.rs485, IOCTL_SERIAL_DIRECTION_CHOOSE, (void *)&dir);
    if(tx_enable) {
        xMBPortSerialPoll();
    }
}

BOOL xMBPortSerialInit(UCHAR com, ULONG baudrate, UCHAR data_bits, eMBParity parity)
{
    ULONG old_baudrate = 0;
    BOOL retval = false;

    /* no use formal parameters */
    (void)com;
    (void)data_bits;
    (void)parity;

    if(!g_platform.handler.rs485) {
        g_platform.handler.rs485 = device_open("rs485");
        assert(g_platform.handler.rs485);
    }
    /* register interrupt callback function */
    device_ioctl(g_platform.handler.rs485, IOCTL_SERIAL_SET_IRQ_HANDLER, __xMBPortSerialIrqHandler);
    /* get old baudrate */
    device_ioctl(g_platform.handler.rs485, IOCTL_SERIAL_GET_BAUDRATE, (void *)&old_baudrate);
    do {
        if(baudrate == old_baudrate) {
            retval = true;
            break;
        }
        /* set new baudrate and re-initialize com for modbus */
        if(CY_EOK != device_ioctl(g_platform.handler.rs485, IOCTL_SERIAL_SET_BAUDRATE, (void *)&baudrate)) {
            break;
        }
        retval = true;
    } while(0);

    return retval;
}

BOOL xMBPortSerialPutByte(CHAR byte)
{
    device_write(g_platform.handler.rs485, &byte, SERIAL_WIRTE_CHANGE_DIR_MANUAL, sizeof(byte));

    return true;
}

BOOL xMBPortSerialGetByte(CHAR *pbyte)
{
    BOOL retval = data_valid;

    if(data_valid) {
        data_valid = false;
        *pbyte = data;
    }

    return retval;
}

void vMBPortClose(void)
{
    device_close(g_platform.handler.rs485);
    g_platform.handler.rs485 = NULL;
}
