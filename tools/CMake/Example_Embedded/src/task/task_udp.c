/**
 * @file src/task/task_udp.c
 *
 * Copyright (C) 2019
 *
 * task_udp.c is free software: you can redistribute it and/or modify
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
#include "w5500_init.h"
#include "socket.h"
#include "printk.h"
#include "platform.h"
#include <string.h>

/*---------- macro ----------*/
#define SOCKET_NUMBER_UDP           (1)

/*---------- variable prototype ----------*/
extern SemaphoreHandle_t w5500_semaphore;

/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
#ifndef CONFIG_W5500_UDP_INTERRUPT
/**
 * @brief udp task function
 * @param para: task parameter
 *
 * @retval Null
 */
static void task_udp(void *para)
{
    uint8_t *pbuf = gw5500_buf;
    int16_t ret = 0;
    uint8_t ip[4] = {0};
    uint16_t port = 0;

    while(true) {
        switch(getSn_SR(SOCKET_NUMBER_UDP)) {
            case SOCK_UDP:
                vTaskDelay(pdMS_TO_TICKS(100));
                if(getSn_IR(SOCKET_NUMBER_UDP) & Sn_IR_RECV) {
                    setSn_IR(SOCKET_NUMBER_UDP, Sn_IR_RECV);
                }
                if((ret = getSn_RX_RSR(SOCKET_NUMBER_UDP)) > 0) {
                    memset(pbuf, 0, ret + 1);
                    recvfrom(SOCKET_NUMBER_UDP, pbuf, ret, ip, &port);
                    printk(KERN_INFO "Receive %s from %d.%d.%d.%d:%d\n", pbuf, ip[0], ip[1], ip[2], ip[3], port);
                    sendto(SOCKET_NUMBER_UDP, pbuf, ret, ip, port);
                }
                break;
            case SOCK_CLOSED:
                if((ret = socket(SOCKET_NUMBER_UDP, Sn_MR_UDP, 30000, 0x00)) >= 0) {
                    printk(KERN_INFO "Socket Number:%d\n", ret);
                } else {
                    printk(KERN_ERROR "Create Socket failed\n");
                }
                break;
            default: break;
        }
    }
}
#else
/**
 * @brief udp task function
 * @param para: task parameter
 *
 * @retval Null
 */
static void task_udp_interrupt(void *para)
{
    uint8_t *pbuf = gw5500_buf;
    int16_t ret = 0;
    uint8_t ip[4] = {0};
    uint16_t port = 0;

    /* SET SOCK_0 interrupt */
    wizchip_setinterruptmask(IK_SOCK_1);
    /* open socket */
    if(getSn_SR(SOCKET_NUMBER_UDP) == SOCK_CLOSED) {
        if((ret = socket(SOCKET_NUMBER_UDP, Sn_MR_UDP, 30000, 0x00)) >= 0) {
            printk(KERN_INFO "Socket Number:%d\n", ret);
        } else {
            printk(KERN_ERROR "Create Socket failed\n");
        }
    }

    while(true) {
        if(pdTRUE == xSemaphoreTake(w5500_semaphore, portMAX_DELAY)) {
            if(getSn_SR(SOCKET_NUMBER_UDP) == SOCK_UDP) {
                if(getSn_IR(SOCKET_NUMBER_UDP) & Sn_IR_RECV) {
                    setSn_IR(SOCKET_NUMBER_UDP, Sn_IR_RECV);
                }
                if((ret = getSn_RX_RSR(SOCKET_NUMBER_UDP)) > 0) {
                    memset(pbuf, 0, ret + 1);
                    recvfrom(SOCKET_NUMBER_UDP, pbuf, ret, ip, &port);
                    printk(KERN_INFO "Receive %s from %d.%d.%d.%d:%d\n", pbuf, ip[0], ip[1], ip[2], ip[3], port);
                    sendto(SOCKET_NUMBER_UDP, pbuf, ret, ip, port);
                }
            }
        }
    }
}
#endif
/**
 * @brief Create udp task
 *
 * @retval Null
 */
void task_udp_create(void)
{
#ifndef CONFIG_W5500_UDP_INTERRUPT
    if(pdFALSE == xTaskCreate(task_udp, "TaskUdp", configMINIMAL_STACK_SIZE << 1, NULL, tskIDLE_PRIORITY + 3, NULL)) {
        printk(KERN_ERROR "Task Udp Create failed\n");
        platform_system_reset();
    }
#else
    if(pdFALSE == xTaskCreate(task_udp_interrupt, "TaskUdp", configMINIMAL_STACK_SIZE << 1, NULL, tskIDLE_PRIORITY + 3, NULL)) {
        printk(KERN_ERROR "Task Udp Create failed\n");
        platform_system_reset();
    }
#endif
}
