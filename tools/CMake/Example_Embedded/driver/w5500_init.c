/**
 * @file w5500_init.c
 *
 * Copyright (C) 2019
 *
 * w5500_init.c is free software: you can redistribute it and/or modify
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
#include "bsp.h"
#include "bsp_w5500.h"
#include "w5500_init.h"
#include "dhcp.h"
#include "dns.h"
#include "config/include/errorno.h"
#include "config/include/os.h"
#include "printk.h"
#include <string.h>

/*---------- macro ----------*/
#define DHCP_SOCKET_NUMBER          (7)
#define DNS_SOCKET_NUMBER           (0)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
uint8_t gw5500_buf[2048] = {0};
SemaphoreHandle_t w5500_semaphore = NULL;
static SemaphoreHandle_t lock = NULL;

/*---------- function ----------*/
/**
 * @brief use dhcp assign ip automatic
 *
 * @retval Null
 */
static void w5500_dhcp_ip_assign(void)
{
    wiz_NetInfo wiz_net_info = {0};

    ctlnetwork(CN_GET_NETINFO, (void *)&wiz_net_info);
    getIPfromDHCP(wiz_net_info.ip);
    getGWfromDHCP(wiz_net_info.gw);
    getSNfromDHCP(wiz_net_info.sn);
    getDNSfromDHCP(wiz_net_info.dns);
    wiz_net_info.dhcp = NETINFO_DHCP;

    ctlnetwork(CN_SET_NETINFO, (void *)&wiz_net_info);
    printk(KERN_INFO "DHCP LEASED TIME:%ld Sec\n", getDHCPLeasetime());
}

/**
 * @brief Print Warnning information when conflict ip
 *
 * @retval Null
 */
static void w5500_dhcp_ip_conflict(void)
{
    printk(KERN_WARN "W5500 Conflict IP from DHCP\n");
}

/**
 * @brief initialize wizchip phy link
 *
 * @retval true  --> initialize success
 *         false --> initialize failed
 */
