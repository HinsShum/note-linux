/**
 * @file driver\seps525.c
 *
 * Copyright (C) 2021
 *
 * seps525.c is free software: you can redistribute it and/or modify
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
#include "seps525.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"

/*---------- macro ----------*/
/* oled的分辨率
 */
#define __WIDTH                             (160)
#define __HEIGHT                            (128)

#define __HORIZONTAL                        (1)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static void __seps525_refresh(seps525_describe_t *pdesc, uint32_t color);
static int32_t seps525_open(driver_t **pdrv);
static void seps525_close(driver_t **pdrv);
static int32_t seps525_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(seps525, seps525_open, seps525_close, NULL, NULL, seps525_ioctl, NULL);

/*---------- function ----------*/
static void inline __seps525_reset(seps525_describe_t *pdesc)
{
    pdesc->rst_ctrl(true);
    __delay_ms(10);
    pdesc->rst_ctrl(false);
    __delay_ms(10);
}

static void __seps525_write_reg(seps525_describe_t *pdesc, uint8_t reg)
{
    pdesc->rs_ctrl(true);
    pdesc->cs_ctrl(true);
    pdesc->xfer(reg);
    pdesc->cs_ctrl(false);
    pdesc->rs_ctrl(false);
}

static void __seps525_write_dat(seps525_describe_t *pdesc, uint8_t dat)
{
    pdesc->cs_ctrl(true);
    pdesc->xfer(dat);
    pdesc->cs_ctrl(false);
}

static void __seps525_init_reg(seps525_describe_t *pdesc)
{
    __seps525_reset(pdesc);
    /* set power save mode(REDUCE_CURRENT) */
    __seps525_write_reg(pdesc, 0x04);
    __seps525_write_dat(pdesc, 0x01);
    __delay_ms(10);
    /* set power save mode(REDUCE_CURRENT) */
    __seps525_write_reg(pdesc, 0x04);
    __seps525_write_dat(pdesc, 0x00);
    /* software reset */
    __seps525_write_reg(pdesc, 0x05);
    __seps525_write_dat(pdesc, 0x00);
    /* set display off */
    __seps525_write_reg(pdesc, 0x06);
    __seps525_write_dat(pdesc, 0x00);
    /* set oscillator control */
    __seps525_write_reg(pdesc, 0x02);
    __seps525_write_dat(pdesc, 0x01);
    /* set display frequency divide ration / oscillator frequency */
    __seps525_write_reg(pdesc, 0x03);
    __seps525_write_dat(pdesc, 0x30);
    /* display duty ratio */
    __seps525_write_reg(pdesc, 0x28);
    __seps525_write_dat(pdesc, 0x7F);
    /* specify the vertical start position of w window for written in memory */
    __seps525_write_reg(pdesc, 0x20);
    __seps525_write_dat(pdesc, 0x00);
    /* specify the vertical start position of a window for written in memory */
    __seps525_write_reg(pdesc, 0x21);
    __seps525_write_dat(pdesc, 0x00);
    /* set display start line */
    __seps525_write_reg(pdesc, 0x29);
    __seps525_write_dat(pdesc, 0x00);
    /* set interface mode */
    __seps525_write_reg(pdesc, 0x14);
    __seps525_write_dat(pdesc, 0x31);
    /* set rgb interface polarity */
    __seps525_write_reg(pdesc, 0x15);
    __seps525_write_dat(pdesc, 0x00);
    /* set column dara display control / re-map format */
    __seps525_write_reg(pdesc, 0x13);
    if(__HORIZONTAL) {
        __seps525_write_dat(pdesc, 0x00);
    } else {
        __seps525_write_dat(pdesc, 0x30);
    }
    /* set memory access control / interface pixel format */
    __seps525_write_reg(pdesc, 0x16);
    __seps525_write_dat(pdesc, 0x66);
    /* set driving current of red */
    __seps525_write_reg(pdesc, 0x10);
    __seps525_write_dat(pdesc, 0x2F);
    /* set driving current of green */
    __seps525_write_reg(pdesc, 0x11);
    __seps525_write_dat(pdesc, 0x31);
    /* set driving current of blue */
    __seps525_write_reg(pdesc, 0x12);
    __seps525_write_dat(pdesc, 0x1E);
    /* set pre-charge time of red */
    __seps525_write_reg(pdesc, 0x08);
    __seps525_write_dat(pdesc, 0x03);
    /* set pre-charge time of green */
    __seps525_write_reg(pdesc, 0x09);
    __seps525_write_dat(pdesc, 0x04);
    /* set pre-charge time of blue */
    __seps525_write_reg(pdesc, 0x0A);
    __seps525_write_dat(pdesc, 0x01);
    /* set pre-charge current of red */
    __seps525_write_reg(pdesc, 0x0B);
    __seps525_write_dat(pdesc, 0x1A);
    /* set pre-charge current of green */
    __seps525_write_reg(pdesc, 0x0C);
    __seps525_write_dat(pdesc, 0x19);
    /* set pre-charge current of blue */
    __seps525_write_reg(pdesc, 0x0D);
    __seps525_write_dat(pdesc, 0x0A);
    /* control reference voltage generation */
    __seps525_write_reg(pdesc, 0x80);
    __seps525_write_dat(pdesc, 0x00);
    /* set display on */
    __seps525_write_reg(pdesc, 0x06);
    __seps525_write_dat(pdesc, 0x01);
    /* refresh */
    __seps525_refresh(pdesc, COLOR_SEPS525_BLACK);
}

