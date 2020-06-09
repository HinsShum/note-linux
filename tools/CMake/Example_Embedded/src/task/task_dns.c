/**
 * @file src/task/task_dns.c
 *
 * Copyright (C) 2019
 *
 * task_dns.c is free software: you can redistribute it and/or modify
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
#include "wizchip_conf.h"
#include "dns.h"
#include "printk.h"
#include "platform.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * @brief dns task function
 * @param para: task parameter
 *
 * @retval Null
 */
static void task_dns(void *para)
{
    uint8_t ip[4] = {0};
    char *ipstr = "niot.dcgc.sluan.com";
    wiz_NetInfo wiz_net_info = {0};
    int8_t retval = 0;

    ctlnetwork(CN_GET_NETINFO, (void *)&wiz_net_info);

    while(true) {
        if(retval <= 0) {
            retval = DNS_run(wiz_net_info.dns, (uint8_t *)ipstr, ip);
            if(retval > 0) {
                printk(KERN_INFO "%s --> %d.%d.%d.%d\n", ipstr, ip[0], ip[1], ip[2], ip[3]);
            } else if(retval == -1) {
                printk(KERN_WARN "%s Domain length is too long\n", ipstr);
            } else {
                printk(KERN_ERROR "DNS failed\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * @brief create DNS task
 *
 * @retval Null
 */
void task_dns_create(void)
{
    if(pdFALSE == xTaskCreate(task_dns, "TaskDNS", configMINIMAL_STACK_SIZE << 1, NULL, tskIDLE_PRIORITY + 2, NULL)) {
        printk(KERN_ERROR "Task DNS Create failed\n");
        platform_system_reset();
    }
}