static bool w5500_wizchip_phylink_init(void)
{
    uint8_t w5500_memsize[2][8] =  {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
    uint8_t tmp = 0;
    bool retval = true;

    if(ctlwizchip(CW_INIT_WIZCHIP, (void *)w5500_memsize) != -1) {
        int8_t err_cnt = 3;
        /* PHY link status check */
        do {
            if(ctlwizchip(CW_GET_PHYLINK, (void *)&tmp) == -1) {
                printk(KERN_WARN "Unkown PHY Link status\n");
            }
            if(tmp == PHY_LINK_ON) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        } while(--err_cnt > 0);
        if(err_cnt <= 0) {
            printk(KERN_ERROR "W5500 get PhyLink falied\n");
            retval = false;
        }
    } else {
        printk(KERN_ERROR "W5500 WIZCHIP SOCKET initialize failed\n");
        retval = false;
    }

    return retval;
}

/**
 * @brief initialize wizchip network.
 * @param wiz_net_info: net information(eg.mac ip gaw dns)
 *        dhcp: W5500_NET_STATIC: not use dhcp, use static ip
 *              W5500_NET_DHCP: use dhcp assign ip.
 *
 * @retval true: initialize network success.
 *         false: initialize network failed.
 */
static bool w5500_wizchip_network_init(wiz_NetInfo wiz_net_info, bool dhcp)
{
    bool retval = true;
    uint8_t tmpstr[6] = {0};
    int8_t err_cnt = 10;

    if(!dhcp) {
        ctlnetwork(CN_SET_NETINFO, (void *)&wiz_net_info);
    } else {
        setSHAR(wiz_net_info.mac);
        DHCP_init(DHCP_SOCKET_NUMBER, gw5500_buf);
        reg_dhcp_cbfunc(w5500_dhcp_ip_assign, w5500_dhcp_ip_assign, w5500_dhcp_ip_conflict);
        do {
            if(DHCP_IP_LEASED == DHCP_run()) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        } while(--err_cnt > 0);
        if(err_cnt <= 0) {
            printk(KERN_ERROR "DHCP alloc ip failed\n");
            retval = false;
        }
    }
    if(retval == true) {
        memset((void *)&wiz_net_info, 0, sizeof(wiz_NetInfo));
        ctlnetwork(CN_GET_NETINFO, (void *)&wiz_net_info);
        /* display network information */
        ctlwizchip(CW_GET_ID, (void *)tmpstr);
        printk(KERN_INFO "=== %s NET CONF ===\n", (char *)tmpstr);
        printk(KERN_INFO "MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", wiz_net_info.mac[0], wiz_net_info.mac[1],
                                                                 wiz_net_info.mac[2], wiz_net_info.mac[3],
                                                                 wiz_net_info.mac[4], wiz_net_info.mac[5]);
        printk(KERN_INFO "SIP: %d.%d.%d.%d\r\n", wiz_net_info.ip[0], wiz_net_info.ip[1], wiz_net_info.ip[2], wiz_net_info.ip[3]);
        printk(KERN_INFO "GAR: %d.%d.%d.%d\r\n", wiz_net_info.gw[0], wiz_net_info.gw[1], wiz_net_info.gw[2], wiz_net_info.gw[3]);
        printk(KERN_INFO "SUB: %d.%d.%d.%d\r\n", wiz_net_info.sn[0], wiz_net_info.sn[1], wiz_net_info.sn[2], wiz_net_info.sn[3]);
        printk(KERN_INFO "DNS: %d.%d.%d.%d\r\n", wiz_net_info.dns[0], wiz_net_info.dns[1], wiz_net_info.dns[2], wiz_net_info.dns[3]);
        printk(KERN_INFO "======================\n");
        /* initialize dns */
        DNS_init(DNS_SOCKET_NUMBER, gw5500_buf);
    }

    return retval;
}

/**
 * @brief get w5500 driver lock
 *
 * @retval Null
 */
static void w5500_lock(void)
{
    xSemaphoreTake(lock, portMAX_DELAY);
}

/**
 * @brief give w5500 driver lock
 *
 * @retval Null
 */
static void w5500_unlock(void)
{
    xSemaphoreGive(lock);
}

/**
 * @brief initialize w5500 phy link, assign ip for w5500
 * @param wiz_net_info: net information(eg.mac ip gaw dns)
 *        dhcp: W5500_NET_STATIC: not use dhcp, use static ip
 *              W5500_NET_DHCP: use dhcp assign ip.
 *
 * @retval true: initialize network success.
 *         false: initialize network failed.
 */
bool w5500_init(wiz_NetInfo wiz_net_info, bool dhcp)
{
    bool retval = false;

    /* reset w5500 */
    bsp_w5500_reset_ctl(false);
    bsp_udelay(800);
    bsp_w5500_reset_ctl(true);
    bsp_udelay(800);
    /* register callback function */
    reg_wizchip_cris_cbfunc(w5500_lock, w5500_unlock);
    reg_wizchip_cs_cbfunc(bsp_w5500_cs_select, bsp_w5500_cs_deselect);
    reg_wizchip_spi_cbfunc(bsp_w5500_getbyte, bsp_w5500_sendbyte);

    if((w5500_semaphore = xSemaphoreCreateCounting(5, 0)) != NULL &&
       (lock = xSemaphoreCreateMutex()) != NULL) {
        if(true == w5500_wizchip_phylink_init() && true == w5500_wizchip_network_init(wiz_net_info, dhcp)) {
            retval = true;
        }
    }
    /* release semaphore */
    if(retval != true) {
        if(w5500_semaphore != NULL) {
            vSemaphoreDelete(w5500_semaphore);
            w5500_semaphore = NULL;
        }
        if(lock != NULL) {
            vSemaphoreDelete(lock);
            lock = NULL;
        }
        printk("Create semaphore for w5500 driver failed\n");
    }

    return retval;
}

/**
 * @brief w5500 interrupt isr server
 *
 * @retval Null
 */
void w5500_interrupt_handler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if(w5500_semaphore != NULL) {
        xSemaphoreGiveFromISR(w5500_semaphore, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken != pdFALSE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

