/**
 * @file arch\gd32e23x\board\cpu.c
 *
 * Copyright (C) 2021
 *
 * cpu.c is free software: you can redistribute it and/or modify
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
#include "cpu.h"
#include "config/options.h"
#include "gd32e23x.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
extern uint32_t Image$$ER_IROM1$$Base[];

/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static uint32_t clk_cycle = 0;
static uint64_t __IO __tick = 0;

/*---------- function ----------*/
static void config_systick(void)
{
    SysTick_Config(SystemCoreClock / 1000U);
    NVIC_SetPriority(SysTick_IRQn, CONFIG_PRIORITY_SYSTICK);
    /* calc clock cycle
     * clk_cycle is used for udelay()
     */
    clk_cycle = SystemCoreClock / 1000000U;
}

static void cpu_system_clock_config(void)
{
    /* deinit RCU */
    rcu_deinit();
    /* Config RCU
     *   select HXTAL(8M) as PREDV clock source
     *   PREDV = 1
     *   PLL = (HXTAL / PREDV) * 9 = (8M / 1) * 9 = 72M
     *   select PLL as system clock source
     *   AHB = PLL = 72M
     *   APB1 = AHB = 72M
     *   APB2 = AHB = 72M
     */
    /* enable HXTAL */
    rcu_osci_on(RCU_HXTAL);
    /* wait HTXAL stable */
    while(SUCCESS != rcu_osci_stab_wait(RCU_HXTAL));
    rcu_hxtal_prediv_config(RCU_PLL_PREDV1);
    rcu_pll_config(RCU_PLLSRC_HXTAL, RCU_PLL_MUL9);
    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV1);
    rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);
    /* enable PLL */
    rcu_osci_on(RCU_PLL_CK);
    /* wait PLL stable */
    while(SUCCESS != rcu_osci_stab_wait(RCU_PLL_CK));
    /* according to system clock frequency, adjust fmc waiting time
     * system_clock <= 24M, FMC waiting time = 0
     * system_clock <= 48M, FMC waiting time = 1
     * system_clock <= 72M, FMC waiting time = 2
     */
    fmc_wscnt_set(WS_WSCNT_2);
    /* select pll as system clock source */
    rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);
    while(RCU_SCSS_PLL != rcu_system_clock_source_get());
    /* update system_clock variable */
    SystemCoreClockUpdate();
}

void cpu_config(void)
{
    /* configure system clock frequency up to 108M */
    cpu_system_clock_config();
    /* relocate vector table */
#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | ((uint32_t)Image$$ER_IROM1$$Base - SRAM_BASE);
#else
    SCB->VTOR = FLASH_BASE | ((uint32_t)Image$$ER_IROM1$$Base - FLASH_BASE);
#endif
    /* initialize systick */
    config_systick();
}

void cpu_restore(void)
{
    /* deinit RCU */
    rcu_deinit();
    /* close systick */
    SysTick->CTRL &= (~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk));
}

void udelay(__IO uint32_t us)
{
    uint32_t tmp1, tmp2, delta;

    tmp1 = SysTick->VAL;
    while(true) {
        tmp2 = SysTick->VAL;
        delta = tmp2 < tmp1 ? (tmp1 - tmp2) : (SysTick->LOAD - tmp2 + tmp1);
        if(delta >= (us * clk_cycle)) {
            break;
        }
    }
}

void mdelay(uint32_t delay)
{
    /* Clear the COUNTFLAG first */
    __IO uint32_t tmp = SysTick->CTRL;
    /* Add this code to indicate that local variable is not used */
    ((void)tmp);

    while(delay) {
        if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U) {
            delay--;
        }
    }
}

void tick_increase(void)
{
    __tick++;
}

uint64_t tick_get(void)
{
    return __tick;
}
