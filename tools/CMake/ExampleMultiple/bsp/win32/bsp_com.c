/**
 * @file bsp\win32\bsp_com.c
 *
 * Copyright (C) 2021
 *
 * bsp_com.c is free software: you can redistribute it and/or modify
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
#include "bsp_com.h"
#include "serial.h"
#include "rs232.h"
#include <stdio.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static bool bsp_init(void);
static void bsp_deinit(void);
static uint16_t bsp_write(uint8_t *pbuf, uint16_t length);

/*---------- type define ----------*/
/*---------- variable ----------*/
static serial_describe_t com = {
    .comport = 0,
    .baudrate = 115200,
    .ops.init = bsp_init,
    .ops.deinit = bsp_deinit,
    .ops.dir_change = NULL,
    .ops.write = bsp_write,
    .ops.irq_handler = NULL
};
DEVICE_DEFINED(com, serial, &com);

/*---------- function ----------*/
static bool bsp_init(void)
{
    bool retval = true;
    char mode[] = {'8', 'N', '1', 0};
    uint32_t comport = 0;
    int32_t count = 0;

    /* allow user input COM number
     */
    printf("Please input COM port:");
    count = scanf("%d", &comport);
    if(count == 1 && comport < 256) {
        com.comport = (uint8_t)comport;
    } else {
        printf("Input error, use default comport:%d\n", com.comport);
    }

    if(RS232_OpenComport(com.comport - 1, com.baudrate, mode)) {
        retval = false;
        printf("Open COM%d failed\n", com.comport);
    }

    return retval;
}

static void bsp_deinit(void)
{
    RS232_CloseComport(com.comport - 1);
}

static uint16_t bsp_write(uint8_t *pbuf, uint16_t length)
{
    RS232_SendBuf(com.comport - 1, (unsigned char *)pbuf, length);

    return length;
}
