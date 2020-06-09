/**
 * /bsp/efm32lgxx/bsp.c
 *
 * Copyright (C) 2018 HinsShum
 *
 * bsp.c is free software: you can redistribute it and/or modify
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

/*---------- includes ----------*/
#include "bsp.h"
#include "efm32lgxx_conf.h"
#include "config/include/attributes.h"

/*---------- marco ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static __IO uint32_t tick = 0;
static uint32_t clk_cycle = 0;

/*---------- function ----------*/
/**
 * bsp_systick1ms_init() - This function configures the Cortex-M3 SysTick source
 * to have 1ms time base.
 *
 * retval: None
 */
bool bsp_systick1ms_init(void)
{
    uint32_t hfcoreclk = 0;

    /* Configures the Cortex-M SysTick source to have 1ms time base. */
    hfcoreclk = CMU_ClockFreqGet(cmuClock_CORE);
    SysTick_Config(hfcoreclk / 1000U);

    /* calc clock cycle
     * @note: clk_cycle is used for bsp_udelay()
     */
    clk_cycle = hfcoreclk / 1000000U;

    return true;
}

/**
 * bsp_systick_enableit() - enable systick interrupt
 *
 * retval: None
 */
__STATIC_INLINE void __used bsp_systick_enableit(void)
{
    BUS_RegMaskedSet(&SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

/**
 * bsp_systick_disableit() - disable systick interrupt
 *
 * retval: None
 */
__STATIC_INLINE void __used bsp_systick_disableit(void)
{
    BUS_RegMaskedClear(&SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

/**
 * bsp_systick_enable() - Enable systick
 *
 * retval: None
 */
__STATIC_INLINE void bsp_systick_enable(void)
{
    BUS_RegMaskedSet(&SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
}

/**
 * bsp_systick_disable() - Disable systick.
 *
 * retval: None
 */
__STATIC_INLINE void bsp_systick_disable(void)
{
    BUS_RegMaskedClear(&SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
}

void bsp_systick_deinit(void)
{
    bsp_systick_disable();
    bsp_systick_disableit();
}

/**
 * bsp_systick_inctick() - This function is called to increment
 * a global variable "tick" used as application time base.
 * @note: In the default implementation, this variable is
 *      incremented each 1ms in Systick ISR.
 *
 * retval: None
 */
void bsp_systick_inctick(void)
{
    tick++;
}

/**
 * bsp_systick_gettick() - Provides a tick value in millisecond.
 *
 * retval: tick value
 */
uint32_t bsp_systick_gettick(void)
{
    return tick;
}

/**
 * bsp_systick_isr() - systick interrupt service function.
 *
 * retval: None
 */
void bsp_systick_isr(void)
{
}

bool bsp_init(void)
{
    extern uint32_t __svector;
    /* Relocate vector table */
#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | ((uint32_t)&__svector - SRAM_BASE);
#else
    SCB->VTOR = FLASH_BASE | ((uint32_t)&__svector - FLASH_BASE);
#endif
    /* This function must be called before flash operations */
    MSC_Init();

    return true;
}

void bsp_deinit(void)
{
    /* Disables the flash controller for writing */
    MSC_Deinit();
}

/**
 * bsp_udelay() - This function provides accurate delay (in microsecond) based on SysTick counter flag
 * @note: Blocking delay
 * @note: Based on the system HFCORECLK.
 * @note: The delay time length can not exceed the SysTick LOAD,
 *      it can be used to delay ms.
 * @note: the function can not delay less than 5us.
 *  for example:
 *      bsp_udelay(0) - the actual delay of 5us,
 *      bsp_udelay(1) - the actual delay of (5 + 1)us,
 *      bsp_udelay(10) - the actual delay of (5 + 10)us.
 * @us: Delay specifies the delay time length, in microsecond.
 *
 * retval: None
 */
void bsp_udelay(uint32_t us)
{
    uint32_t tmp1, tmp2, delta;

    tmp1 = SysTick->VAL;
    while(true) {
        tmp2 = SysTick->VAL;
        delta = tmp2 < tmp1 ? (tmp1 - tmp2) : (SysTick->LOAD - tmp2 + tmp1);
        /**
         * Here clk_cycle depends on the system clock
         */
        if(delta >= (us * clk_cycle)) {
            break;
        }
    }
}

/**
 * bsp_mdelay() - This function provides accurate delay (in
 * milliseconds) based on SysTick counter flag
 * @note: When a RTOS is used, it is recommended to avoid using
 *      blocking delay and use rather osDelay service.
 * @note: To respect 1ms timebase, user should call @ref
 *      LL_Init1msTick function which will configure Systick to
 *      1ms.
 * @delay: Delay specifies the delay time length, in
 *       milliseconds.
 *
 * retval:
 */
void bsp_mdelay(uint32_t delay)
{
    /* Clear the COUNTFLAG first */
    __IO uint32_t tmp = SysTick->CTRL;
    /* Add this code to indicate that local variable is not used */
    ((void)tmp);

    if(delay < 0xFFFFFFFFU) {
        delay++;
    }
    while(delay) {
        if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U) {
            delay--;
        }
    }
}

/**
 * bsp_exit_sleep() - Do not sleep when returning to Thread
 * mode.
 *
 * retval: None
 */
void bsp_exit_sleep(void)
{
}

/**
 * bsp_get_reset_cause() - get mcu reset cause and clear cause
 * register
 *
 * retval: reset cause
 */
uint8_t bsp_get_reset_cause(void)
{
    uint32_t cause = 0;
    uint8_t flag = 0;

    cause = RMU_ResetCauseGet();
    RMU_ResetCauseClear();

    if(cause & RMU_RSTCAUSE_PORST) {
        flag |= RSTCAUSE_POR;
    }
    if(cause & RMU_RSTCAUSE_EXTRST) {
        flag |= RSTCAUSE_EXTWDO;
    }
    if(cause & RMU_RSTCAUSE_SYSREQRST) {
        flag |= RSTCAUSE_SYSREQ;
    }
    if((cause & RMU_RSTCAUSE_BODUNREGRST)   ||
       (cause & RMU_RSTCAUSE_BODREGRST)     ||
       (cause & RMU_RSTCAUSE_BODAVDD0)      ||
       (cause & RMU_RSTCAUSE_BODAVDD1)      ||
       (cause & RMU_RSTCAUSE_BUBODVDDDREG)  ||
       (cause & RMU_RSTCAUSE_BUBODBUVIN)    ||
       (cause & RMU_RSTCAUSE_BUBODUNREG)    ||
       (cause & RMU_RSTCAUSE_BUBODREG)) {
        flag |= RSTCAUSE_BOD;
    }

    return flag;
}

