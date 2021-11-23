/**
 * @file driver/driver.c
 *
 * Copyright (C) 2021
 *
 * driver.c is free software: you can redistribute it and/or modify
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
#include "driver.h"
#include "device.h"
#include "config/errorno.h"
#include <string.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
int32_t driver_search_device(void)
{
#if defined(__linux__) || defined(_WIN32)
    extern driver_t __start_drv_defined;
    extern driver_t __stop_drv_defined;
    extern device_t __start_dev_defined;
    extern device_t __stop_dev_defined;
    driver_t *__begin_drv = &__start_drv_defined;
    driver_t *__end_drv = &__stop_drv_defined;
    device_t *__begin_dev = &__start_dev_defined;
    device_t *__end_dev = &__stop_dev_defined;
#elif defined(__ARMCC_VERSION)
    extern driver_t Image$$DRV_REGION$$Base[];
    extern driver_t Image$$DRV_REGION$$Limit[];
    extern device_t Image$$DEV_REGION$$Base[];
    extern device_t Image$$DEV_REGION$$Limit[];
    driver_t *__begin_drv = Image$$DRV_REGION$$Base;
    driver_t *__end_drv = Image$$DRV_REGION$$Limit;
    device_t *__begin_dev = Image$$DEV_REGION$$Base;
    device_t *__end_dev = Image$$DEV_REGION$$Limit;
#else
    extern driver_t _sdrv;
    extern driver_t _edrv;
    extern device_t _sdev;
    extern device_t _edev;
    driver_t *__begin_drv = &_sdrv;
    driver_t *__end_drv = &_edrv;
    device_t *__begin_dev = &_sdev;
    device_t *__end_dev = &_edev;
#endif
    driver_t *drv = NULL;
    device_t *dev = NULL;

    for(dev = __begin_dev; dev < __end_dev; ++dev) {
        for(drv = __begin_drv; drv < __end_drv; ++drv) {
            if(!strncmp(dev->drv_name, drv->drv_name, sizeof(drv->drv_name))) {
                dev->pdrv = drv;
            }
        }
    }

    return CY_EOK;
}
