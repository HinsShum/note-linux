/**
 * @file src\cy001\fputc.c
 *
 * Copyright (C) 2021
 *
 * fputc.c is free software: you can redistribute it and/or modify
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
#include "config/options.h"
#include <stdio.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
#if defined(__ARMCC_VERSION)
int fputc(int ch, FILE *p)
{
    uint8_t c = (uint8_t)ch;

    device_write(g_plat.dev.com, &c, SERIAL_WIRTE_CHANGE_DIR_AUTOMATICALLY, sizeof(c));

    return ch;
}
#else
int _write(int fd, char *buf, int size)
{
    device_write(g_plat.dev.com, (void *)buf, SERIAL_WIRTE_CHANGE_DIR_AUTOMATICALLY, size);

    return size;
}
#endif
