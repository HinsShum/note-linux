/**
 * @file portable.h
 *
 * Copyright (C) 2020
 *
 * portable.h is free software: you can redistribute it and/or modify
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
 */
#ifndef __HEAP_PORTABLE_H
#define __HEAP_PORTABLE_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
#ifndef PORT_BYTE_ALIGNMENT
#define PORT_BYTE_ALIGNMENT                     (8)
#endif

#if PORT_BYTE_ALIGNMENT == 32
    #define PORT_BYTE_ALIGNMENT_MASK            (0x001f)
#endif

#if PORT_BYTE_ALIGNMENT == 16
    #define PORT_BYTE_ALIGNMENT_MASK            (0x000f)
#endif

#if PORT_BYTE_ALIGNMENT == 8
    #define PORT_BYTE_ALIGNMENT_MASK            (0x0007)
#endif

#if PORT_BYTE_ALIGNMENT == 4
    #define PORT_BYTE_ALIGNMENT_MASK            (0x0003)
#endif

#if PORT_BYTE_ALIGNMENT == 2
    #define PORT_BYTE_ALIGNMENT_MASK            (0x0001)
#endif

#if PORT_BYTE_ALIGNMENT == 1
    #define PORT_BYTE_ALIGNMENT_MASK            (0x0000)
#endif

#ifndef PORT_TOTAL_HEAP_SIZE
#define PORT_TOTAL_HEAP_SIZE                    ((size_t)100 * 1024)
#endif

#ifndef PORT_HEAP_LOCK
#define PORT_HEAP_LOCK()
#endif

#ifndef PORT_HEAP_UNLOCK
#define PORT_HEAP_UNLOCK()
#endif

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
#endif /* __HEAP_PORTABLE_H */
