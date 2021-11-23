/**
 * @file src\ymodem_crc.c
 *
 * Copyright (C) 2021
 *
 * ymodem_crc.c is free software: you can redistribute it and/or modify
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
#include "ymodem_crc.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * CRC16-XMODE:
 *  width: 16
 *  poly: 0x1021 (X16+X12+X5+1)
 *  init: 0x00
 *  refin: false
 *  refout: false
 *  xorout: 0x00
 */
uint16_t ymodem_crc16_xmodem(const uint8_t *byte, uint16_t length)
{
    uint16_t crc = 0x00, poly = 0x1021;

    while(length--) {
        crc ^= ((uint16_t)*byte << 8);
        byte++;
        for(uint8_t i = 0; i < 8; ++i) {
            if(crc & 0x8000) {
                crc <<= 1;
                crc ^= poly;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}
