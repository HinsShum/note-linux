/**
 * @file bsp/efm32lgxx/bsp_w5500.c
 *
 * Copyright (C) 2019
 *
 * bsp_w5500.c is free software: you can redistribute it and/or modify
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
 */

/*---------- includes ----------*/
#include "bsp_w5500.h"
#include "efm32lgxx_conf.h"
#include "config/include/attributes.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * @brief control cs pin
 * @param on: ture  --> low(selected)
 *            false --> high(release)
 *
 * @retval Null
 */
static void __inline bsp_w5500_spi_cs_ctl(bool on)
{
    on == true ? GPIO_PinOutClear(gpioPortC, 5) : GPIO_PinOutSet(gpioPortC, 5);
}

/**
 * @brief exchange data with the w5500
 * @param data: data that to the w5500
 *
 * @retval data that get from the w5500
 */
static uint8_t __inline bsp_w5500_xfer(uint8_t data)
{
    return USART_SpiTransfer(USART2, data);
}

/**
 * @brief select the w5500
 *
 * @retval Null
 */
void bsp_w5500_cs_select(void)
{
    bsp_w5500_spi_cs_ctl(true);
}

/**
 * @brief deselect the w5500
 *
 * @retval Null
 */
void bsp_w5500_cs_deselect(void)
{
    bsp_w5500_spi_cs_ctl(false);
}

/**
 * @brief get a byte from the w5500
 *
 * @retval the byte that get from the w5500
 */
uint8_t bsp_w5500_getbyte(void)
{
    return bsp_w5500_xfer(0x00);
}

/**
 * @brief send a byte to the w5500
 * @param byte: the byte that send to the w5500
 *
 * @retval Null
 */
void bsp_w5500_sendbyte(uint8_t byte)
{
    bsp_w5500_xfer(byte);
}

/**
 * @brief control w5500 reset pin
 * @param on: true  --> pull up
 *            false --> pull down
 *
 * @retval Null
 */
void bsp_w5500_reset_ctl(bool on)
{
    on == true ? GPIO_PinOutSet(gpioPortB, 6) : GPIO_PinOutClear(gpioPortB, 6);
}

/**
 * @brief gpio initialize to driver the w5500
 *
 * @retval Null
 */
static void bsp_w5500_ioinit(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);

    /* Configure SPI GPIO
     * CS   --->    PC5
     * SCLK --->    PC4
     * MISO --->    PC3
     * MOSI --->    PC2
     */
    GPIO_PinModeSet(gpioPortC, 5, gpioModePushPull, 1);
    GPIO_PinModeSet(gpioPortC, 4, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortC, 3, gpioModeInput, 0);
    GPIO_PinModeSet(gpioPortC, 2, gpioModePushPull, 0);
    /* Reset Pin */
    GPIO_PinModeSet(gpioPortB, 6, gpioModePushPull, 0);
    /* Interrupt Pin */
    GPIO_PinModeSet(gpioPortC, 10, gpioModeInput, 0);
    GPIO_IntConfig(gpioPortC, 10, false, true, true);
}

/**
 * @brief spi register initialize to driver the w5500
 *
 * @retval Null
 */
static void bsp_w5500_reginit(void)
{
    USART_InitSync_TypeDef USART_InitSyncStructure;

    CMU_ClockEnable(cmuClock_USART2, true);

    /* Configure USART2 */
    USART_InitSyncStructure.enable = usartDisable;
    USART_InitSyncStructure.refFreq = 0;
    USART_InitSyncStructure.baudrate = 1000000U;
    USART_InitSyncStructure.databits = usartDatabits8;
    USART_InitSyncStructure.master = true;
    USART_InitSyncStructure.msbf = true;
    USART_InitSyncStructure.clockMode = usartClockMode3;
    USART_InitSyncStructure.prsRxEnable = false;
    USART_InitSyncStructure.prsRxCh = usartPrsRxCh0;
    USART_InitSyncStructure.autoTx = false;
    USART_InitSync(USART2, &USART_InitSyncStructure);

    /* Enable I/O pins at USART2 location #0 */
    USART2->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN | USART_ROUTE_LOCATION_LOC0;
    USART2->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
    USART_Enable(USART2, usartEnable);
}

/**
 * @brief initialize interrput Pin NVIC controller
 *
 * @retval Null
 */
static void bsp_w5500_nvic_init(void)
{
    NVIC_SetPriority(GPIO_EVEN_IRQn, 4);
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}

/**
 * @brief initialize the gpio registers and spi registers
 *
 * @retval true
 */
bool bsp_w5500_init(void)
{
    bsp_w5500_ioinit();
    bsp_w5500_reginit();
    bsp_w5500_nvic_init();

    return true;
}

/**
 * @brief reset spi registers and close spi clock
 *
 * @retval Null
 */
void bsp_w5500_deinit(void)
{
    CMU_ClockEnable(cmuClock_USART2, false);

    USART_Reset(USART2);
}

