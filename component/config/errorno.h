/**
 * @file /config/errorno.h
 *
 * Copyright (C) 2020
 *
 * errorno.h is free software: you can redistribute it and/or modify
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
#ifndef __ERRORNO_H
#define __ERRORNO_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
#define CY_OK                   (0)
#define CY_EOK                  (0)
#define CY_ERROR                (-1)
#define CY_E_POINT_NONE         (-2)
#define CY_E_TIME_OUT           (-3)
#define CY_E_BUSY               (-4)
#define CY_E_NO_MEMORY          (-5)
#define CY_E_WRONG_ARGS         (-6)
#define CY_E_WRONG_CRC          (-7)

#define CY_E_DEVICE_IDEL        (-31)
#define CY_E_DEVICE_POWER_OFF   (-32)

#define CY_E_USER(x)            (x - 100)

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __ERRORNO_H */
