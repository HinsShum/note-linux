/**
 * /arch/efm32lgxx/efm32lgxx_it.c
 *
 * Copyright (C) 2018 HinsShum
 *
 * efm32lgxx_it.c is free software: you can redistribute it and/or modify
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
#include "efm32lgxx_it.h"
#include "efm32lgxx_conf.h"

/*---------- marco ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                        */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
__attribute__((weak)) void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
__attribute__((weak)) void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
__attribute__((weak)) void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32L0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_efm32lg.s).                                                 */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  This function handles UART1 RX interrupt request.
  * @param  None
  * @retval None
  */
void UART1_RX_IRQHandler(void)
{
    uint8_t ch = 0x00;

    if(UART_IF_RXUF == (USART_IntGet(UART1) & UART_IF_RXUF)) {
        USART_IntClear(UART1, UART_IFC_RXUF);
    }
    if(UART_IF_RXOF == (USART_IntGet(UART1) & UART_IF_RXOF)) {
        USART_IntClear(UART1, UART_IFC_RXOF);
    }
    if(UART_IF_RXDATAV == (USART_IntGet(UART1) & UART_IF_RXDATAV)) {
        ch = USART_RxDataGet(UART1);
        (void)ch;
    }
}

/**
  * @brief  This function handles UART0 RX interrupt request.
  * @param  None
  * @retval None
  */
void UART0_RX_IRQHandler(void)
{
    uint8_t ch = 0x00;

    if(UART_IF_RXUF == (USART_IntGet(UART0) & UART_IF_RXUF)) {
        USART_IntClear(UART0, UART_IFC_RXUF);
    }
    if(UART_IF_RXOF == (USART_IntGet(UART0) & UART_IF_RXOF)) {
        USART_IntClear(UART0, UART_IFC_RXOF);
    }
    if(UART_IF_RXDATAV == (USART_IntGet(UART0) & UART_IF_RXDATAV)) {
        ch = USART_RxDataGet(UART0);
        (void)ch;
    }
}

/**
  * @brief  This function handles LEUART0 interrupt request.
  * @param  None
  * @retval None
  */
void LEUART0_IRQHandler(void)
{
    uint8_t data;

    if(LEUART_IF_RXUF == (LEUART_IntGet(LEUART0) & LEUART_IF_RXUF)) {
        LEUART_IntClear(LEUART0, LEUART_IFC_RXUF);
    }
    if(LEUART_IF_RXOF == (LEUART_IntGet(LEUART0) & LEUART_IF_RXOF)) {
        LEUART_IntClear(LEUART0, LEUART_IFC_RXOF);
    }
    if(LEUART_IF_RXDATAV == (LEUART_IntGet(LEUART0) & LEUART_IF_RXDATAV)) {
        data = LEUART_RxDataGet(LEUART0);
        (void)data;
    }
}

/**
  * @brief  This function handles LEUART1 interrupt request.
  * @param  None
  * @retval None
  */
void LEUART1_IRQHandler(void)
{
    uint8_t data;

    if(LEUART_IF_RXUF == (LEUART_IntGet(LEUART1) & LEUART_IF_RXUF)) {
        LEUART_IntClear(LEUART0, LEUART_IFC_RXUF);
    }
    if(LEUART_IF_RXOF == (LEUART_IntGet(LEUART1) & LEUART_IF_RXOF)) {
        LEUART_IntClear(LEUART0, LEUART_IFC_RXOF);
    }
    if(LEUART_IF_RXDATAV == (LEUART_IntGet(LEUART1) & LEUART_IF_RXDATAV)) {
        data = LEUART_RxDataGet(LEUART1);
        (void)data;
    }
}

/**
  * @brief  This function handles GPIO interrupt request.
  * @param  None
  * @retval None
  */
void GPIO_EVEN_IRQHandler(void)
{
    extern void w5500_interrupt_handler(void);

    /* W5500 interrupt Pin PC10 */
    if(GPIO_IntGet() & (1U << 10)) {
        GPIO_IntClear(1U << 10);
        w5500_interrupt_handler();
    }
}

/**
  * @brief  This function handles LPTIM interrupt request.
  * @param  None
  * @retval None
  */
void LETIMER0_IRQHandler(void)
{
    if(LETIMER_IF_UF == (LETIMER_IntGet(LETIMER0) & LETIMER_IF_UF)) {
        LETIMER_IntClear(LETIMER0, LETIMER_IFC_UF);
    }
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Sluan *****END OF FILE****/

