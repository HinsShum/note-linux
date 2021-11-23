/**
 * @file driver\include\motorcomm_yt8512x.h
 *
 * Copyright (C) 2021
 *
 * motorcomm_yt8512x.h is free software: you can redistribute it and/or modify
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
#ifndef __MOTORCOMM_YT8512X_H
#define __MOTORCOMM_YT8512X_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"

/*---------- macro ----------*/
#define IOCTL_YT8512X_SET_IRQ_HANDLER                       (IOCTL_USER_START + 0x00)
#define IOCTL_YT8512X_GET_LINK_STATUS                       (IOCTL_USER_START + 0x01)
#define IOCTL_YT8512X_ENABLE                                (IOCTL_USER_START + 0x02)
#define IOCTL_YT8512X_DISABLE                               (IOCTL_USER_START + 0x03)
#define IOCTL_YT8512X_AUTONE                                (IOCTL_USER_START + 0x04)
#define IOCTL_YT8512X_GET_MAC                               (IOCTL_USER_START + 0x05)

/*---------- type define ----------*/
typedef enum {
    LINK_DOWN,
    LINK_UP
} yt8512_link_status_en;

typedef enum {
    MII,
    RMII1,
    RMII2,
    REMII
} yt8512_mii_mode_en;

typedef enum {
    CLOCK_SOURCE_XTAL_25M,
    CLOCK_SOURCE_CLOCK_SIGNAL_50M
} yt8512_clock_source_en;

typedef enum {
    FULL_DUPLEX,
    HALF_DUPLEX
} yt8512_duplex_en;

typedef enum {
    MBPS_10,
    MBPS_100
} yt8512_speed_en;

typedef struct {
    yt8512_link_status_en link_status;
    yt8512_mii_mode_en mii_mode;
    yt8512_clock_source_en clk_source;
    yt8512_duplex_en duplex;
    yt8512_speed_en speed;
    uint8_t phy_addr;
    uint8_t mac[6];
    int32_t (*init)(void);
    void (*deinit)(void);
    void (*rst_ctrl)(bool ctrl);
    void (*enable)(bool en);
    int32_t (*adjust_mode)(void);
    int32_t (*phy_read)(uint16_t reg);
    int32_t (*phy_write)(uint16_t reg, uint16_t val);
    int32_t (*irq_handler)(uint32_t irq_handler, void *args, uint32_t len);
} motorcomm_yt8512x_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __MOTORCOMM_YT8512X_H */
