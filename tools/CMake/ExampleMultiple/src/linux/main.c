/**
 * @file src\linux\main.c
 *
 * Copyright (C) 2021
 *
 * main.c is free software: you can redistribute it and/or modify
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

/*---------- includes ----------*/
#include "platform.h"
#include "strategy.h"
#include "firmware.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void thread_ticks_create(void);
extern void thread_com_create(void);

/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
int32_t main(void)
{
    int32_t result = STRATEGY_ERR_OK;

    plat_init();
    /* create thread */
    thread_ticks_create();
    thread_com_create();
reboot:
    /* start strategy */
    result = strategy_process();
    do {
        if(result == STRATEGY_ERR_REBOOT) {
            debug_message("Reboot\r\n");
            break;
        } else if(result == STRATEGY_ERR_JUMP) {
            debug_message("Jump\r\n");
            break;
        }
        if(firmware_get_updated_flag() == false) {
            if(firmware_update() != CY_EOK) {
                result = STRATEGY_ERR_FAILED;
                break;
            }
        }
        result = STRATEGY_ERR_JUMP;
    } while(0);
    if(result == STRATEGY_ERR_JUMP) {
        debug_message("Jump to App\r\n");
    } else {
        goto reboot;
    }

    return 0;
}
