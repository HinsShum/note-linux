/**
 * @file components\freemodbus\modbus\functions\mbfuncid.c
 *
 * Copyright (C) 2021
 *
 * mbfuncid.c is free software: you can redistribute it and/or modify
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
#include "mb.h"
#include "port.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"

/*---------- macro ----------*/
#define MB_PDU_FUNC_READ_TYPE_OFF                   ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_READ_ADDR_OFF                   ( MB_PDU_DATA_OFF + 1 )
#define MB_PDU_FUNC_READ_IDCNT_OFF                  ( MB_PDU_DATA_OFF + 3 )
#define MB_PDU_FUNC_READ_SIZE                       ( 5 )
#define MB_PDU_FUNC_READ_IDCNT_MAX                  ( 0x003E )

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern eMBException prveMBError2Exception(eMBErrorCode eErrorCode);

/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
#if MB_FUNC_READ_ID_ENABLED > 0
eMBException eMBFuncReadId(UCHAR *pcuFrame, USHORT *usLen)
{
    USHORT usAddress;
    USHORT usCount;
    UCHAR *pucFrameCur, ucType;
    eMBException eStatus = MB_EX_NONE;
    eMBErrorCode eRegStatus;

    if(*usLen == (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN)) {
        ucType = (UCHAR)(pcuFrame[MB_PDU_FUNC_READ_TYPE_OFF]);
        usAddress = (USHORT)(pcuFrame[MB_PDU_FUNC_READ_ADDR_OFF] << 8);
        usAddress |= (USHORT)(pcuFrame[MB_PDU_FUNC_READ_ADDR_OFF + 1]);
        usCount = (USHORT)(pcuFrame[MB_PDU_FUNC_READ_IDCNT_OFF] << 8);
        usCount |= (USHORT)(pcuFrame[MB_PDU_FUNC_READ_IDCNT_OFF + 1]);
        /* Check if the number of id counts to read is valid. If not
         * return Modbus illegal data value exception.
         */
        if((usCount >= 1) && (usCount <= MB_PDU_FUNC_READ_IDCNT_MAX)) {
            /* Set the current PDU data pointer to the beginning */
            pucFrameCur = &pcuFrame[MB_PDU_FUNC_OFF];
            *usLen = MB_PDU_FUNC_OFF;
            /* First byte contains the function code */
            *pucFrameCur++ = MB_FUNC_READ_ID_INFORMATION;
            *usLen += 1;
            /* Second byte in the response contain the number of bytes */
            *pucFrameCur++ = (UCHAR)(usCount * 4);
            *usLen += 1;
            /* Make callback to fill the buffer */
            eRegStatus = eMBReadIdCB(pucFrameCur, ucType, usAddress, usCount);
            /* If an error occured convert it into a Modbus exception */
            if(eRegStatus != MB_ENOERR) {
                eStatus = prveMBError2Exception(eRegStatus);
            } else {
                *usLen += usCount * 4;
            }
        } else {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    } else {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }

    return eStatus;
}
#endif
