/**
 * @file components\freemodbus\port\porttimer.c
 *
 * Copyright (C) 2021
 *
 * porttimer.c is free software: you can redistribute it and/or modify
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
#include "timer.h"
#include "config/errorno.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static int32_t __xMBPortTimersIrqHanlder(uint32_t irq_handler, void *args, uint32_t len)
{
    pxMBPortCBTimerExpired();

    return CY_EOK;
}

BOOL xMBPortTimersInit(USHORT t35_50us)
{
    ULONG us = t35_50us * 50;
    ULONG ms = us / 1000UL;
    ULONG interval = 0;
    BOOL retval = false;

    if(us % 1000UL) {
        ms++;
    }
    if(!g_platform.handler.t35) {
        g_platform.handler.t35 = device_open("timer");
        assert(g_platform.handler.t35);
    }
    device_ioctl(g_platform.handler.t35, IOCTL_TIMER_SET_INTERVAL, &ms);
    device_ioctl(g_platform.handler.t35, IOCTL_TIMER_SET_IRQ_HANDLER, __xMBPortTimersIrqHanlder);

    return true;
}

void xMBPortTimersClose(void)
{
    device_close(g_platform.handler.t35);
    g_platform.handler.t35 = NULL;
}

void vMBPortTimersEnable(void)
{
    device_ioctl(g_platform.handler.t35, IOCTL_TIMER_ENABLE, NULL);
}

void vMBPortTimersDisable(void)
{
    device_ioctl(g_platform.handler.t35, IOCTL_TIMER_DISABLE, NULL);
}

void vMBPortTimersDelay(uint16_t timeout)
{
    __delay_ms(timeout);
}
