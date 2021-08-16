/**
 * /bsp/efm32lgxx/bsp_led.c
 *
 * Copyright (C) 2018 HinsShum
 *
 * bsp_led.c is free software: you can redistribute it and/or modify
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

/*---------- includes ----------*/
#include "bsp_led.h"
#include "efm32lgxx_conf.h"

/*---------- marco ----------*/
#define LED_SYS_PORT        (gpioPortA)
#define LED_SYS_PIN         (13)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
bool bsp_led_init(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);
    /* GPIOF_PIN9 */
    GPIO_PinModeSet(LED_SYS_PORT, LED_SYS_PIN, gpioModePushPull, 0);

    return true;
}

void bsp_led_on(uint8_t led_id)
{
    switch(led_id) {
        case BSP_LED_SYS : GPIO_PinOutSet(LED_SYS_PORT, LED_SYS_PIN); break;
        default : break;
    }
}

void bsp_led_off(uint8_t led_id)
{
    switch(led_id) {
        case BSP_LED_SYS : GPIO_PinOutClear(LED_SYS_PORT, LED_SYS_PIN); break;
        default : break;
    }
}

void bsp_led_revert(uint8_t led_id)
{
    switch(led_id) {
        case BSP_LED_SYS : GPIO_PinOutToggle(LED_SYS_PORT, LED_SYS_PIN); break;
        default : break;
    }
}

