/**
 * @file driver\motorcomm_yt8512x.c
 *
 * Copyright (C) 2021
 *
 * motorcomm_yt8512x.c is free software: you can redistribute it and/or modify
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
#include "motorcomm_yt8512x.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"
#include <string.h>

/*---------- macro ----------*/
/* phy id defined
 */
#define PHY_ID_MASK                             (0x00000FFF)
#define PHY_ID_YT8010                           (0x00000309)
#define PHY_ID_YT8510                           (0x00000109)
#define PHY_ID_YT8511                           (0x0000010A)
#define PHY_ID_YT8512                           (0x00000118)
#define PHY_ID_YT8512B                          (0x00000128)
#define PHY_ID_YT8521                           (0x0000011A)

/* phy MII register defined
 */
#define PHY_REG_BASIC_CONTROL                   (0x00)
#define PHY_REG_BASIC_STATUS                    (0x01)
#define PHY_REG_IDENTIFICATION1                 (0x02)
#define PHY_REG_IDENTIFICATION2                 (0x03)
#define PHY_REG_AN_ADV                          (0x04)
#define PHY_REG_AN_LINK_PARTNER                 (0x05)
#define PHY_REG_SPEC_STATUS                     (0x11)
#define PHY_REG_DEBUG_ADDR_OFFSET               (0x1E)
#define PHY_REG_DEBUG_DATA                      (0x1F)

/* phy extended register defined
 */
#define PHY_EXTREG_AFE_PLL                      (0x50)
#define PHY_EXTREG_EXTEND_COMBO                 (0x4000)
#define PHY_EXTREG_LED0                         (0x40C0)
#define PHY_EXTREG_LED1                         (0x40C3)
#define PHY_EXTREG_SLEEP_CONTROL1               (0x2027)

/* phy MII register value defined
 */
#define PHY_REG_BCR_SOFTWARE_RESET              (0x8000)
#define PHY_REG_BCR_DUPLEX_BIT                  (8)
#define PHY_REG_BCR_DUPLEX_HALF                 (0 << PHY_REG_BCR_DUPLEX_BIT)
#define PHY_REG_BCR_DUPLEX_FULL                 (1 << PHY_REG_BCR_DUPLEX_BIT)
#define PHY_REG_BCR_AUTONEG_EN_BIT              (12)
#define PHY_REG_BCR_AUTONEG_EN                  (1 << PHY_REG_BCR_AUTONEG_EN_BIT)
#define PHY_REG_BCR_SPEED_BIT                   (13)
#define PHY_REG_BCR_SPEED_10M                   (0 << PHY_REG_BCR_SPEED_BIT)
#define PHY_REG_BCR_SPEED_100M                  (1 << PHY_REG_BCR_SPEED_BIT)
#define PHY_REG_BSR_LINK_BIT                    (2)
#define PHY_REG_BSR_LINK_DOWN                   (0 << PHY_REG_BSR_LINK_BIT)
#define PHY_REG_BSR_LINK_UP                     (1 << PHY_REG_BSR_LINK_BIT)
#define PHY_REG_BSR_AUTONEG_BIT                 (5)
#define PHY_REG_BSR_AUTONEG_COMPLETE            (1 << PHY_REG_BSR_AUTONEG_BIT)
#define PHY_REG_SR_SPEED_MODE_BIT               (14)
#define PHY_REG_SR_SPEED_10M                    (0 << PHY_REG_SR_SPEED_MODE_BIT)
#define PHY_REG_SR_SPEED_100M                   (1 << PHY_REG_SR_SPEED_MODE_BIT)
#define PHY_REG_SR_DUPLEX_BIT                   (13)
#define PHY_REG_SR_DUPLEX_HALF                  (0 << PHY_REG_SR_DUPLEX_BIT)
#define PHY_REG_SR_DUPLEX_FULL                  (1 << PHY_REG_SR_DUPLEX_BIT)

/* phy extend register value defined
 */
