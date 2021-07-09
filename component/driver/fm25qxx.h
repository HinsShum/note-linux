/**
 * @file driver/include/fm25qxx.h
 *
 * Copyright (C) 2021
 *
 * fm25qxx.h is free software: you can redistribute it and/or modify
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
#ifndef __FM25QXX_H
#define __FM25QXX_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "flash.h"

/*---------- macro ----------*/
#define IOCTL_FM25QXX_GET_IDCODE                    (IOCTL_FLASH_USER_START + 0x00)

/*---------- type define ----------*/
typedef struct {
    flash_describe_t flash;
    uint16_t idcode;
    void (*cs_ctrl)(bool cs);
    uint8_t (*xfer)(uint8_t data);
} fm25qxx_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __FM25QXX_H */
