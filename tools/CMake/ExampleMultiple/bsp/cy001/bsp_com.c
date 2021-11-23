/**
 * @file bsp/cy001/bsp_com.c
 *
 * Copyright (C) 2021
 *
 * bsp_com.c is free software: you can redistribute it and/or modify
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
#include "bsp_com.h"
#include "serial.h"
#include "stm32f1xx.h"
#include "config/options.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static bool bsp_init(void);
static void bsp_deinit(void);
static uint16_t bsp_write(uint8_t *pbuf, uint16_t len);

/*---------- type define ----------*/
/*---------- variable ----------*/
static serial_describe_t com = {
    .comport = 1,
    .baudrate = 115200,
    .ops.init = bsp_init,
    .ops.deinit = bsp_deinit,
    .ops.dir_change = NULL,
    .ops.write = bsp_write,
    .ops.irq_handler = NULL
};
DEVICE_DEFINED(com, serial, &com);

/*---------- function ----------*/
static void bsp_io_init(void)
{
    LL_GPIO_InitTypeDef gpio_init_structure;

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    while(true != LL_APB2_GRP1_IsEnabledClock(LL_APB2_GRP1_PERIPH_GPIOA));
    /* GPIO configure
     * PA9  ---> TX
     * PA10 ---> RX
     */
    LL_GPIO_StructInit(&gpio_init_structure);
    gpio_init_structure.Pin = LL_GPIO_PIN_10;
    LL_GPIO_Init(GPIOA, &gpio_init_structure);

    gpio_init_structure.Pin = LL_GPIO_PIN_9;
    gpio_init_structure.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init_structure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOA, &gpio_init_structure);
}

static void bsp_uart_init(void)
{
    LL_USART_InitTypeDef uart_init_structure;

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    while(true != LL_APB2_GRP1_IsEnabledClock(LL_APB2_GRP1_PERIPH_USART1));

    LL_USART_StructInit(&uart_init_structure);
    uart_init_structure.BaudRate = com.baudrate;
    LL_USART_Init(USART1, &uart_init_structure);
    LL_USART_ConfigAsyncMode(USART1);

    LL_USART_ClearFlag_RXNE(USART1);
    LL_USART_EnableIT_RXNE(USART1);
    LL_USART_Enable(USART1);
}

static void bsp_nvic_init(void)
{
    NVIC_ClearPendingIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, CONFIG_PRIORITY_COM);
    NVIC_EnableIRQ(USART1_IRQn);
}

static bool bsp_init(void)
{
    bsp_io_init();
    bsp_uart_init();
    bsp_nvic_init();

    return true;
}

static void bsp_deinit(void)
{
    LL_GPIO_InitTypeDef gpio_init = {0};

    NVIC_DisableIRQ(USART1_IRQn);
    LL_USART_Disable(USART1);
    LL_USART_DeInit(USART1);
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART1);

    gpio_init.Pin = LL_GPIO_PIN_9 | LL_GPIO_PIN_10;
    gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
    gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio_init.Pull = LL_GPIO_PULL_DOWN;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
    LL_GPIO_Init(GPIOA, &gpio_init);
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_9 | LL_GPIO_PIN_10);
}

static uint16_t bsp_write(uint8_t *pbuf, uint16_t len)
{
    for(uint16_t i = 0; i < len; ++i) {
        LL_USART_TransmitData8(USART1, pbuf[i]);
        while(SET != LL_USART_IsActiveFlag_TC(USART1));
    }

    return len;
}
