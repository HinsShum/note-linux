/**
 * /arch/efm32lgxx/cpu_freq.c
 *
 * Copyright (C) 2018 HinsShum
 *
 * cpu_freq.c is free software: you can redistribute it and/or modify
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
#include "cpu_freq.h"
#include "efm32lgxx_conf.h"

/*---------- marco ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * cpu_freq_config() - Initializes the Flash interface and
 * configure the system clock.
 *
 * retval: None
 */
void cpu_freq_config(void)
{
    /*
     * This function must be called immediately,
     * API to initialize chip for errata workarounds.
     */
    CHIP_Init();
    /* Enable AUXFRCO */
    CMU_OscillatorEnable(cmuOsc_AUXHFRCO, true, true);
    /* Enable HFXO */
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    /* Select HF clock source */
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
    /* Prescaling HF clock */
    CMU_ClockDivSet(cmuClock_HF, cmuClkDiv_1);
    /* Prescaling HF peripheral clock sub-branch */
    CMU_ClockDivSet(cmuClock_HFPER, cmuClkDiv_1);
    /* Enable HFPER clock */
    CMU_ClockEnable(cmuClock_HFPER, true);
    /* Prescaling HF core sub-branch */
    CMU_ClockDivSet(cmuClock_CORE, cmuClkDiv_1);
    /* Get system core clock */
    SystemCoreClockGet();
    /* Enable HFCORECLKLE */
    CMU_ClockEnable(cmuClock_HFLE, true);
    /* Select LFB clock source */
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
    /* Enable LFXO */
    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
    /* LFXO mode higher driving capability */
    BUS_RegMaskedWrite(&CMU->CTRL, _CMU_CTRL_LFXOBOOST_MASK,
                       ((uint32_t)(!!(uint32_t)cmuLfxoBoost100)) << _CMU_CTRL_LFXOBOOST_SHIFT);
    /* Select LFA clock source */
    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
    /*
     * Close HFRCO
     * HFRCO will startup then core restart
     * as HF clock source
     */
    CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);

}
