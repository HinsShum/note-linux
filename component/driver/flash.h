/**
 * @file driver/include/flash.h
 *
 * Copyright (C) 2021
 *
 * flash.h is free software: you can redistribute it and/or modify
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
#ifndef __FLASH_H
#define __FLASH_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <device.h>

/*---------- macro ----------*/
#define IOCTL_FLASH_ERASE_MINIMUM_SIZE              (IOCTL_USER_START + 0x00)
#define IOCTL_FLASH_ERASE_PAGE                      (IOCTL_USER_START + 0x01)
#define IOCTL_FLASH_ERASE_SECTOR                    (IOCTL_USER_START + 0x02)
#define IOCTL_FLASH_ERASE_CHIP                      (IOCTL_USER_START + 0x03)
#define IOCTL_FLASH_GET_INFO                        (IOCTL_USER_START + 0x04)
#define IOCTL_FLASH_USER_START                      (IOCTL_USER_START + 0x05)

/*---------- type define ----------*/
typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t page_size;
    uint32_t pages;
    uint32_t sector_size;
    uint32_t sectors;
    uint32_t minimum_erase_size;
} flash_info_t;

typedef struct {
    flash_info_t info;
    bool (*init)(void);
    void (*deinit)(void);
    uint32_t (*write)(const uint8_t *pbuf, uint32_t addr, uint32_t len);
    uint32_t (*read)(uint8_t *pbuf, uint32_t addr, uint32_t len);
    bool (*erase_page)(uint32_t addr);
    bool (*erase_sector)(uint32_t addr);
    bool (*erase_chip)(void);
} flash_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __FLASH_H */
