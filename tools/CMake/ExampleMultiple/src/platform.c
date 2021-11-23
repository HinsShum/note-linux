/**
 * @file src\platform.c
 *
 * Copyright (C) 2021
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
 *
 * @encoding utf-8
 */

/*---------- includes ----------*/
#include "platform.h"
#include "driver.h"
#include "device.h"
#include "simplefifo.h"
#include "firmware.h"
#include "flash.h"
#include "wdt.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void irq_process_init(void);

/*---------- type define ----------*/
/*---------- variable ----------*/
platform_describe_t g_plat;
static uint8_t _fifo_buffer[CONFIG_FIFO_SIZE];

/*---------- function ----------*/
static void _driver_init(void)
{
    /* initialize driver frame */
    driver_search_device();
    /* open device */
    g_plat.dev.embed_flash = device_open("flash1");
    assert(g_plat.dev.embed_flash);
    g_plat.dev.backup_flash = device_open("flash2");
    assert(g_plat.dev.backup_flash);
    g_plat.dev.com = device_open("com");
    assert(g_plat.dev.com);
    g_plat.dev.wdt = device_open("wdt");
    assert(g_plat.dev.wdt);
    g_plat.dev.fifo = simplefifo_new();
    assert(g_plat.dev.fifo);
    /* initialize fifo */
    simplefifo_init(g_plat.dev.fifo, _fifo_buffer, ARRAY_SIZE(_fifo_buffer));
}

static void _misc_init(void)
{
    irq_process_init();
    firmware_init();
    device_ioctl(g_plat.dev.embed_flash, IOCTL_FLASH_SET_CALLBACK, (void *)plat_wdt_feed);
    device_ioctl(g_plat.dev.backup_flash, IOCTL_FLASH_SET_CALLBACK, (void *)plat_wdt_feed);
}

int32_t plat_init(void)
{
    _driver_init();
    _misc_init();

    return CY_EOK;
}

void plat_wdt_feed(void)
{
    device_ioctl(g_plat.dev.wdt, IOCTL_WDT_FEED, NULL);
}
