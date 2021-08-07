/**
 * @file common/parameters.c
 *
 * Copyright (C) 2020
 *
 * parameters.c is free software: you can redistribute it and/or modify
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
#include "parameters.h"

/*---------- macro ----------*/
/* define the embeded flash type
 * FLASH_TYPE_ZERO: erase the embeded flash, the value is zero, for example: stm32l073
 * FLASH_TYPE_ONE: erase the embeded flash, the value is one, for example: efm32lg280
 */
#if !defined(FLASH_TYPE_ZERO) && !defined(FLASH_TYPE_ONE)
    #error "No embeded flash erase type set"
#endif

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * @brief Determines if the parameter block at the given address is valid.
 *
 * This function will compute the checksum of a parameter block in flash to
 * determine if it is valid.
 *
 * @param offset is the address of the parameter block to check.
 *
 * @retval true:legal
 *         false:illegal
 */
static bool _parameter_is_vailed(struct st_para_info *pinfo, uint32_t offset)
{
    uint32_t index = 0, sum = 0;
    bool retval = false;

    for(; index < pinfo->para_size; ++index) {
        uint8_t tmp = 0;
        pinfo->read(offset + index, &tmp, sizeof(tmp));
        sum += tmp;
    }
    /* check sum should be zero, if not, return false
     */
    if(!(sum & 0xFF)) {
#if defined(FLASH_TYPE_ZERO)
        for(index = 0; index < pinfo->para_size; ++index) {
            uint8_t tmp = 0;
            pinfo->read(offset + index, &tmp, sizeof(tmp));
            if(tmp != 0x00) {
                break;
            }
        }
        if(index != pinfo->para_size) {
            retval = true;
        }
#else   /* defined FLASH_TYPE_ONE */
        /* If the sum is equal to the size * 255,
         * then the block is all ones and should
         * not be considered valid.
         * g_ulFlashPBSize * 255 = 256  * 255 = 0x0FF00
         *                         1024 * 255 = 0x3FC00
         *                         2048 * 255 = 0x7F800
         */
        if((pinfo->para_size * 255) != sum) {
            retval = true;
        }
#endif
    }

    return retval;
}

bool parameters_init(struct st_para_info *pinfo)
{
    uint32_t offset = 0, current = 0;
    uint32_t one = 0, two = 0;
    bool retval = false;

    /* loop through the portion of flash memory 
     * used for stoting parameter block
     */
    for(offset = pinfo->start_address; offset < pinfo->end_address;
        offset += pinfo->para_size) {
        if(_parameter_is_vailed(pinfo, offset)) {
            if(current) {
                pinfo->read(current, &one, sizeof(one));
                pinfo->read(offset, &two, sizeof(two));
                if(one > two) {
                    continue;
                }
            }
            current = offset;
            retval = true;
        }
    }
    pinfo->para_address = current;
    pinfo->para_valid = retval;

    return retval;
}

bool parameters_get(struct st_para_info *pinfo, void *data, uint32_t len)
{
    /* return true when the address of the most recent parameter block if it exits
     * otherwise return false
     */
    bool retval = false;

    if(pinfo->para_valid && len <= pinfo->para_size) {
        if(_parameter_is_vailed(pinfo, pinfo->para_address) &&
           pinfo->read(pinfo->para_address, data, len) == len) {
            retval = true;
        }
    }

    return retval;
}

bool parameters_save(struct st_para_info *pinfo, void *new)
{
    bool retval = false;
    uint8_t *pnew_para = NULL;
    uint32_t index = 0, sum = 0;
    uint32_t offset = 0;

    pnew_para = (uint8_t *)new;
    if(pinfo->para_valid) {
        {
            uint32_t seq = 0;
            pinfo->read(pinfo->para_address, &seq, sizeof(seq));
            ((uint32_t *)pnew_para)[0] = seq + 1;    
        }
        offset = pinfo->para_address + pinfo->para_size;
        if(offset == pinfo->end_address) {
            offset = pinfo->start_address;
        }
    } else {
        ((uint32_t *)pnew_para)[0] = 0;
        offset = pinfo->start_address;
    }
    /* calc the check sum */
    for(; index < pinfo->para_size; ++index) {
        sum -= pnew_para[index];
    }
    pnew_para[4] += sum;
    /* look for a location to store the new parameter block
     */
    while(true) {
        if((offset & (pinfo->erase_block_size - 1)) == 0) {
            pinfo->erase(offset);    /* into new block */
        }
#if defined(FLASH_TYPE_ZERO)
        for(index = 0; index < pinfo->para_size; ++index) {
            uint8_t tmp = 0;
            pinfo->read(offset + index, &tmp, sizeof(tmp));
            if(tmp != 0x00) {
                break;
            }
        }
#else   /* defined FLASH_TYPE_ONE */
        for(index = 0; index < pinfo->para_size; ++index) {
            uint8_t tmp = 0;
            pinfo->read(offset + index, &tmp, sizeof(tmp));
            if(tmp != 0xFF) {
                break;
            }
        }
#endif
        if(index == pinfo->para_size) {
            /* find the new clear page */
            retval = true;
            break;
        }
        offset += pinfo->para_size;
        if(offset == pinfo->end_address) {
            offset = pinfo->start_address;
        }
        if((pinfo->para_valid && offset == pinfo->para_address) ||
           (!pinfo->para_valid && offset == pinfo->start_address)) {
            break;
        }
    }
    if(retval) {
        pinfo->write(offset, (void *)pnew_para, pinfo->para_size);
        /* check if the value is written to the embeded flash correctly */
        for(index = 0; index < pinfo->para_size; ++index) {
            uint8_t tmp = 0;
            pinfo->read(offset + index, &tmp, sizeof(tmp));
            if(tmp != pnew_para[index]) {
                retval = false;
                break;
            }
        }
        if(retval) {
            pinfo->para_valid = true;
            pinfo->para_address = offset;
        }
    }

    return retval;
}
