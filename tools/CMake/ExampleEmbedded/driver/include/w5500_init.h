/**
 * @file driver/include/w5500_init.h
 *
 * Copyright (C) 2019
 *
 * w5500_init.h is free software: you can redistribute it and/or modify
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
#ifndef __W5500_INIT_H
#define __W5500_INIT_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "wizchip_conf.h"

/*---------- macro ----------*/
#define W5500_NET_STATIC        (0)
#define W5500_NET_DHCP          (1)

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
extern uint8_t gw5500_buf[];

/*---------- function prototype ----------*/
extern bool w5500_init(wiz_NetInfo wiz_net_info, bool dhcp);
extern void w5500_interrupt_handler(void);

#endif /* __W5500_INIT_H */