#define PHY_EXTREG_CONFIG_PLL_REFCLK_SEL_EN     (0x0040)
#define PHY_EXTREG_CONTROL1_MII_MODE_MSK        (0x0003)
#define PHY_EXTREG_CONTROL1_MII_MODE_MII        (0x0000)
#define PHY_EXTREG_CONTROL1_MII_MODE_REMII      (0x0001)
#define PHY_EXTREG_CONTROL1_MII_MODE_RMII2      (0x0002)
#define PHY_EXTREG_CONTROL1_MII_MODE_RMII1      (0x0003)
#define PHY_EXTREG_LED0_ACT_BLK_IND             (0x1000)
#define PHY_EXTREG_LED0_DIS_LED_AN_TRY          (0x0001)
#define PHY_EXTREG_LED0_BT_BLK_EN               (0x0002)
#define PHY_EXTREG_LED0_HT_BLK_EN               (0x0004)
#define PHY_EXTREG_LED0_COL_BLK_EN              (0x0008)
#define PHY_EXTREG_LED0_BT_ON_EN                (0x0010)
#define PHY_EXTREG_LED1_BT_ON_EN                (0x0010)
#define PHY_EXTREG_LED1_TXACT_BLK_EN            (0x0100)
#define PHY_EXTREG_LED1_RXACT_BLK_EN            (0x0200)
#define PHY_EXTREG_EN_SLEEP_SW                  (0x8000)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t yt8512x_open(driver_t **pdrv);
static void yt8512_close(driver_t **pdrv);
static int32_t yt8512_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t yt8512_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len);

/*---------- type define ----------*/
/*---------- variable ----------*/
DRIVER_DEFINED(yt8512x, yt8512x_open, yt8512_close, NULL, NULL, yt8512_ioctl, yt8512_irq_handler);

/*---------- function ----------*/
static int32_t yt8512_read_extreg(motorcomm_yt8512x_describe_t *pdesc, uint16_t reg)
{
    int32_t retval = CY_EOK;

    do {
        if(CY_EOK != pdesc->phy_write(PHY_REG_DEBUG_ADDR_OFFSET, reg)) {
            retval = CY_ERROR;
            break;
        }
        retval = pdesc->phy_read(PHY_REG_DEBUG_DATA);
    } while(0);

    return retval;
}

static int32_t yt8512_write_extrge(motorcomm_yt8512x_describe_t *pdesc, uint16_t reg, uint16_t val)
{
    int32_t retval = CY_EOK;

    do {
        if(CY_EOK != pdesc->phy_write(PHY_REG_DEBUG_ADDR_OFFSET, reg)) {
            retval = CY_ERROR;
            break;
        }
        retval = pdesc->phy_write(PHY_REG_DEBUG_DATA, val);
    } while(0);

    return retval;
}

static int32_t yt8512_clock_init(motorcomm_yt8512x_describe_t *pdesc)
{
    int32_t retval = CY_EOK;
    bool reset = false;
    uint16_t mii = PHY_EXTREG_CONTROL1_MII_MODE_RMII2;

    do {
        if(pdesc->clk_source == CLOCK_SOURCE_CLOCK_SIGNAL_50M) {
            retval = yt8512_read_extreg(pdesc, PHY_EXTREG_AFE_PLL);
            if(retval < CY_EOK) {
                break;
            }
            retval |= PHY_EXTREG_CONFIG_PLL_REFCLK_SEL_EN;
            if(CY_EOK != yt8512_write_extrge(pdesc, PHY_EXTREG_AFE_PLL, (uint16_t)retval)) {
                retval = CY_ERROR;
                break;
            }
            reset = true;
        }
        /* select RMII2 mode */
        retval = yt8512_read_extreg(pdesc, PHY_EXTREG_EXTEND_COMBO);
        if(retval < CY_EOK) {
            break;
        }
        switch(pdesc->mii_mode) {
            case MII:
                mii = PHY_EXTREG_CONTROL1_MII_MODE_MII;
                break;
            case RMII1:
                mii = PHY_EXTREG_CONTROL1_MII_MODE_RMII1;
                break;
            case RMII2:
                mii = PHY_EXTREG_CONTROL1_MII_MODE_RMII2;
                break;
            case REMII:
                mii = PHY_EXTREG_CONTROL1_MII_MODE_REMII;
                break;
            default:
                break;
        }
        if((retval & PHY_EXTREG_CONTROL1_MII_MODE_MSK) != mii) {
            retval &= ~PHY_EXTREG_CONTROL1_MII_MODE_MSK;
            retval |= mii;
            if(CY_EOK != yt8512_write_extrge(pdesc, PHY_EXTREG_EXTEND_COMBO, (uint16_t)retval)) {
                retval = CY_ERROR;
                break;
            }
            reset = true;
        }
        if(reset) {
            /* software reset */
            retval = pdesc->phy_read(PHY_REG_BASIC_CONTROL);
            if(retval < CY_EOK) {
                break;
            }
            retval |= PHY_REG_BCR_SOFTWARE_RESET;
            retval = pdesc->phy_write(PHY_REG_BASIC_CONTROL, (uint16_t)retval);
            if(retval != CY_EOK) {
                break;
            }
            __delay_ms(150);
        }
    } while(0);

    return retval;
}

