/**
 * @file /bsp/efm32lgxx/include/bsp_w5500.h
 *
 * Copyright (C) 2019
 *
 * bsp_w5500.h is free software: you can redistribute it and/or modify
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
#ifndef __BSP_W5500_H
#define __BSP_W5500_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void bsp_w5500_cs_select(void);
extern void bsp_w5500_cs_deselect(void);
extern uint8_t bsp_w5500_getbyte(void);
extern void bsp_w5500_sendbyte(uint8_t byte);
extern void bsp_w5500_reset_ctl(bool on);
extern bool bsp_w5500_init(void);
extern void bsp_w5500_deinit(void);

#endif /* __BSP_W5500_H */
