/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*${header:start}*/
#include "pin_mux.h"
#include "board.h"
#include <stdbool.h>
#include "fsl_power.h"


void BOARD_InitClocks(void)
{
    /* Already set to mainCLK by default */
    CLOCK_AttachClk(kMAIN_CLK_to_ASYNC_APB);

    /* 32MHz clock */
    CLOCK_EnableClock(kCLOCK_Xtal32M);      // Only required if CPU running from 32MHz crystal
    CLOCK_EnableClock(kCLOCK_Fro32M);       // derived from 198MHz FRO
    CLOCK_EnableClock(kCLOCK_Xtal32k);

    /* 32KHz clock */
#if !gLoggingActive_d
    CLOCK_EnableClock(CLOCK_32k_source);
#endif

    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_InputMux);

    /* Enable GPIO for LED controls */
    CLOCK_EnableClock(kCLOCK_Gpio0);

    /* Enable ADC clock */
    CLOCK_EnableClock(kCLOCK_Adc0);
    /* Power on the ADC converter. */
    POWER_EnablePD(kPDRUNCFG_PD_LDO_ADC_EN);

    /* needed for Timer manager if RTC is used */
    //CLOCK_EnableClock(kCLOCK_Rtc);

    /* INMUX and IOCON are used by many apps, enable both INMUX and IOCON clock bits here. */
    CLOCK_AttachClk(kOSC32M_to_USART_CLK);
    CLOCK_EnableClock(kCLOCK_Usart0);
    CLOCK_EnableClock(kCLOCK_Usart1);

    CLOCK_EnableClock(kCLOCK_Aes);
    CLOCK_EnableClock(kCLOCK_I2c0) ;
    CLOCK_AttachClk(kOSC32M_to_I2C_CLK);

    /* WWDT clock config (32k oscillator, no division) */
    CLOCK_AttachClk(kOSC32K_to_WDT_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivWdtClk, 1, true);

    /* Attach the ADC clock. */
    CLOCK_AttachClk(kXTAL32M_to_ADC_CLK);
    /* Enable LDO ADC 1v1 */
    PMC -> PDRUNCFG |= PMC_PDRUNCFG_ENA_LDO_ADC_MASK;

    // Alternative setting for UART
    //CLOCK_AttachClk(FRG_CLK_to_USART_CLK);
    //CLOCK_AttachClk(kOSC32M_to_FRG_CLK);

    //CLOCK_AttachClk(kOSC32M_to_I2C_CLK);
    //CLOCK_AttachClk(kFRO48M_to_DMI_CLK);
    //CLOCK_AttachClk(kMAIN_CLK_to_DMI_CLK);

}

void BOARD_SetClockForPowerMode(void)
{
    // Comment line if some PRINTF is still done prior to this step before sleep
    CLOCK_AttachClk(kNONE_to_USART_CLK);
    CLOCK_DisableClock(kCLOCK_Iocon);
    CLOCK_DisableClock(kCLOCK_InputMux);
    CLOCK_DisableClock(kCLOCK_Gpio0);
    CLOCK_DisableClock(kCLOCK_Aes);
    CLOCK_DisableClock(kCLOCK_I2c0);
    CLOCK_DisableClock(kCLOCK_Usart0);
    CLOCK_DisableClock(kCLOCK_Usart1);
    /* Disable LDO ADC 1v1 */
    PMC -> PDRUNCFG &= ~PMC_PDRUNCFG_ENA_LDO_ADC_MASK;
    POWER_DisablePD(kPDRUNCFG_PD_LDO_ADC_EN);
    CLOCK_DisableClock(kCLOCK_Adc0);
}

void hardware_init(void)
{
    BOARD_common_hw_init();
    //TODO check that BOARD_k32w061 do the zigbee work done here APP_vSetUpHardware in app_main.c
    //then remove it
}
/*${function:end}*/
