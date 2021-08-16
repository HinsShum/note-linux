/**
 * @file src/task/task_dhcp.c
 *
 * Copyright (C) 2019
 *
 * task_dhcp.c is free software: you can redistribute it and/or modify
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
#include "dhcp.h"
#include "printk.h"
#include "platform.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * @brief dhcp task function
 * @param para: task parameter
 *
 * @retval Null
 */
static void task_dhcp(void *para)
{
    while(true) {
        vTaskDelay(pdMS_TO_TICKS(getDHCPLeasetime() * 1000));
        DHCP_run();
    }
}

/**
 * @brief Create dhcp task
 *
 * @retval Null
 */
void task_dhcp_create(void)
{
    if(pdFALSE == xTaskCreate(task_dhcp, "TaskDHCP", configMINIMAL_STACK_SIZE << 1, NULL, tskIDLE_PRIORITY + 2, NULL)) {
        printk(KERN_ERROR "Task DHCP Create failed\n");
        platform_system_reset();
    }
}
