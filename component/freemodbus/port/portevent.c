/**
 * @file components\freemodbus\port\portevent.c
 *
 * Copyright (C) 2021
 *
 * portevent.c is free software: you can redistribute it and/or modify
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
#include "mb.h"
#include "mbport.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static eMBEventType queue_event;
static BOOL event_in_queue;

/*---------- function ----------*/
BOOL xMBPortEventInit(void)
{
    event_in_queue = false;

    return true;
}

BOOL xMBPortEventPost(eMBEventType event)
{
    queue_event = event;
    event_in_queue = true;

    return true;
}

BOOL xMBPortEventGet(eMBEventType *pevent)
{
    BOOL retval = event_in_queue;

    if(event_in_queue) {
        event_in_queue = false;
        *pevent = queue_event;
    }

    return retval;
}
