/**
 * /bsp/efm32lgxx/bsp_lpuart.c
 *
 * Copyright (C) 2018 HinsShum
 *
 * bsp_lpuart.c is free software: you can redistribute it and/or modify
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
#include "bsp_lpuart.h"
//#include "user_serial.h"
#include "printk.h"
#include "efm32lgxx_conf.h"
#include "config/include/attributes.h"

/*---------- marco ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static void bsp_lpuartio_init(void);
static void bsp_lpuartreg_init(void);
__STATIC_INLINE void bsp_lpuartnvic_init(void);
static void bsp_lpuart_register(void);
static void bsp_lpuart_unregister(void);
static void bsp_lpuart_lp_manage(bool on);

/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * bsp_lpuart_init() - Configure lpusart register and GPIO register.
 *
 * retval: true
 */
bool bsp_lpuart_init(void)
{
    bsp_lpuartio_init();
    bsp_lpuartreg_init();
    bsp_lpuartnvic_init();
    bsp_lpuart_register();

    return true;
}

/**
 * bsp_lpuart_deinit() - Disable lpusart.
 *
 * retval: None
 */
void bsp_lpuart_deinit(void)
{
    CMU_ClockEnable(cmuClock_LEUART1, false);
    LEUART_Enable(LEUART1, leuartDisable);
    NVIC_DisableIRQ(LEUART1_IRQn);
    LEUART_Reset(LEUART1);
    bsp_lpuart_unregister();
}

/**
 * bsp_lpuartio_init() - Configure lpusart gpio pin
 *
 * retval: None
 */
static void bsp_lpuartio_init(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);

    /* LEUART1 GPIO configuration
     * PA5  ---> LEUART1 TX
     * PA6  ---> LEUART1 RX
     */
    GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 6, gpioModeInput, 0);
}

/**
 * bsp_lpuartreg_init() - Configure LEUART register.

 * retval: None
 */
static void bsp_lpuartreg_init(void)
{
    LEUART_Init_TypeDef LEUART_InitStructure;

    CMU_ClockEnable(cmuClock_LEUART1, true);

    /* Configure LEUART1 */
    LEUART_InitStructure.enable = leuartDisable;
    LEUART_InitStructure.refFreq = 0;
    LEUART_InitStructure.baudrate = 115200U;
    LEUART_InitStructure.databits = leuartDatabits8;
    LEUART_InitStructure.parity = leuartNoParity;
    LEUART_InitStructure.stopbits = leuartStopbits1;
    LEUART_Init(LEUART1, &LEUART_InitStructure);

    /* Prepare LEUART1 RX interrupt */
    LEUART_IntClear(LEUART1, _LEUART_IFC_MASK);

    /* Enable I/O pins at LEUART1 location #1 */
    LEUART1->ROUTE = LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN | LEUART_ROUTE_LOCATION_LOC1;
    /* Enbale LEUART1 */
    LEUART_Enable(LEUART1, leuartEnable);
}

/**
 * bsp_lpuartnvic_init() - Configure lpuart nvic register.
 *
 * retval: None
 */
__STATIC_INLINE void bsp_lpuartnvic_init(void)
{
    NVIC_SetPriority(LEUART1_IRQn, 5);
    NVIC_ClearPendingIRQ(LEUART1_IRQn);
}

/**
 * bsp_lpuart_pwctl() - open or close lpuart
 * @on: true: enable lpuart
 *      false: disable lpuart
 *
 * retval: true
 */
static bool __unused bsp_lpuart_pwctl(bool on)
{
    if(on) {
        LEUART_IntEnable(LEUART1, LEUART_IEN_RXDATAV);
        NVIC_EnableIRQ(LEUART1_IRQn);
    } else {
        LEUART_IntDisable(LEUART1, LEUART_IEN_RXDATAV);
        NVIC_DisableIRQ(LEUART1_IRQn);
    }

    return true;
}

/**
 * bsp_lpuart_lp_manage() - select lpuart work normal or low
 * power mode
 * @note: LEUART0 and LEUART1 use the same clock source!
 * @on: true: low power mode
 *      false: normal mode
 *
 * retval: None
 */
static void __unused bsp_lpuart_lp_manage(bool on)
{
    LEUART_Init_TypeDef LEUART_InitStructure;

    if(on) {
        /* Reselct leuart1 clock source to accommodate low power mode */
        LEUART_Enable(LEUART0, leuartDisable);
        LEUART_Enable(LEUART1, leuartDisable);
        CMU_ClockEnable(cmuClock_LEUART0, false);
        CMU_ClockEnable(cmuClock_LEUART1, false);
        CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
        CMU_ClockEnable(cmuClock_LEUART1, true);
        /* Configure LEUART1 */
        LEUART_InitStructure.enable = leuartDisable;
        LEUART_InitStructure.refFreq = 0;
        LEUART_InitStructure.baudrate = 9600U;
        LEUART_InitStructure.databits = leuartDatabits8;
        LEUART_InitStructure.parity = leuartNoParity;
        LEUART_InitStructure.stopbits = leuartStopbits1;
        LEUART_Init(LEUART1, &LEUART_InitStructure);
        LEUART_Enable(LEUART1, leuartEnable);
    } else {
        /* Restore lpuart work normal */
        LEUART_Enable(LEUART1, leuartDisable);
        CMU_ClockEnable(cmuClock_LEUART1, false);
        CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
        CMU_ClockEnable(cmuClock_LEUART0, true);
        CMU_ClockEnable(cmuClock_LEUART1, true);
        /* Configure LEUART1 */
        LEUART_InitStructure.enable = leuartDisable;
        LEUART_InitStructure.refFreq = 0;
        LEUART_InitStructure.baudrate = 115200U;
        LEUART_InitStructure.databits = leuartDatabits8;
        LEUART_InitStructure.parity = leuartNoParity;
        LEUART_InitStructure.stopbits = leuartStopbits1;
        LEUART_Init(LEUART1, &LEUART_InitStructure);
        LEUART_Enable(LEUART0, leuartEnable);
        LEUART_Enable(LEUART1, leuartEnable);
    }
}

static void bsp_lpuart_putc(int8_t ch)
{
    if('\n' == ch) {
        LEUART_Tx(LEUART1, '\r');
    }
    LEUART_Tx(LEUART1, ch);
}

static uint32_t bsp_lpuart_puts(const char *str, uint32_t len)
{
    uint32_t i = 0;

    for(; (i < len) && str[i]; ++i) {
        bsp_lpuart_putc(str[i]);
    }

    return i;
}

static int16_t __unused bsp_lpuart_write(uint8_t *pbuf, int16_t len)
{
    for(uint16_t i = 0; i < len; ++i) {
        LEUART_Tx(LEUART1, pbuf[i]);
    }

    return len;
}

static void bsp_lpuart_register(void)
{
    console_driver.write = (unsigned int (*)(const char *, unsigned int))bsp_lpuart_puts;
    console_driver.getc = NULL;
}

static void bsp_lpuart_unregister(void)
{
    console_driver.write = NULL;
    console_driver.getc = NULL;
}


