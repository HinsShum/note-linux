/**
 * @file /src/main.c
 *
 * Copyright (C) 2019
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
 */

/*---------- includes ----------*/
#include "efm32lgxx_conf.h"
#include "cpu_freq.h"
#include "platform.h"
#include "printk.h"
#include "config/include/os.h"
#include "config/include/attributes.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
#ifdef CONFIG_UDP_DEBUG
extern void task_udp_create(void);
#else
extern void task_net_create(void);
#endif
#ifdef CONFIG_W5500_NET_DHCP
extern void task_dhcp_create(void);
#endif
extern void task_dns_create(void);

/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
void deamon_task(void *para)
{
    platform_init();

    /* create some tasks */
#ifdef CONFIG_UDP_DEBUG
    task_udp_create();
#else
    task_net_create();
#endif
#ifdef CONFIG_W5500_NET_DHCP
    task_dhcp_create();
#endif
    task_dns_create();

    while(true) {
        platform_sysled_ctrl(true);
        vTaskDelay(pdMS_TO_TICKS(1000));
        platform_sysled_ctrl(false);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void _start(void)
{
    cpu_freq_config();
    platform_hw_init();

    /* Create first task */
    xTaskCreate(deamon_task, "Deamon", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    /* start task scheduler, not return */
    vTaskStartScheduler();
}

