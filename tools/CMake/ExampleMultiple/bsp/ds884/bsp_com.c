/**
 * @file bsp\ds884\bsp_com.c
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
#include "gd32e23x.h"
#include "config/options.h"

/*---------- macro ----------*/
#define ONE_BYTE_NEED_TIME_US(baudrate)                         ((1000000UL * 11) / baudrate)
#define __BSP_DEFAULT_BAUDRATE                                  (116200)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static bool bsp_init(void);
static void bsp_deinit(void);
static void bsp_dir_change(serial_direction_en dir);
static uint16_t bsp_write(uint8_t *pbuf, uint16_t len);

/*---------- type define ----------*/
/*---------- variable ----------*/
static serial_describe_t com = {
    .comport = 0,
    .baudrate = __BSP_DEFAULT_BAUDRATE,
    .ops.init = bsp_init,
    .ops.deinit = bsp_deinit,
    .ops.dir_change = bsp_dir_change,
    .ops.write = bsp_write,
    .ops.irq_handler = NULL
};
DEVICE_DEFINED(com, serial, &com);

/*---------- function ----------*/
static void bsp_io_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);

    /* TX: GPIOA_9
     * RX: GPIOA_10
     * DE: GPIOA_8
     */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
    /* output configure */
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);
    /* connect port to usart */
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_9);
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_10);
    /* set com as receiving state */
    gpio_bit_reset(GPIOA, GPIO_PIN_8);
}

static void bsp_uart_init(void)
{
    rcu_periph_clock_enable(RCU_USART0);

    /* configure usart0 */
    usart_deinit(USART0);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_baudrate_set(USART0, com.baudrate);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_interrupt_enable(USART0, USART_INT_RBNE);
    usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);
    usart_enable(USART0);
}

static void bsp_nvic_init(void)
{
    NVIC_ClearPendingIRQ(USART0_IRQn);
    NVIC_SetPriority(USART0_IRQn, CONFIG_PRIORITY_COM);
    NVIC_EnableIRQ(USART0_IRQn);
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
    usart_disable(USART0);
    usart_deinit(USART0);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_10);
    gpio_bit_set(GPIOA, GPIO_PIN_8);
    gpio_bit_set(GPIOA, GPIO_PIN_9);
    gpio_bit_set(GPIOA, GPIO_PIN_10);
}

static void bsp_dir_change(serial_direction_en dir)
{
    switch(dir) {
        case SERIAL_DIRECTION_TX:
            gpio_bit_set(GPIOA, GPIO_PIN_8);
            break;
        case SERIAL_DIRECTION_RX:
            if(SET == gpio_output_bit_get(GPIOA, GPIO_PIN_8)) {
                uint32_t ms = (ONE_BYTE_NEED_TIME_US(com.baudrate) / 1000) + 1;
                __delay_ms(ms);
            }
            gpio_bit_reset(GPIOA, GPIO_PIN_8);
            break;
        case SERIAL_DIRECTION_NRX_NTX:
            break;
        default:
            break;
    }
}

static uint16_t bsp_write(uint8_t *pbuf, uint16_t len)
{
    for(uint16_t i = 0; i < len; ++i) {
        usart_data_transmit(USART0, pbuf[i]);
        while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
    }

    return len;
}
