/**
 * @file device.c
 *
 * @copyright This file creat by rensuiyi ,all right reserve!
 * 
 * @author rensuyi
 * 
 * @date 2013/12/21 13:29:27
 */

#include "device.h"
#include "config/include/errorno.h"

/**
 * call the device initial functions
 * 
 * @author rensuiyi (2013/12/21)
 * 
 * @param device 
 * 
 * @return uint32_t
 */
int32_t device_init(struct st_device *device)
{
    if ((device != NULL)&&(device->init!= NULL)) {
           return device->init(device);
    }
    return SL_E_POINT_NONE;
}

/**
 * call the device read functions
 * 
 * @author rensuiyi (2013/12/21)
 * 
 * @param device 
 * @param buf 
 * @param addition 
 * @param len 
 * 
 * @return uint32_t
 */
int32_t device_read(struct st_device *device,void *buf, uint32_t addition, uint32_t len)
{
    if ((device!=NULL)&&(device->read!= NULL)) {
        return device->read(device,buf,addition,len);
    }
    return SL_E_POINT_NONE;
}

/**
 * call the device write functions
 * 
 * @author rensuiyi (2013/12/21)
 * 
 * @param device 
 * @param buf 
 * @param addtion 
 * @param len 
 * 
 * @return uint32_t
 */
int32_t device_write(struct st_device *device, void *buf, uint32_t addtion, uint32_t len)
{
    if ((device!=NULL) && (device->write!=NULL)) {
        return device->write(device,buf,addtion,len);
    }
    return SL_E_POINT_NONE;
}

/**
 * call the device ioctl functions
 * 
 * @author rensuiyi (2013/12/21)
 * 
 * @param device 
 * @param cmd 
 * @param arg 
 * 
 * @return uint32_t
 */
int32_t device_ioctl(struct st_device *device, uint32_t cmd, void *arg)
{
    if ((device!=NULL)&&(device->ioctl!=NULL)) {
        return device->ioctl(device,cmd,arg);
    }
    return SL_E_POINT_NONE;
}
