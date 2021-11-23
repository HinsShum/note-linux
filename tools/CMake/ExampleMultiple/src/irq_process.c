/**
 * @file src\irq_process.c
 *
 * Copyright (C) 2021
 *
 * irq_process.c is free software: you can redistribute it and/or modify
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
#include "platform.h"
#include "serial.h"
#include "simplefifo.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
int32_t com_irq_handle(uint32_t irq_handler, void *args, uint32_t length)
{
    uint32_t avaiale_len = simplefifo_get_avaiable(g_plat.dev.fifo);

    if(length > avaiale_len) {
        length = avaiale_len;
    }

    return (int32_t)simplefifo_write(g_plat.dev.fifo, (const uint8_t *)args, length);
}

void irq_process_init(void)
{
    device_ioctl(g_plat.dev.com, IOCTL_SERIAL_SET_IRQ_HANDLER, (void *)com_irq_handle);
}