static void __seps525_set_address(seps525_describe_t *pdesc, uint32_t x, uint32_t y, 
                                  uint32_t width, uint32_t height)
{
    /* specify the horizontal start position of a window for written in memory */
    __seps525_write_reg(pdesc, 0x20);
    __seps525_write_dat(pdesc, x);
    /* specify the vertical start position of a window for written in memory */
    __seps525_write_reg(pdesc, 0x21);
    __seps525_write_dat(pdesc, y);
    /* set col start address */
    __seps525_write_reg(pdesc, 0x17);
    __seps525_write_dat(pdesc, x);
    /* set col end address */
    __seps525_write_reg(pdesc, 0x18);
    __seps525_write_dat(pdesc, x + width - 1);
    /* set row start address */
    __seps525_write_reg(pdesc, 0x19);
    __seps525_write_dat(pdesc, y);
    /* set row end address */
    __seps525_write_reg(pdesc, 0x1A);
    __seps525_write_dat(pdesc, y + height - 1);
    /* start write */
    __seps525_write_reg(pdesc, 0x22);
}

static void __seps525_draw_pixel(seps525_describe_t *pdesc, uint32_t x, uint32_t y, uint32_t color)
{
    __seps525_set_address(pdesc, x, y, 1, 1);
    pdesc->cs_ctrl(true);
    pdesc->xfer((color >> 8) & 0xFF);
    pdesc->xfer(color & 0xFF);
    pdesc->cs_ctrl(false);
}

static void __seps525_draw_rectangle(seps525_describe_t *pdesc, uint32_t x, uint32_t y, 
                                     uint32_t width, uint32_t height, uint16_t *color)
{
    uint32_t size = width * height;

    __seps525_set_address(pdesc, x, y, width, height);
    pdesc->cs_ctrl(true);
    for(uint32_t i = 0; i < size; ++i) {
        pdesc->xfer((*color >> 8) & 0xFF);
        pdesc->xfer(*color & 0xFF);
        color++;
    }

    pdesc->cs_ctrl(false);
}

static void __seps525_refresh(seps525_describe_t *pdesc, uint32_t color)
{
    __seps525_set_address(pdesc, 0, 0, __WIDTH, __HEIGHT);
    pdesc->cs_ctrl(true);
    for(uint32_t i = 0; i < (__WIDTH * __HEIGHT); ++i) {
        pdesc->xfer((color >> 8) & 0xFF);
        pdesc->xfer(color & 0xFF);
    }
    pdesc->cs_ctrl(false);
}

static int32_t seps525_open(driver_t **pdrv)
{
    seps525_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        retval = (pdesc->init() ? CY_EOK : CY_ERROR);
        if(CY_EOK == retval) {
            __seps525_init_reg(pdesc);
        }
    }

    return retval;
}

static void seps525_close(driver_t **pdrv)
{
    seps525_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

static int32_t seps525_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    seps525_describe_t *pdesc = NULL;
    int32_t retval = CY_ERROR;
    seps525_ioctl_args_un *parg = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_SEPS525_DRAW_PIXEL:
            if(pdesc && args) {
                parg = (seps525_ioctl_args_un *)args;
                __seps525_draw_pixel(pdesc, parg->pixel.x, parg->pixel.y, parg->pixel.color);
                retval = CY_EOK;
            }
            break;
        case IOCTL_SEPS525_SCREEN_CLEAR:
            if(pdesc && args) {
                parg = (seps525_ioctl_args_un *)args;
                __seps525_refresh(pdesc, parg->pixel.color);
                retval = CY_EOK;
            }
            break;
        case IOCTL_SEPS525_DRAW_RECTANGLE:
            if(pdesc && args) {
                parg = (seps525_ioctl_args_un *)args;
                __seps525_draw_rectangle(pdesc, parg->rectangle.x, parg->rectangle.y,
                                         parg->rectangle.width, parg->rectangle.height, parg->rectangle.color);
                retval = CY_EOK;
            }
        default:
            retval = CY_E_WRONG_ARGS;
            break;
    }

    return retval;
}
