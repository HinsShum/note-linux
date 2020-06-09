/**
 * @file src/include/platform.h
 *
 * Copyright (C) 2019
 *
 * platform.h is free software: you can redistribute it and/or modify
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
#ifndef __PLATFORM_H
#define __PLATFORM_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void platform_hw_init(void);
extern void platform_hw_deinit(void);
extern bool platform_init(void);
extern bool platform_get_peripherals_init_state(void);
extern void platform_sysled_ctrl(bool on);
extern void __error_handler(char *file, int line);

/**
 * @brief reset the mcu by the NVIC controller
 *
 * @retval Null
 */
static inline void platform_system_reset(void)
{
    NVIC_SystemReset();
}

#endif /* __PLATFORM_H */
