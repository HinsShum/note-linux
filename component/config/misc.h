/**
 * @file config/misc.h
 *
 * Copyright (C) 2021
 *
 * misc.h is free software: you can redistribute it and/or modify
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
#ifndef __MISC_H
#define __MISC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define container_of(ptr, type, member) (                       \
        (type *)((char *)ptr - offsetof(type, member)))
#elif defined(__GNUC__)
#define container_of(ptr, type, member) ({                      \
        const __typeof(((type *)0)->member) *__mptr = (ptr);    \
        (type *)((char *)__mptr - offsetof(type, member));})
#elif defined(_MSC_VER)
#define container_of(ptr, type, member) (                       \
        (type *)((char *)ptr - offsetof(type, member)))
#else
#error "No container_of defined in this compiler"
#endif

/* format to string
 */
#define _STRING(x)                      #x              /*<< only format alphabet as string */
#define STRING(x)                       _STRING(x)      /*<< format alphabet or digit as string */

#define ARRAY_SIZE(x)                   (sizeof(x) / sizeof((x)[0]))
#define FIELD_SIZEOF(t, f)              (sizeof(((t *)0)->f))

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __MISC_H */
