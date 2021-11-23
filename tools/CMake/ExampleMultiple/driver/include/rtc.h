/**
 * @file driver\include\rtc.h
 *
 * Copyright (C) 2021
 *
 * rtc.h is free software: you can redistribute it and/or modify
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
#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    bool (*set_utc)(int32_t utc);
    int32_t (*get_utc)(void);
} rtc_describe_t;

/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __RTC_H */