static int32_t yt8512_led_init(motorcomm_yt8512x_describe_t *pdesc)
{
    int32_t retval = CY_EOK;

    do {
        retval = yt8512_read_extreg(pdesc, PHY_EXTREG_LED0);
        if(retval < CY_EOK) {
            break;
        }
        retval |= PHY_EXTREG_LED0_ACT_BLK_IND;
        retval &= ~(PHY_EXTREG_LED0_DIS_LED_AN_TRY | PHY_EXTREG_LED0_BT_BLK_EN |\
                    PHY_EXTREG_LED0_HT_BLK_EN | PHY_EXTREG_LED0_COL_BLK_EN | PHY_EXTREG_LED0_BT_ON_EN);
        if(CY_EOK != yt8512_write_extrge(pdesc, PHY_EXTREG_LED0, (uint16_t)retval)) {
            retval = CY_ERROR;
            break;
        }
        retval = yt8512_read_extreg(pdesc, PHY_EXTREG_LED1);
        if(retval < CY_EOK) {
            break;
        }
        retval |= PHY_EXTREG_LED1_BT_ON_EN;
        retval &= ~(PHY_EXTREG_LED1_TXACT_BLK_EN | PHY_EXTREG_LED1_RXACT_BLK_EN);
        retval = yt8512_write_extrge(pdesc, PHY_EXTREG_LED1, (uint16_t)retval);
    } while(0);

    return retval;
}

static int32_t yt8512x_open(driver_t **pdrv)
{
    motorcomm_yt8512x_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        retval = pdesc->init();
        if(CY_EOK == retval) {
            do {
                retval = yt8512_clock_init(pdesc);
                if(retval < CY_EOK) {
                    __debug_error("YT8512 initialize clock register failed\n");
                    break;
                }
                retval = yt8512_led_init(pdesc);
                if(retval < CY_EOK) {
                    __debug_error("YT8512 initialize led register failed\n");
                    break;
                }
                /* disable auto sleep */
                retval = yt8512_read_extreg(pdesc, PHY_EXTREG_SLEEP_CONTROL1);
                if(retval < CY_EOK) {
                    __debug_error("YT8512 read %04X register failed\n", PHY_EXTREG_SLEEP_CONTROL1);
                    break;
                }
                retval &= ~PHY_EXTREG_EN_SLEEP_SW;
                retval = yt8512_write_extrge(pdesc, PHY_EXTREG_SLEEP_CONTROL1, (uint16_t)retval);
            } while(0);
            if(retval != CY_EOK) {
                __debug_message("YT8512 deinit hardware\n");
                pdesc->deinit();
            }
        } else if(CY_E_TIME_OUT == retval) {
            __debug_error("Reset Ethernet Controller tiemout\n");
        } else {
            __debug_error("YT8512 can not be reset, maybe SIM interface has some errors take place\n");
        }
    }

    return retval;
}

