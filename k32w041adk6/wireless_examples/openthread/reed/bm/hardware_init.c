/*
* Copyright 2020 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "board.h"
#include "clock_config.h"
#include "fsl_power.h"

void BOARD_InitClocks(void)
{
    BOARD_BootClockRUN();
}

void hardware_init(void)
{
    BOARD_common_hw_init();
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
    // Keep on the FRO32K clock if it's used for 32k clock source
#if !gClkUseFro32K
    CLOCK_DisableClock(kCLOCK_Fro32k);
#endif
    /* Disable LDO ADC 1v1 */
    PMC -> PDRUNCFG &= ~PMC_PDRUNCFG_ENA_LDO_ADC_MASK;
    POWER_DisablePD(kPDRUNCFG_PD_LDO_ADC_EN);
    CLOCK_DisableClock(kCLOCK_Adc0);
}
