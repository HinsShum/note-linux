/**
 * @file driver/device.c
 *
 * Copyright (C) 2020
 *
 * device.c is free software: you can redistribute it and/or modify
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
#include "device.h"
#include "driver.h"
#include "config/errorno.h"
#include <string.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
void *device_open(char *name)
{
#if defined(__linux__) || defined(_WIN32)
    extern device_t __start_dev_defined;
    extern device_t __stop_dev_defined;
    device_t *__begin_dev = &__start_dev_defined;
    device_t *__end_dev = &__stop_dev_defined;
#elif defined(__ARMCC_VERSION)
    extern device_t Image$$DEV_REGION$$Base[];
    extern device_t Image$$DEV_REGION$$Limit[];
    device_t *__begin_dev = Image$$DEV_REGION$$Base;
    device_t *__end_dev = Image$$DEV_REGION$$Limit;
#else
    extern device_t _sdev;
    extern device_t _edev;
    device_t *__begin_dev = &_sdev;
    device_t *__end_dev = &_edev;
#endif
    device_t *dev = NULL;
    int32_t result = CY_EOK;

    for(dev = __begin_dev; dev < __end_dev; ++dev) {
        if(!strncmp(name, dev->dev_name, sizeof(dev->dev_name))) {
            if(!__device_attrib_isstart(dev->attribute)) {
                if(dev->pdrv && ((driver_t *)dev->pdrv)->open) {
                    result = ((driver_t *)dev->pdrv)->open((driver_t **)(&dev->pdrv));
                }
                if(CY_EOK == result) {
                    __device_attrib_setstart(dev->attribute, DEVICE_ATTRIB_START);
                }
            }
            if(CY_EOK == result) {
                dev->count++;
            }
            break;
        }
    }

    return ((dev == __end_dev) || (CY_EOK != result)) ? NULL : dev;
}

void device_close(device_t *dev)
{
    if(dev->count) {
        dev->count--;
    }
    if(!dev->count) {
        if(__device_attrib_isstart(dev->attribute)) {
            if(dev && dev->pdrv && ((driver_t *)dev->pdrv)->close) {
                ((driver_t *)dev->pdrv)->close((driver_t **)(&dev->pdrv));
            }
            __device_attrib_setstart(dev->attribute, DEVICE_ATTRIB_IDLE);
        }
    }
}

int32_t device_write(device_t *dev, void *buf, uint32_t addition, uint32_t len)
{
    int32_t retval = CY_E_POINT_NONE;

    if(dev && dev->pdrv && ((driver_t *)dev->pdrv)->write) {
        retval = ((driver_t *)dev->pdrv)->write((driver_t **)(&dev->pdrv), buf, addition, len);
    }

    return retval;
}

int32_t device_read(device_t *dev, void *buf, uint32_t addition, uint32_t len)
{
    int32_t retval = CY_E_POINT_NONE;

    if(dev && dev->pdrv && ((driver_t *)dev->pdrv)->read) {
        retval = ((driver_t *)dev->pdrv)->read((driver_t **)(&dev->pdrv), buf, addition, len);
    }

    return retval;
}

int32_t device_ioctl(device_t *dev, uint32_t cmd, void *args)
{
    int32_t retval = CY_E_POINT_NONE;

    if(dev && dev->pdrv && ((driver_t *)dev->pdrv)->ioctl) {
        retval = ((driver_t *)dev->pdrv)->ioctl((driver_t **)(&dev->pdrv), cmd, args);
    }

    return retval;
}

int32_t device_irq_process(device_t *dev, uint32_t irq_handler, void *args, uint32_t len)
{
    int32_t retval = CY_E_POINT_NONE;

    if(dev && dev->pdrv && ((driver_t *)dev->pdrv)->irq_handler) {
        retval = ((driver_t *)dev->pdrv)->irq_handler((driver_t **)(&dev->pdrv),
                  irq_handler, args, len);
    }

    return retval;
}
