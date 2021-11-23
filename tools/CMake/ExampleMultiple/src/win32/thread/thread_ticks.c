/**
 * @file src\win32\thread\thread_ticks.c
 *
 * Copyright (C) 2021
 *
 * thread_ticks.c is free software: you can redistribute it and/or modify
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
#include "config/errorno.h"
#include "config/options.h"
#include <pthread.h>
#include <stdlib.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static uint64_t _ticks;
static pthread_mutex_t _lock;

/*---------- function ----------*/
static void _init(void)
{
    if(pthread_mutex_init(&_lock, NULL) != 0) {
        __debug_error("Create mutex for ticks failed\n");
        exit(EXIT_FAILURE);
    }
}

static void *thread_ticks_inc(void *args)
{
    _init();
    for(;;) {
        __delay_ms(1);
        pthread_mutex_lock(&_lock);
        _ticks++;
        pthread_mutex_unlock(&_lock);
    }

    return NULL;
}

uint64_t ticks_get(void)
{
    uint64_t ticks = 0;

    pthread_mutex_lock(&_lock);
    ticks = _ticks;
    pthread_mutex_unlock(&_lock);

    return ticks;
}

void thread_ticks_create(void)
{
    pthread_t id;

    if(pthread_create(&id, NULL, thread_ticks_inc, NULL) != 0) {
        __debug_error("Create thread ticks failed\n");
        exit(EXIT_FAILURE);
    }

}
