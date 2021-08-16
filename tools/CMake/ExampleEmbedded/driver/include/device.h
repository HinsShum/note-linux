/**
 * @file device.h
 *
 * @copyright This file creat by rensuiyi ,all right reserve!
 *
 * @author rensuyi
 *
 * @date 2013/12/21 13:29:49
 */
#ifndef __DEVICE_H__
#define __DEVICE_H__

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "config/include/os.h"

// define the device type
// tiny ,no mutex,no name
// min ,have mutex,name
// full,have device list and register and find functions
#define DEVICE_TYPE_TINY           (0x01)
#define DEVICE_TYPE_MIN            (0x02)
#define DEVICE_TYPE_FULL           (0x03)

//! @option device type
#ifndef  DEVICE_TYPE
#define  DEVICE_TYPE     DEVICE_TYPE_TINY
#endif

// define of the cmd type
#define  IOCTL_USER_START                   (0x80000000)
#define  IOCTL_DEVICE_POWER_ON              (0x00001000)
#define  IOCTL_DEVICE_POWER_OFF             (0x00001001)

// define the device attrib
#define  DEVICE_ATTRIB_IDLE              (0x0)
#define  DEVICE_ATTRIB_START             (0x1)
#define  DEVICE_ATTRIB_POWER_OFF         (0x0<<4)
#define  DEVICE_ATTRIB_POWER_ON          (0x1<<4)


#define  __device_attrib_ispower(attrib)         ((attrib&0xF0)==DEVICE_ATTRIB_POWER_ON)
#define  __device_attrib_isstart(attrib)         ((attrib&0x0F)==DEVICE_ATTRIB_START)
#define  __device_attrib_setpower(attrib,power)  (attrib = (attrib&(~0xF0))|power)
#define  __device_attrib_setstart(attrib,start)  (attrib = (attrib&(~0x0F))|start)



// the struct of the device
struct st_device
{
    #if DEVICE_TYPE == DEVICE_TYPE_FULL
    // add list struct here
    #endif
    #if DEVICE_TYPE > DEVICE_TYPE_TINY
    // mutex and name
    char name[8];
    // mutex lock;
    #endif
    uint16_t  attrib;
    int32_t (*init)(struct st_device *device);
    int32_t (*read)(struct st_device *device, void *buf, uint32_t addition, uint32_t len);
    int32_t (*write)(struct st_device *device, void *buf, uint32_t addition, uint32_t len);
    int32_t (*ioctl)(struct st_device *device, uint32_t cmd, void *arg);
    void *user;

};

typedef int32_t (* device_isr_handler)(struct st_device* pdev, uint32_t isr, void* arg);

extern int32_t device_init(struct st_device *device);
extern int32_t device_read(struct st_device *device, void *buf, uint32_t addition, uint32_t len);
extern int32_t device_write(struct st_device *device, void *buf, uint32_t addtion, uint32_t len);
extern int32_t device_ioctl(struct st_device *device, uint32_t cmd,void *arg);


#ifdef __cplusplus
}
#endif /*__cplusplus */
#endif /* __DEVICE_H__ */
