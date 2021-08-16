/**
 * @file src/platform.c
 *
 * Copyright (C) 2019
 *
 * platform.c is free software: you can redistribute it and/or modify
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

/*---------- includes ----------*/
#include "efm32lgxx_conf.h"
#include "platform.h"
#include "bsp.h"
#include "bsp_led.h"
#include "bsp_lpuart.h"
#include "bsp_w5500.h"
#include "printk.h"
#include "w5500_init.h"
#include "config/include/attributes.h"
#include "config/include/os.h"
#include <string.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
typedef bool (*init_fnc_t)(void);
typedef void (*deinit_fnc_t)(void);

/*---------- variable ----------*/
/* initlialize seuqence */
static init_fnc_t init_fnc_sequence[] = {
    bsp_init,
    bsp_systick1ms_init,
    bsp_led_init,
    bsp_lpuart_init,
    bsp_w5500_init,
    NULL
};

/* deinitlialize seuqence */
static deinit_fnc_t deinit_fnc_sequence[] = {
    bsp_deinit,
    bsp_systick_deinit,
    bsp_lpuart_deinit,
    bsp_w5500_deinit,
    NULL
};

static bool peripherals_init_state = false;

/* w5500 config information */
static wiz_NetInfo net_info = { .mac = {0x00, 0x08, 0xdc, 0x11, 0x11, 0x11},
                                .ip = {172, 15, 10, 115},
                                .sn = {255, 255, 255, 0},
                                .gw = {172, 15, 10, 1},
                                .dns = {223, 5, 5, 5},
                                .dhcp = NETINFO_STATIC };

/*---------- function ----------*/
/**
 * platform_peripherals_init() - Initlialize the peripherals that will be used.
 * @note: intrinsic void __disable_irq();
 *        intrinsic void __enable_irq();
 *
 * retval: true
 */
static bool platform_peripherals_init(void)
{
    init_fnc_t *init_fnc_ptr = NULL;

    /* disable interrupt */
    __disable_irq();
    /* init function */
    for(init_fnc_ptr = init_fnc_sequence; *init_fnc_ptr != NULL; ++init_fnc_ptr) {
        if(true != (*init_fnc_ptr)()) {
            __error_handler(__FILE__, __LINE__);
        }
    }

    /* enable interrupt */
    __enable_irq();

    return true;
}

/**
 * platform_peripherals_deinit() - Uninitialize the peripherals.
 *
 * retval: None
 */
static __unused void platform_peripherals_deinit(void)
{
    deinit_fnc_t *deinit_fnc_ptr = NULL;

    for(deinit_fnc_ptr = deinit_fnc_sequence; *deinit_fnc_ptr != NULL; ++deinit_fnc_ptr) {
        (*deinit_fnc_ptr)();
    }
}

/**
 * platform_init() - Initlialize
 *
 * retval: None
 */
void platform_hw_init(void)
{
    platform_peripherals_init();
    /*
     * must disable systick when peripherals init ok
     * it will restart when freertos startup
     */
    bsp_systick_deinit();
    peripherals_init_state = true;
}

/**
 * platform_deinit() - Uninitialize
 *
 * retval: None
 */
void __unused platform_hw_deinit(void)
{
    platform_peripherals_deinit();
    peripherals_init_state = false;
}

/**
 * platform_misc_init() - other initlialize
 *
 * retval: true
 */
static bool platform_misc_init(void)
{
    /* if use syslog function, must initialize rtc
     * before console initialize.
     */
    if(true != console_driver_init()) {
        __error_handler(__FILE__, __LINE__);
    }

    return true;
}

/**
 * @brief initialize the all drivers in this function
 *
 * @retval if the board has some drivers initialize failed, the
 *         function will return false, otherwise return true.
 */
static bool platform_driver_init(void)
{
    bool retval = true;
#ifdef CONFIG_W5500_NET_DHCP
    bool dhcp = W5500_NET_DHCP;
#else
    bool dhcp = W5500_NET_STATIC;
#endif

    /* initialize w5500 driver */
    if(true != w5500_init(net_info, dhcp)) {
        retval = false;
    }

    return retval;
}

/**
 * platform_get_peripherals_init_state() - get peripherals init
 * state.
 * @note: when peripherals init success, task can be scheduler.
 *
 * retval: peripherals init state
 */
bool platform_get_peripherals_init_state(void)
{
    return peripherals_init_state;
}

/**
 * @brief initialize msic and driver
 *
 * @retval true
 */
bool platform_init(void)
{
    platform_misc_init();
    if(true != platform_driver_init()) {
        printk(KERN_ERROR "Some drivers initialize failed, reset the system\n");
        /* delay some times for printk completed */
        bsp_mdelay(1);
        platform_system_reset();
    }

    return true;
}

/**
 * platform_sysled_ctrl() - control sys led on or off
 * @on: true: turn on
 *      false: turn off
 *
 * retval: None
 */
void platform_sysled_ctrl(bool on)
{
    on ? bsp_led_on(BSP_LED_SYS) : bsp_led_off(BSP_LED_SYS);
}

/**
 * __error_handle() - User can add his own implementation report
 * the error return state.
 * @file: error file
 * @line: error line
 *
 * retval: None
 */
void __error_handler(char *file, int line)
{
    while(true) {
    }
}
