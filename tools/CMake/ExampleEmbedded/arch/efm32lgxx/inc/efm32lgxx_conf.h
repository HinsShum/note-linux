/**
 * /arch/efm32lgxx/efm32lgxx_conf.h
 *
 * Copyright (C) 2018 HinsShum
 *
 * efm32lgxx_conf.h is free software: you can redistribute it and/or modify
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
#ifndef __EFM32LGXX_CONF_H
#define __EFM32LGXX_CONF_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Uncomment/Comment the line below to enable/disable peripheral header file inclusion */
//#include "em_acmp.h"
#include "em_adc.h"
//#include "em_aes.h"
//#include "em_burtc.h"
#include "em_bus.h"
//#include "em_can.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_common.h"
#include "em_core.h"
//#include "em_cryotimer.h"
//#include "em_crypto.h"
//#include "em_csen.h"
//#include "em_dac.h"
//#include "em_dbg.h"
//#include "em_dma.h"
//#include "em_ebi.h"
#include "em_emu.h"
//#include "em_gpcrc.h"
#include "em_gpio.h"
//#include "em_i2c.h"
//#include "em_idac.h"
//#include "em_int.h"
//#include "em_lcd.h"
//#include "em_ldma.h"
//#include "em_lesense.h"
#include "em_letimer.h"
#include "em_leuart.h"
//#include "em_mpu.h"
#include "em_msc.h"
//#include "em_opamp.h"
//#include "em_pcnt.h"
//#include "em_prs.h"
//#include "em_qspi.h"
//#include "em_ramfunc.h"
#include "em_rmu.h"
//#include "em_rtc.h"
//#include "em_rtcc.h"
//#include "em_smu.h"
#include "em_system.h"
//#include "em_timer.h"
#include "em_usart.h"
//#include "em_vcmp.h"
//#include "em_vdac.h"
#include "em_version.h"
//#include "em_wdog.h"

/*---------- marco ----------*/
#ifdef USE_FULL_ASSERT
#include "em_assert.h"
#else
#define assert_param(expr)  ((void)0U)
#endif

#define __aligned_1     __attribute__((packed))

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
#endif /* __EFM32LGXX_CONF_H */
