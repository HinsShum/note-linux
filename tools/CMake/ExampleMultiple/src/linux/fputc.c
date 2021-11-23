/**
 * @file src\linux\fputc.c
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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static char _buf[1024];

/*---------- function ----------*/
static uint16_t _fputs(const char *s, uint16_t length)
{
    device_write(g_plat.dev.com, (void *)s, SERIAL_WIRTE_CHANGE_DIR_AUTOMATICALLY, length);

    return length;
}

int32_t printk(const char *fmt, ...)
{
    va_list args;
    int32_t len = 0;

    va_start(args, fmt);
    len = vsnprintf(_buf, ARRAY_SIZE(_buf), fmt, args);
    va_end(args);

    return _fputs(_buf, len);
}
