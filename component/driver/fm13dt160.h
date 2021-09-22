/**
 * @file driver\include\fm13dt160.h
 *
 * Copyright (C) 2021
 *
 * fm13dt160.h is free software: you can redistribute it and/or modify
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
#ifndef __FM13DT160_H
#define __FM13DT160_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "i2c_bus.h"

/*---------- macro ----------*/
#define IOCTL_FM13DT160_CLEAR_IRQ                   (IOCTL_USER_START + 0x00)
#define IOCTL_FM13DT160_INIT_REGFILE                (IOCTL_USER_START + 0x01)

/* Sector address defined
 */
#define FM13DT160_SECTOR1_BASE                      (0xB040)
#define FM13DT160_SECTOR2_BASE                      (0xB080)
#define FM13DT160_SECTOR3_BASE                      (0xB0C0)
#define FM13DT160_SECTOR4_BASE                      (0xB100)
#define FM13DT160_SECTOR5_BASE                      (0xB140)
#define FM13DT160_SECTOR6_BASE                      (0xB180)

#define FM13DT160_ADDRESS_USER_CFG0                 (FM13DT160_SECTOR1_BASE + 0x00)
#define FM13DT160_ADDRESS_USER_CFG1                 (FM13DT160_SECTOR1_BASE + 0x02)
#define FM13DT160_ADDRESS_USER_CFG2                 (FM13DT160_SECTOR1_BASE + 0x04)
#define FM13DT160_ADDRESS_USER_CFG3                 (FM13DT160_SECTOR1_BASE + 0x06)
#define FM13DT160_ADDRESS_USER_AREA_SIZE            (FM13DT160_SECTOR1_BASE + 0x14)
#define FM13DT160_ADDRESS_USER_AREA_PASSWD          (FM13DT160_SECTOR4_BASE + 0x20)
#define FM13DT160_ADDRESS_UNLOCK_PASSWD             (FM13DT160_SECTOR4_BASE + 0x2C)
#define FM13DT160_ADDRESS_KILL_PASSWD               (FM13DT160_SECTOR5_BASE + 0x00)
#define FM13DT160_ADDRESS_ACCESS_PASSWD             (FM13DT160_SECTOR5_BASE + 0x04)
#define FM13DT160_ADDRESS_EPC                       (FM13DT160_SECTOR5_BASE + 0x0C)
#define FM13DT160_ADDRESS_TID                       (FM13DT160_SECTOR5_BASE + 0x20)
#define FM13DT160_LOCK_CFG                          (FM13DT160_SECTOR5_BASE + 0x38)

#define FM13DT160_USER_CFG0_IO_OD_LED               (0x01)
#define FM13DT160_USER_CFG0_IO_IRQ                  (0x02)
#define FM13DT160_USER_CFG0_IO_SENSOR               (0x03)
#define FM13DT160_USER_CFG0_IO_MASK                 (0x03)
#define FM13DT160_USER_CFG3_IO_INT_EN_HF            (0x01)
#define FM13DT160_USER_CFG3_IO_INT_EN_UHF           (0x02)
#define FM13DT160_USER_CFG3_IO_INT_EN_RTC           (0x04)
#define FM13DT160_USER_CFG3_IO_INT_MODE_LEVEL       (0x00)
#define FM13DT160_USER_CFG3_IO_INT_MODE_PULSE       (0x08)
#define FM13DT160_USER_AREA_SIZE_NO_EXIT            (0x030F)
#define FM13DT160_USER_AREA_SIZE_1K                 (0x83FF)

/*---------- type define ----------*/
/* ioctl irq enum defined
 */
typedef enum {
    FM13DT160_IRQ_HF = (1 << 0),
    FM13DT160_IRQ_UHF = (1 << 1),
    FM13DT160_IRQ_RTC = (1 << 2),
    FM13DT160_IRQ_ALL = FM13DT160_IRQ_HF | FM13DT160_IRQ_UHF | FM13DT160_IRQ_RTC
} fm13dt160_irq_en;

typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    bool (*pwctl)(bool on);
    int32_t (*irq_handler)(uint32_t irq_handler, void *args, uint32_t len);
} fm13dt160_ops_t;

typedef struct {
    uint8_t address;
    char *bus_name;
    void *bus;
    fm13dt160_ops_t ops;
} fm13dt160_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __FM13DT160_H */
