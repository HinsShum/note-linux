/**
 * @file driver\include\at24cxx.h
 *
 * Copyright (C) 2021
 *
 * at24cxx.h is free software: you can redistribute it and/or modify
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
#ifndef __AT24CXX_H
#define __AT24CXX_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "flash.h"
#include "i2c_bus.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    void (*write_protect_set)(bool enable);
    bool (*write_protect_get)(void);
    void (*cb)(void);
} at24cxx_ops_t;

typedef struct {
    uint8_t address;
    char *bus_name;
    void *bus;
    uint8_t mem_addr_counts;
    flash_info_t info;
    at24cxx_ops_t ops;
} at24cxx_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __AT24CXX_H */
