/**
 * @file common/include/simplefifo.h
 *
 * Copyright (C) 2021
 *
 * simplefifo.h is free software: you can redistribute it and/or modify
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
#ifndef __SIMPLEFIFO_H
#define __SIMPLEFIFO_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief new a fifo handler
 * 
 * @retval fifo handler pointer
 */
extern void *simplefifo_new(void);

/**
 * @brief delete a fifo handler
 * @param handler: fifo handler pointer
 * 
 * @retval None
 */
extern void simplefifo_delete(void *handler);

/**
 * @brief initialize specified simplefifo by handler pointer
 * @param handler: simple fifo handler pointer
 * @param pbuf: specified buffer is bound to simple fifo
 * @param size: specified buffer size
 * 
 * @retval CY_E_WRONG_ARGS: handler pointer is not created by the function `simplefifo_new`
 *         CY_EOK: initialized successfully
 */
extern int32_t simplefifo_init(void *handler, uint8_t *pbuf, uint16_t size);

/**
 * @brief get the length of data stored in the simple fifo
 * @param handler: simple fifo handler pointer
 * 
 * @retval the length of the data 
 */
extern uint16_t simplefifo_get_size(void *handler);

/**
 * @brief reset the simple fifo
 * @param handler: simple fifo handler pointer
 * 
 * @retval None
 */
extern void simplefifo_reset(void *handler);

/**
 * @brief write data to the simple fifo
 * @param handler: simple fifo handler pointer
 * @param pbuf: the data will be stored to the simple fifo
 * @param length: the data length
 * 
 * @retval the acutal length of data stored
 */ 
extern uint16_t simplefifo_write(void *handler, const uint8_t *pbuf, uint16_t length);

/**
 * @brief read the specified length of data from simple fifo
 * @param handler: simple fifo handler pointer
 * @param pbuf: the buffer to store the data read from the simple fifo
 * @param length: the length of data you want to read
 * 
 * @retval the actual length of data read from the simple fifo
 */
extern uint16_t simplefifo_read(void *handler, uint8_t *pbuf, uint16_t length);

/**
 * @brief get simple fifo avaiable size
 * @param handler: simple fifo handler pointer
 * 
 * @retval the avaiable size of simple fifo
 */
extern uint16_t simplefifo_get_avaiable(void *handler);

#endif /* __SIMPLEFIFO_H */
