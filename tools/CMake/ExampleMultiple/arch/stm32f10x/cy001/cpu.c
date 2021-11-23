/**
 * @file arch\stm32f10x\cy001\cpu.c
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
#include "stm32f1xx.h"
#include "stm32f1xx_ll_conf.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$ER_IROM1$$Base[];
#elif defined(__GNUC__)
extern uint32_t _sisr_vector;
#else
#error "The compiler not armcc, armclang or gcc"
#endif

/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static uint32_t clk_cycle = 0;
static uint64_t __IO _ticks = 0;

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
    LL_UTILS_ClkInitTypeDef clk_init = {0};
    LL_UTILS_PLLInitTypeDef pll_init = {0};

    /* deinit RCC */
    LL_RCC_DeInit();
    /* enable prefetch buffer */
    LL_FLASH_EnablePrefetch();
    /* After a system reset, the HSI oscollator will be selected as system clock.
     * If config System clock from HSE failed, automatic switch to HSI(MAX: 64MHz)
     * HSI will keep enable, it will be used when operate embedded flash memory
     * AHB_CLK = PLL_CLK/1
     * APB1_CLK = AHB_CLK/2
     * APB2_CLK = AHB_CLK/1
     * APB1_PERIPHERAL = APB1_CLK
     * APB1_TIMER = APB1_CLK * 2
     * APB2_PERIPHERAL = APB2_CLK
     * APB2_TIMER = APB2_CLK
     */
#if HSE_VALUE > 8000000
    pll_init.Prediv = LL_RCC_PREDIV_DIV_2;
#else
    pll_init.Prediv = LL_RCC_PREDIV_DIV_1;
#endif
    pll_init.PLLMul = LL_RCC_PLL_MUL_9;
    clk_init.AHBCLKDivider = LL_RCC_SYSCLK_DIV_1;
    clk_init.APB1CLKDivider = LL_RCC_APB1_DIV_2;
    clk_init.APB2CLKDivider = LL_RCC_APB2_DIV_1;
    if(SUCCESS != LL_PLL_ConfigSystemClock_HSE(HSE_VALUE, LL_UTILS_HSEBYPASS_OFF, &pll_init, &clk_init)) {
        pll_init.PLLMul = LL_RCC_PLL_MUL_16;
        while(SUCCESS != LL_PLL_ConfigSystemClock_HSI(&pll_init, &clk_init));
    }
    /* close ohter no use oscallator */
    LL_RCC_LSI_Disable();
    LL_RCC_LSE_Disable();
    /* update system clock variable */
    SystemCoreClockUpdate();
}

void cpu_config(void)
{
    uint32_t reloacte = 0;

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    reloacte = (uint32_t)Image$$ER_IROM1$$Base;
#elif defined(__GNUC__)
    reloacte = (uint32_t)&_sisr_vector;
#endif
    /* configure system clock frequency up to 72M */
    cpu_system_clock_config();
    /* relocate vector table */
#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | (reloacte - SRAM_BASE);
#else
    SCB->VTOR = FLASH_BASE | (reloacte - FLASH_BASE);
#endif
    /* set nvic group */
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    /* enable afio clock */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
    while(true != LL_APB2_GRP1_IsEnabledClock(LL_APB2_GRP1_PERIPH_AFIO));
    /* initialize systick */
    config_systick();
}

void cpu_restore(void)
{
    /* deinit RCC */
    LL_RCC_DeInit();
    /* disable afio clock */
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_AFIO);
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
    _ticks++;
}

uint64_t tick_get(void)
{
    return _ticks;
}

uint32_t HAL_GetTick(void)
{
    return (uint32_t)_ticks;
}
