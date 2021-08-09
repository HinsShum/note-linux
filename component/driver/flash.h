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
/**
 * @brief Erase a block on the flash device.
 * @note If the flash supports different block size, the block size that 
 *       this interface may erase is differnet, which is determined by
 *       the characteristics of the flash and the address to be erased.
 *       for example, stm32f407xx is support 16K, 64K and 128K block size.
 * @param Args is a pointer of the address offset to be erased. It does not 
 *        need to be the first address of the block. For example, the block
 *        is from address 0x0000 to address 0x1000, the Args is a pointer of
 *        the address offset 0x0400, then, the block from address 0x0000 
 *        to the address 0x1000 will be erased. And this interface will 
 *        return the block size if erase successful.
 * @retval If the flash device is invalid or args is NULL, the interface
 *         will return CY_E_WRONG_ARGS.
 *         If the flash device erase the block failed, the interface will
 *         return CY_ERROR.
 *         It the flash device erase the block successful, the interface
 *         will return the acutal erased block size.
 */
#define IOCTL_FLASH_ERASE_BLOCK                     (IOCTL_USER_START + 0x00)

/**
 * @brief Erase the entire flash chip.
 * @note It may take a long time to erase the entire flash chip. If the wdt
 *       is enabled in the system, a wdt reset event may occur during the
 *       erasing of the entire flash. So flash device callback can be set,
 *       a typical callback function can be a wdt feeding function.
 * @param Args is not useful, it can be NULL.
 * @retval If te flash device has no erase the chip function, the interface 
 *         will return CY_E_WRONG_ARGS.
 *         If the flash device erase the chip failed, the interface will
 *         return CY_ERROR.
 *         If the flash device erase the chip successfully, the interface
 *         will return CY_EOK.
 */
#define IOCTL_FLASH_ERASE_CHIP                      (IOCTL_USER_START + 0x01)

/**
 * @brief Check if the address is the first address of the block.
 * @param Args is a pointer of the address offset that will be check.
 * @retval If the address is the first address of the block, the interface
 *         will return CY_EOK, otherwise, return CY_ERROR.
 */
#define IOCTL_FLASH_CHECK_ADDR_IS_BLOCK_START       (IOCTL_USER_START + 0x02)

/**
 * @brief Get detailed information about the flash device.
 * @note The detailed information about the flash device includes the start
 *       address, end address and block size. If the flash device supports
 *       different block size, it means the largest block size.
 * @param Args is a pointer of the buffer to store the detailed information
 *        about the flash device. Its type is `flash_info_t`.
 * @retval If the args is NULL, the inetrface will return CY_E_WRONG_ARGS,
 *         otherwise, args will be filled, and return CY_EOK.
 */
#define IOCTL_FLASH_GET_INFO                        (IOCTL_USER_START + 0x03)

/**
 * @brief Set a callback function for the flash device.
 * @note Some flash operations(read, write and erase chip) may take a long 
 *       time. If the wdt is enabled in the system, a wdt reset event may 
 *       occur during the flash operations.
 *       In order to avoid this situation, the flash device provide callback
 *       function. A typical callback function can be a wdt feeding function.
 * @retval If the args is NULL, the interface will return CY_E_WRONG_ARGS,
 *         otherwise, return CY_EOK.
 */
#define IOCTL_FLASH_SET_CALLBACK                    (IOCTL_USER_START + 0x04)

/**
 * @brief The IOCTL_FLASH_INHERIT_START is used when other drivers to inherit 
 *        the flash driver.
 */
#define IOCTL_FLASH_INHERIT_START                   (IOCTL_USER_START + 0x05)

/*---------- type define ----------*/
typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    uint32_t (*write)(const uint8_t *pbuf, uint32_t offset, uint32_t len);
    uint32_t (*read)(uint8_t *pbuf, uint32_t offset, uint32_t len);
    uint32_t (*erase_block)(uint32_t offset);
    bool (*erase_chip)(void);
    bool (*addr_is_block_start)(uint32_t offset);
    void (*cb)(void);
} flash_ops_t;

typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t block_size;
    flash_ops_t ops;
} flash_describe_t;

typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t block_size;
} flash_info_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __FLASH_H */
