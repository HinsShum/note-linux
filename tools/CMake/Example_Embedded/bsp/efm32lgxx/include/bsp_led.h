/**
 * /bsp/efm32lgxx/bsp_led.h
 *
 * Copyright (C) 2018 HinsShum
 *
 * bsp_led.h is free software: you can redistribute it and/or modify
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
 */
#ifndef __BSP_LED_H
#define __BSP_LED_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- marco ----------*/
#define BSP_LED_SYS         (0x00)

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern bool bsp_led_init(void);
extern void bsp_led_on(uint8_t led_id);
extern void bsp_led_off(uint8_t led_id);
extern void bsp_led_revert(uint8_t led_id);

#endif /* __BSP_LED_H */
