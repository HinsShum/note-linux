/**
 * @file bsp\cy001\bsp_w25qxx.c
 *
 * Copyright (C) 2021
 *
 * bsp_w25qxx.c is free software: you can redistribute it and/or modify
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
#include "bsp_w25qxx.h"
#include "fm25qxx.h"
#include "stm32f1xx.h"
#include "stm32f1xx_ll_conf.h"

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static bool bsp_init(void);
static void bsp_deinit(void);
static void bsp_cs_ctrl(bool ctrl);
static uint8_t bsp_xfer(uint8_t data);
static bool bsp_block_start_check(uint32_t offset);

/*---------- type define ----------*/
/*---------- variable ----------*/
static fm25qxx_describe_t w25qxx = {
    .flash.start = 0,
    .flash.end = 0,
    .flash.block_size = 0,
    .flash.ops.init = bsp_init,
    .flash.ops.deinit = bsp_deinit,
    .flash.ops.write = NULL,
    .flash.ops.read = NULL,
    .flash.ops.erase_block = NULL,
    .flash.ops.erase_chip = NULL,
    .flash.ops.addr_is_block_start = bsp_block_start_check,
    .flash.ops.cb = NULL,
    .flash.ops.lock = NULL,
    .flash.ops.unlock = NULL,
    .idcode = 0,
    .cs_ctrl = bsp_cs_ctrl,
    .xfer = bsp_xfer
};
DEVICE_DEFINED(flash2, fm25qxx, &w25qxx);

/*---------- function ----------*/
static void bsp_io_init(void)
{
    LL_GPIO_InitTypeDef gpio_init = {0};

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    while(true != LL_APB2_GRP1_IsEnabledClock(LL_APB2_GRP1_PERIPH_GPIOB));
    /* GPIO configure
     * CS   ---> PB12
     * CLK  ---> PB13
     * MOSI ---> PB15
     * MISO ---> PB14
     */
    LL_GPIO_StructInit(&gpio_init);
    gpio_init.Pin = LL_GPIO_PIN_14;
    LL_GPIO_Init(GPIOB, &gpio_init);

    gpio_init.Pin = LL_GPIO_PIN_12;
    gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(GPIOB, &gpio_init);
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_14);

    gpio_init.Pin = LL_GPIO_PIN_13 | LL_GPIO_PIN_15;
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(GPIOB, &gpio_init);
}

static void bsp_spi_init(void)
{
    LL_SPI_InitTypeDef spi_init = {0};

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
    while(true != LL_APB1_GRP1_IsEnabledClock(LL_APB1_GRP1_PERIPH_SPI2));
    /* spi clock = 72M / 64 = 1.125M */
    LL_SPI_StructInit(&spi_init);
    spi_init.Mode = LL_SPI_MODE_MASTER;
    spi_init.ClockPolarity = LL_SPI_POLARITY_LOW;
    spi_init.ClockPhase = LL_SPI_PHASE_1EDGE;
    spi_init.NSS = LL_SPI_NSS_SOFT;
    spi_init.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV64;
    LL_SPI_Init(SPI2, &spi_init);
    LL_SPI_Enable(SPI2);
}

static bool bsp_init(void)
{
    bsp_io_init();
    bsp_spi_init();

    return true;
}

static void bsp_deinit(void)
{
    LL_SPI_Disable(SPI2);
    LL_SPI_DeInit(SPI2);
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_SPI2);
}

static void bsp_cs_ctrl(bool ctrl)
{
    if(ctrl) {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_12);
    } else {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);
    }
}

static uint8_t bsp_xfer(uint8_t data)
{
    while(true != LL_SPI_IsActiveFlag_TXE(SPI2));
    LL_SPI_TransmitData8(SPI2, data);
    while(true != LL_SPI_IsActiveFlag_RXNE(SPI2));
    return LL_SPI_ReceiveData8(SPI2);
}

static bool bsp_block_start_check(uint32_t offset)
{
    uint32_t addr = w25qxx.flash.start + offset;
    bool check = false;
    uint32_t blk_size = w25qxx.flash.block_size;

    if(addr >= w25qxx.flash.start && addr < w25qxx.flash.end) {
        check = ((addr & (blk_size - 1)) == 0);
    }

    return check;
}