static void yt8512_close(driver_t **pdrv)
{
    motorcomm_yt8512x_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

static int32_t yt8512_autoneg(motorcomm_yt8512x_describe_t *pdesc, uint32_t timeout_ms)
{
    int32_t retval = CY_EOK;

    do {
        /* yt8512 enable automatic auto negottation, 
         * if send command to set enable the bit, 
         * will make link down.
         */
#if 0   
        /* enable auto negotiation */
        retval = PHY_REG_BCR_AUTONEG_EN;
        if(CY_EOK != (retval = pdesc->phy_write(PHY_REG_BASIC_CONTROL, (uint16_t)retval))) {
            printk(KERN_ERROR "YT8512 enable auto negotiation failed\n");
            break;
        }
#endif
        /* check whether auto negotiation is complete */
        do {
            retval = pdesc->phy_read(PHY_REG_BASIC_STATUS);
            if(CY_EOK > retval) {
                /* do nothing, delay 1ms, read again */
            } else if(!!(retval & PHY_REG_BSR_AUTONEG_COMPLETE)) {
                break;
            } else if(!(retval & PHY_REG_BSR_LINK_UP)) {
                timeout_ms = 1;
            }
            timeout_ms--;
            __delay_ms(1);
        } while(timeout_ms);
        if(timeout_ms == 0) {
            retval = CY_E_TIME_OUT;
        } else {
            if(CY_EOK > (retval = pdesc->phy_read(PHY_REG_SPEC_STATUS))) {
                __debug_error("YT8512 read specific status register failed\n");
                break;
            }
            if(retval & PHY_REG_SR_SPEED_100M) {
                pdesc->speed = MBPS_100;
            } else {
                pdesc->speed = MBPS_10;
            }
            if(retval & PHY_REG_SR_DUPLEX_FULL) {
                pdesc->duplex = FULL_DUPLEX;
            } else {
                pdesc->duplex = HALF_DUPLEX;
            }
        }
        retval = CY_EOK;
        if(pdesc->adjust_mode) {
            retval = pdesc->adjust_mode();
        }
    } while(0);

    return retval;
}

static int32_t yt8512_get_link_status(motorcomm_yt8512x_describe_t *pdesc, yt8512_link_status_en *link)
{
    int32_t retval = CY_EOK;

    do {
        retval = pdesc->phy_read(PHY_REG_BASIC_STATUS);
        if(CY_EOK > retval) {
            break;
        }
        if(!!(retval & PHY_REG_BSR_LINK_UP)) {
            *link = LINK_UP;
        } else {
            *link = LINK_DOWN;
        }
        retval = CY_EOK;
    } while(0);

    return retval;
}

static int32_t yt8512_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    motorcomm_yt8512x_describe_t *pdesc = NULL;
    int32_t retval = CY_E_POINT_NONE;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_YT8512X_SET_IRQ_HANDLER:
            if(pdesc) {
                pdesc->irq_handler = (int32_t (*)(uint32_t, void *, uint32_t))args;
                retval = CY_EOK;
            }
            break;
        case IOCTL_YT8512X_GET_LINK_STATUS:
            if(pdesc) {
                retval = yt8512_get_link_status(pdesc, args);
            }
            break;
        case IOCTL_YT8512X_ENABLE:
            if(pdesc && pdesc->enable) {
                pdesc->enable(true);
                retval = CY_EOK;
            }
            break;
        case IOCTL_YT8512X_DISABLE:
            if(pdesc && pdesc->enable) {
                pdesc->enable(false);
                retval = CY_EOK;
            }
            break;
        case IOCTL_YT8512X_AUTONE:
            if(pdesc) {
                uint32_t timeout_ms = 0;
                if(args) {
                    timeout_ms = *(uint32_t *)args;
                }
                retval = yt8512_autoneg(pdesc, timeout_ms);
            }
            break;
        case IOCTL_YT8512X_GET_MAC:
            if(pdesc && args) {
                *(void **)args = pdesc->mac;
                retval = ARRAY_SIZE(pdesc->mac);
            }
            break;
        default:
            retval = CY_E_WRONG_ARGS;
            break;
    }

    return retval;
}

static int32_t yt8512_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t len)
{
    motorcomm_yt8512x_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->irq_handler) {
        retval = pdesc->irq_handler(irq_handler, args, len);
    }

    return retval;
}
