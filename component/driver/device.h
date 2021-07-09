/**
 * @file driver/include/device.h
 *
 * Copyright (C) 2020
 *
 * device.h is free software: you can redistribute it and/or modify
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
#ifndef __DEVICE_H
#define __DEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
#undef _DEV_SECTION_PREFIX
#if defined(__linux__) || defined(_WIN32)
#define _DEV_SECTION_PREFIX
#else
#define _DEV_SECTION_PREFIX                     "."
#endif

//! @option define of the command type
#define IOCTL_USER_START                        (0X80000000)
#define IOCTL_DEVICE_POWER_ON                   (0x00001000)
#define IOCTL_DEVICE_POWER_OFF                  (0x00001001)

//! @option define the device attribute
#define DEVICE_ATTRIB_IDLE                      (0x00)
#define DEVICE_ATTRIB_START                     (0x01)
#define DEVICE_ATTRIB_POWER_OFF                 (0x0 << 4U)
#define DEVICE_ATTRIB_POWER_ON                  (0x1 << 4U)

//! @option define the method to operate the attribute
#define __device_attrib_ispower(attrib)         ((attrib & 0xF0) == DEVICE_ATTRIB_POWER_ON)
#define __device_attrib_isstart(attrib)         ((attrib & 0x0F) == DEVICE_ATTRIB_START)
#define __device_attrib_setpower(attrib, power) (attrib = (attrib & (~0xF0)) | power)
#define __device_attrib_setstart(attrib, start) (attrib = (attrib & (~0x0F)) | start)

#define DEVICE_DEFINED(dev_name, drv_name, desc) \
        device_t device_##dev_name __attribute__((used, section(_DEV_SECTION_PREFIX "dev_defined"))) \
        __attribute__((aligned(4))) = {#dev_name, #drv_name, 0, 0, NULL, desc}

/*---------- type define ----------*/
typedef struct {
    char dev_name[10];
    char drv_name[10];
    uint16_t attribute;
    uint32_t count;
    void *pdrv;
    void *pdesc;
} device_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void *device_open(char *name);
extern void device_close(device_t *dev);
extern int32_t device_write(device_t *dev, void *buf, uint32_t addition, uint32_t len);
extern int32_t device_read(device_t *dev, void *buf, uint32_t addition, uint32_t len);
extern int32_t device_ioctl(device_t *dev, uint32_t cmd, void *args);
extern int32_t device_irq_process(device_t *dev, uint32_t irq_handler, void *args, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_H */
