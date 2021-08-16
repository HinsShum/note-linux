/**
 * @file src/task/task_tickhook.c
 *
 * Copyright (C) 2019
 *
 * task_tickhook.c is free software: you can redistribute it and/or modify
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
#include "config/include/os.h"
#include "bsp.h"
#include "dhcp.h"
#include "dns.h"
#include "platform.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * @brief Systick Hook function
 *
 * @retval Null
 */
void vApplicationTickHook(void)
{
    bsp_systick_inctick();
    /* every second inc the dhcp and dns tick */
    if((bsp_systick_gettick() % 1000) == 0) {
#ifdef CONFIG_W5500_NET_DHCP
        DHCP_time_handler();
#endif
        DNS_time_handler();
    }
}

