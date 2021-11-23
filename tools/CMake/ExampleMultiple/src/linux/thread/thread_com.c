/**
 * @file src\linux\thread\thread_com.c
 *
 * Copyright (C) 2021
 *
 * thread_com.c is free software: you can redistribute it and/or modify
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
#include "rs232.h"
#include "platform.h"
#include "serial.h"
#include "config/errorno.h"
#include "config/options.h"
#include <pthread.h>
#include <stdlib.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static uint8_t _comport;
static uint8_t _buf[256];

/*---------- function ----------*/
static void *_thread_com(void *args)
{
    int32_t count = 0;

    device_ioctl(g_plat.dev.com, IOCTL_SERIAL_GET_COMPORT, &_comport);
    for(;;) {
        if(0 < (count = RS232_PollComport(_comport - 1, _buf, ARRAY_SIZE(_buf)))) {
            device_irq_process(g_plat.dev.com, 0, _buf, count);
        } else {
            __delay_ms(1);
        }
    }

    return NULL;
}

void thread_com_create(void)
{
    pthread_t id;

    if(pthread_create(&id, NULL, _thread_com, NULL) != 0) {
        __debug_error("Create thread com failed\n");
        exit(EXIT_FAILURE);
    }
}
