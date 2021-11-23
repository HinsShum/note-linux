/**
 * @file src\cy001\main.c
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
#include "cpu.h"
#include "device.h"
#include "simplefifo.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static void _jump_to_app(void)
{
    void (*jump)(void) = NULL;
    uint32_t app_addr = *(__IO uint32_t *)(CONFIG_APP_LOCATION_BASE + 4);
    uint32_t sp_addr = *(__IO uint32_t *)CONFIG_APP_LOCATION_BASE;

    if(sp_addr > CONFIG_SRAM_BASE && sp_addr <= CONFIG_SRAM_END) {
        /* close all peripherals */
        device_close(g_plat.dev.com);
        device_close(g_plat.dev.embed_flash);
        device_close(g_plat.dev.backup_flash);
        simplefifo_delete(g_plat.dev.fifo);
        __disable_irq();
        cpu_restore();
        jump = (void (*)(void))app_addr;
        jump();
    }
    debug_warn("Stack pointer address is invalid, the app may be corrupted\r\n");
}

int main(void)
{
    int32_t result = STRATEGY_ERR_OK;

    cpu_config();
    plat_init();
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
        /* force jump to app */
        result = STRATEGY_ERR_JUMP;
    } while(0);
    if(result == STRATEGY_ERR_JUMP) {
        _jump_to_app();
    }
    /* print useless info before reset system
     * avoid the effective info can not be printed
     */
    debug_info("    \r\n");
    /* reset system */
    NVIC_SystemReset();

    return 0;
}
