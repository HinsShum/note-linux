/**
 * @file APP\driver\buzzer.c
 *
 * Copyright (C) 2021
 *
 * buzzer.c is free software: you can redistribute it and/or modify
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
#include "buzzer.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t buzzer_open(driver_t **pdrv);
static void buzzer_close(driver_t **pdrv);
static int32_t buzzer_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(buzzer, buzzer_open, buzzer_close, NULL, NULL, buzzer_ioctl, NULL);

/*---------- function ----------*/
static int32_t buzzer_open(driver_t **pdrv)
{
    buzzer_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        retval = (pdesc->init() ? CY_EOK : CY_ERROR);
    }

    return retval;
}

static void buzzer_close(driver_t **pdrv)
{
    buzzer_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

static int32_t buzzer_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    buzzer_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;
    buzzer_cycle_t *cycle = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_BUZZER_ON:
            if(pdesc && pdesc->ctrl) {
                pdesc->ctrl(true);
            }
            break;
        case IOCTL_BUZZER_OFF:
            if(pdesc && pdesc->ctrl) {
                pdesc->ctrl(false);
                pdesc->cycle_count = 0;
                pdesc->cycle_time = 0;
            }
            break;
        case IOCTL_BUZZER_TOGGLE:
            if(pdesc && pdesc->toggle) {
                pdesc->toggle();
                if(pdesc->cycle_count != 0 && pdesc->cycle_count != BUZZER_CYCLE_COUNT_MAX) {
                    pdesc->cycle_count--;
                }
            }
            break;
        case IOCTL_BUZZER_SET_CYCLE:
            if(pdesc && args) {
                cycle = (buzzer_cycle_t *)args;
                pdesc->cycle_time = cycle->cycle_time;
                pdesc->cycle_count = cycle->cycle_count;
            }
            break;
        case IOCTL_BUZZER_GET_CYCLE:
            if(pdesc && args) {
                cycle = (buzzer_cycle_t *)args;
                cycle->cycle_time = pdesc->cycle_time;
                cycle->cycle_count = pdesc->cycle_count;
            }
            break;
        default:
            retval = CY_E_WRONG_ARGS;
            break;
    }

    return retval;
}
