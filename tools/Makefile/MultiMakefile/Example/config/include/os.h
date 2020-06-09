/**
 * /config/os.h
 *
 * Copyright (C) 2017 HinsShum
 *
 * os.h is free software: you can redistribute it and/or modify
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
 */
#ifndef __OS_H
#define __OS_H

/*---------- includes ----------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "semphr.h"

/*---------- marco ----------*/
#define sl_malloc(x)  pvPortMalloc(x)
#define sl_free(x)    vPortFree(x)

/*---------- prototype ----------*/


#endif /* __OS_H */
