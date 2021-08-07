/**
 * @file common/include/parameters.h
 *
 * Copyright (C) 2020
 *
 * parameters.h is free software: you can redistribute it and/or modify
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
#ifndef __PARAMETERS_H
#define __PARAMETERS_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
struct st_para_info {
    uint32_t start_address;      /*<< the start address of the flash, 4-byte aligned */
    uint32_t end_address;        /*<< the end address of the flash, 4-byte aligned */
    uint32_t para_size;         /*<< para size, n * para size = block size */
    uint32_t erase_block_size;  /*<< the min size of the erase block size */
    uint32_t para_address;      /*<< the lastest parameter table address */
    bool para_valid;            /*<< the valid flag about para address */
    /* flash write interface */
    bool (*write)(uint32_t address, void *data, uint32_t len);
    /* flash read interface */
    uint32_t (*read)(uint32_t address, void *data, uint32_t len);
    /* flash erase interface, erase signal block */
    bool (*erase)(uint32_t address);
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief Initializes the flash parameter block.
 *
 * This function initializes a fault-tolerant, persistent storage mechanism
 * for a parameter block for an application.  The last several erase blocks
 * of flash (as specified by \e ulStart and \e ulEnd are used for the
 * storage; more than one erase block is required in order to be
 * fault-tolerant.
 *
 * A parameter block is an array of bytes that contain the persistent
 * parameters for the application.  The only special requirement for the
 * parameter block is that the first byte is a sequence number (explained
 * in FlashPBSave()) and the second byte is a checksum used to validate the
 * correctness of the data (the checksum byte is the byte such that the sum of
 * all bytes in the parameter block is zero).
 *
 * The portion of flash for parameter block storage is split into N
 * equal-sized regions, where each region is the size of a parameter block
 * (\e ulSize).  Each region is scanned to find the most recent valid
 * parameter block.  The region that has a valid checksum and has the highest
 * sequence number (with special consideration given to wrapping back to zero)
 * is considered to be the current parameter block.
 *
 * In order to make this efficient and effective, three conditions must be
 * met.  The first is \e ulStart and \e ulEnd must be specified such that at
 * least two erase blocks of flash are dedicated to parameter block storage.
 * If not, fault tolerance can not be guaranteed since an erase of a single
 * block will leave a window where there are no valid parameter blocks in
 * flash.  The second condition is that the size (\e ulSize) of the parameter
 * block must be an integral divisor of the size of an erase block of flash.
 * If not, a parameter block will end up spanning between two erase blocks of
 * flash, making it more difficult to manage.  The final condition is that the
 * size of the flash dedicated to parameter blocks (\e ulEnd - \e ulStart)
 * divided by the parameter block size (\e ulSize) must be less than or equal
 * to 128.  If not, it will not be possible in all cases to determine which
 * parameter block is the most recent (specifically when dealing with the
 * sequence number wrapping back to zero).
 *
 * When the microcontroller is initially programmed, the flash blocks used for
 * parameter block storage are left in an erased state.
 *
 * This function must be called before any other flash parameter block
 * functions are called.
 *
 * @param pinfo: the info of the flash and parameter table.
 * 
 * @retval false: no parameter not found
 *         true: find the paratmeter
 */
extern bool parameters_init(struct st_para_info *pinfo);

/**
 * @brief Writes a new parameter block to flash.
 *
 * This function will write a parameter block to flash.  Saving the new
 * parameter blocks involves three steps:
 *
 * - Setting the sequence number such that it is one greater than the sequence
 *   number of the latest parameter block in flash.
 * - Computing the checksum of the parameter block.
 * - Writing the parameter block into the storage immediately following the
 *   latest parameter block in flash; if that storage is at the start of an
 *   erase block, that block is erased first.
 *
 * By this process, there is always a valid parameter block in flash.  If
 * power is lost while writing a new parameter block, the checksum will not
 * match and the partially written parameter block will be ignored.  This is
 * what makes this fault-tolerant.
 *
 * Another benefit of this scheme is that it provides wear leveling on the
 * flash.  Since multiple parameter blocks fit into each erase block of flash,
 * and multiple erase blocks are used for parameter block storage, it takes
 * quite a few parameter block saves before flash is re-written.
 *
 * @param pinfo: is the info of the parameter table
 * @param new: is the address of the parameter block to be
 * written to flash
 *
 * @retval false: save new parameter failed
 *         true: save new parameter success.
 */
extern bool parameters_save(struct st_para_info *pinfo, void *new);

/**
 * @brief Gets the address of the most recent parameter block.
 * This function returns the address of the most recent parameter block
 * that is stored in flash.
 * 
 * @param pinfo: the info of the flash and parameter table
 * @param data: the buffer for store data
 * @param len: the buffer length
 * 
 * @retval true/false
 */
extern bool parameters_get(struct st_para_info *pinfo, void *data, uint32_t len);

#endif /* __PARAMETERS_H */
