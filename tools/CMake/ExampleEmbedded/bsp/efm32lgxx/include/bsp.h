/**
 * /bsp/efm32lgxx/bsp.h
 *
 * Copyright (C) 2018 HinsShum
 *
 * bsp.h is free software: you can redistribute it and/or modify
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
 */
#ifndef __BSP_H
#define __BSP_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- marco ----------*/
#define NVIC_PriorityGroup_0        (0x07) /*!< 0 bits for pre-emption priority
                                                            3 bits for subpriority */
#define NVIC_PriorityGroup_1        (0x06) /*!< 1 bits for pre-emption priority
                                                            2 bits for subpriority */
#define NVIC_PriorityGroup_2        (0x05) /*!< 2 bits for pre-emption priority
                                                            1 bits for subpriority */
#define NVIC_PriorityGroup_3        (0x04) /*!< 3 bits for pre-emption priority
                                                            0 bits for subpriority */

/* MCU reset cause flag */
#define RSTCAUSE_POR                (1U << 0)   /* cold startup */
#define RSTCAUSE_SYSREQ             (1U << 1)   /* system request reset */
#define RSTCAUSE_BOD                (1U << 2)   /* Brown-out has been detected */
#define RSTCAUSE_EXTWDO             (1U << 3)   /* external watch dog reset */

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void bsp_systick_deinit(void);
extern bool bsp_systick1ms_init(void);
extern void bsp_systick_inctick(void);
extern uint32_t bsp_systick_gettick(void);
extern void bsp_systick_isr(void);
extern bool bsp_init(void);
extern void bsp_deinit(void);
extern void bsp_udelay(uint32_t us);
extern void bsp_mdelay(uint32_t delay);
extern void bsp_exit_sleep(void);
extern uint8_t bsp_get_reset_cause(void);

#endif /* __BSP_H */
