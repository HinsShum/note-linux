/**
 * @file components\freemodbus\port\port.h
 *
 * Copyright (C) 2021
 *
 * port.h is free software: you can redistribute it and/or modify
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
#ifndef __FREEMODBUS_PORT_H
#define __FREEMODBUS_PORT_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "config/options.h"

/*---------- macro ----------*/
#define INLINE
#define PR_BEGIN_EXTERN_C                           extern "C" {
#define PR_END_EXTERN_C                             }

#define ENTER_CRITICAL_SECTION()
#define EXIT_CRITICAL_SECTION()

#define MB_PORT_HAS_CLOSE                           (0)

#ifndef TRUE
#define TRUE                                        true
#endif

#ifndef FALSE
#define FALSE                                       false
#endif

/*---------- type define ----------*/
#ifndef __WIN32
typedef bool BOOL;
typedef uint8_t UCHAR;
typedef int8_t CHAR;
typedef uint16_t USHORT;
typedef int16_t SHORT;
typedef uint32_t ULONG;
typedef int32_t LONG;
#else
#define __IO
#endif

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
#endif /* __FREEMODBUS_PORT_H */

