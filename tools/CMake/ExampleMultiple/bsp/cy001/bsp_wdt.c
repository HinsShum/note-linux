/**
 * @file bsp/cy001/bsp_wdt.c
 *
 * Copyright (C) 2021
 *
 * bsp_wdt.c is free software: you can redistribute it and/or modify
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
#include "bsp_wdt.h"
#include "wdt.h"
#include "stm32f1xx.h"
#include "stm32f1xx_ll_conf.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static bool bsp_init(void);
static bool bsp_feed(void);

/*---------- type define ----------*/
/*---------- variable ----------*/
static wdt_describe_t wdt = {
    .init = bsp_init,
    .deinit = NULL,
    .feed = bsp_feed
};
DEVICE_DEFINED(wdt, wdt, &wdt);

/*---------- function ----------*/
static bool bsp_init(void)
{
#ifdef NDEBUG

#endif

    return true;
}

static bool bsp_feed(void)
{
#ifdef NDEBUG

#endif

    return true;
}
