/**
 * @file src/task/task_net.c
 *
 * Copyright (C) 2019
 *
 * task_net.c is free software: you can redistribute it and/or modify
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
#include "config/include/attributes.h"
#include "printk.h"
#include "platform.h"
#include "w5500_init.h"
#include "socket.h"
#include "wizchip_conf.h"
#include <string.h>

/*---------- macro ----------*/
#define TASK_NET_SOCKET_NUMBER_DATA         (1)
#define TASK_NET_SOCKET_NUMBER_OTA          (2)
#define TASK_NET_SOCKET_NUMBER_CONFIG       (3)
#define TASK_NET_SOCKET_NUMBER_ALL          (0xFF)

#define TASK_NET_LOACL_PORT_DATA            (30000)
#define TASK_NET_LOACL_PORT_OTA             (30001)
#define TASK_NET_LOACL_PORT_CONFIG          (30002)

/*---------- variable prototype ----------*/
extern SemaphoreHandle_t w5500_semaphore;

/*---------- function prototype ----------*/
static int16_t test_cbfunc(uint8_t *remote_ip, uint16_t remote_port, uint8_t *in, uint16_t in_len, uint8_t *out);

/*---------- type define ----------*/
typedef int16_t (*cbfunc_t)(uint8_t *remote_ip, uint16_t remote_port, uint8_t *in, uint16_t in_len, uint8_t *out);

typedef struct {
    uint8_t socket_number;
    uint16_t local_port;
    cbfunc_t cbfunc;
} net_info_t;

/*---------- variable ----------*/
net_info_t net_info[] = {
    { .socket_number = TASK_NET_SOCKET_NUMBER_DATA,
      .local_port = TASK_NET_LOACL_PORT_DATA,
      .cbfunc = test_cbfunc },
    { .socket_number = TASK_NET_SOCKET_NUMBER_OTA,
      .local_port = TASK_NET_LOACL_PORT_OTA,
      .cbfunc = test_cbfunc },
    { .socket_number = TASK_NET_SOCKET_NUMBER_CONFIG,
      .local_port  = TASK_NET_LOACL_PORT_CONFIG,
      .cbfunc = test_cbfunc }
};

/*---------- function ----------*/
static int16_t test_cbfunc(uint8_t *remote_ip, uint16_t remote_port, uint8_t *in, uint16_t in_len, uint8_t *out)
{
    printk(KERN_INFO "Receive %s from %d.%d.%d.%d:%d\n", in, remote_ip[0], remote_ip[1],
                                                         remote_ip[2], remote_ip[3], remote_port);

    return in_len;
}

/**
 * @brief get net info(socket_number, port) by socket_number
 * @param socket_number: socket number
 *
 * @retval net information
 */
static net_info_t *get_net_info_by_socket_number(uint8_t socket_number)
{
    net_info_t *pinfo = NULL;

    for(uint8_t i = 0; i < (sizeof(net_info) / sizeof(net_info[0])); ++i) {
        if(net_info[i].socket_number == socket_number) {
            pinfo = &net_info[i];
            break;
        }
    }

    return pinfo;
}

/**
 * @brief open scoket
 * @param socket_number: the socket number that will be opened
 *
 * @retval Null
 */
static void task_net_open_socket(uint8_t socket_number)
{
    if(socket_number == TASK_NET_SOCKET_NUMBER_ALL) {
        for(uint8_t i = 0; i < (sizeof(net_info) / sizeof(net_info[0])); ++i) {
            task_net_open_socket(net_info[i].socket_number);
        }
    } else {
        net_info_t *pinfo = get_net_info_by_socket_number(socket_number);
        if(pinfo != NULL) {
            /* open socket */
            if(getSn_SR(socket_number) == SOCK_CLOSED) {
                if(socket(socket_number, Sn_MR_UDP, pinfo->local_port, 0) >= 0) {
                    printk(KERN_INFO "Socket(%d) is opened\n", socket_number);
                } else {
                    printk(KERN_ERROR "Socket(%d) failed to be open\n", socket_number);
                }
            } else {
                printk(KERN_WARN "Socket(%d) has been opened\n", socket_number);
            }
        } else {
            printk(KERN_ERROR "!!!Socket(%d) has no information structure\n", socket_number);
        }
    }
}

/**
 * @brief close socket
 * @param socket_number: the socket number that will be closed
 *
 * @retval Null
 */
static void __unused task_net_close_socket(uint8_t socket_number)
{
    if(socket_number == TASK_NET_SOCKET_NUMBER_ALL) {
        for(uint8_t i = 0; i < (sizeof(net_info) / sizeof(net_info[0])); ++i) {
            task_net_close_socket(net_info[i].socket_number);
        }
    } else {
        close(socket_number);
        printk(KERN_INFO "Socket(%d) is closed\n", socket_number);
    }
}

/**
 * @brief set socket interrupt mask
 * @param socket_number: the socket number that will be set
 *                     interrupt mask.
 *
 * @retval Null
 */
static void task_net_set_socket_interrupt_mask(uint8_t socket_number)
{
    if(socket_number == TASK_NET_SOCKET_NUMBER_ALL) {
        for(uint8_t i = 0; i < (sizeof(net_info) / sizeof(net_info[0])); ++i) {
            task_net_set_socket_interrupt_mask(net_info[i].socket_number);
        }
    } else {
        uint16_t tmp = 0;
        /* get mask */
        tmp = (uint16_t)wizchip_getinterruptmask();
        tmp |= (((uint16_t)IK_SOCK_0) << socket_number);
        wizchip_setinterruptmask((intr_kind)tmp);
        printk(KERN_INFO "Socket(%d) interrupt mask is set\n", socket_number);
    }
}

static void task_init(void)
{
    task_net_open_socket(TASK_NET_SOCKET_NUMBER_ALL);
    task_net_set_socket_interrupt_mask(TASK_NET_SOCKET_NUMBER_ALL);
}

void task_net(void *para)
{
    uint8_t *pbuf = gw5500_buf;
    int16_t ret = 0;
    uint8_t ip[4] = {0};
    uint16_t port = 0;

    task_init();

    while(true) {
        if(pdTRUE == xSemaphoreTake(w5500_semaphore, portMAX_DELAY)) {
            for(uint8_t i = 0; i < (sizeof(net_info) / sizeof(net_info[0])); ++i) {
                if(getSn_IR(net_info[i].socket_number) & Sn_IR_RECV) {
                    setSn_IR(net_info[i].socket_number, Sn_IR_RECV);
                    if((ret = getSn_RX_RSR(net_info[i].socket_number)) > 0) {
                        /* clear buf */
                        memset(pbuf, 0, ret + 1);
                        recvfrom(net_info[i].socket_number, pbuf, ret, ip, &port);
                        ret = net_info[i].cbfunc(ip, port, (void *)pbuf, ret, (void *)pbuf);
                        if(ret > 0) {
                            sendto(net_info[i].socket_number, pbuf, ret, ip, port);
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Create Net task
 *
 * @retval Null
 */
void task_net_create(void)
{
    if(pdFALSE == xTaskCreate(task_net, "TASKNET", configMINIMAL_STACK_SIZE<< 1, NULL, tskIDLE_PRIORITY + 3, NULL)) {
        printk(KERN_ERROR "Task Net Create failed\n");
        platform_system_reset();
    }
}

