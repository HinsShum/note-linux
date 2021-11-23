/**
 * @file src\include\firmware.h
 *
 * Copyright (C) 2021
 *
 * firmware.h is free software: you can redistribute it and/or modify
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
#ifndef __FIRMWARE_H
#define __FIRMWARE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern int32_t firmware_init(void);
extern bool firmware_get_updated_flag(void);
extern void firmware_clear_update_flag(void);
extern int32_t firmware_update_info(uint32_t fsize, uint8_t *md5, bool updated);
extern int32_t firmware_update(void);

#ifdef __cplusplus
}
#endif
#endif /* __FIRMWARE_H */
