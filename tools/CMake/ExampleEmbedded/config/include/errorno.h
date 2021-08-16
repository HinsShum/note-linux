/**
 * @file errorno.h
 *
 * @copyright This file creat by rensuiyi ,all right reserve!
 * 
 * @author rensuyi
 * 
 * @date 2013/12/21 14:10:03
 */
#ifndef __ERRORNO_H__
#define __ERRORNO_H__

#ifdef __cplusplus
extern "C" 
{
#endif /*__cplusplus */

#define  SL_OK                   (0)
#define  SL_EOK                  (0)
#define  SL_ERROR                (-1)
#define  SL_E_POINT_NONE         (-2)
#define  SL_E_TIME_OUT           (-3)
#define  SL_E_BUSY               (-4)
#define  SL_E_NO_MEM             (-5)
#define  SL_E_WRONG_ARGS         (-6)
#define  SL_E_WRONG_CRC          (-7)
#define  SL_E_NOT_SUPPORT        (-8)


#define  SL_E_DEVICE_IDEL        (-31)
#define  SL_E_DEVICE_POWER_OFF   (-32)

#define  SL_E_USER(x)            (x-100)

#ifdef __cplusplus
}
#endif /*__cplusplus */
#endif /* __ERRORNO_H__ */
