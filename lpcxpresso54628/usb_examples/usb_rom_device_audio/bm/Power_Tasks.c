// KWW - new file

/*
 * @brief I2C interface (header file) to DW8904
 *
 * @note
 * Copyright  2015, NXP
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "fsl_common.h"
#include "board.h"
#include "Power_Tasks.h"
#include "audio_codec.h"
#include "i2s_if.h"
#include "fsl_iocon.h"
//#include "power_lib_5411x.h"
#include "fsl_power.h"

volatile unsigned int powerFlags;

uint32_t power_GetMainClock()
{
    SystemCoreClockUpdate();
    return SystemCoreClock * ((SYSCON->AHBCLKDIV & 0xFF) + 1);
}

clock_attach_id_t power_GetMainClockSource()
{
    clock_attach_id_t clkSource = kNONE_to_NONE;

    switch (SYSCON->MAINCLKSELB & SYSCON_MAINCLKSELB_SEL_MASK)
    {
        case 0x00:
            switch (SYSCON->MAINCLKSELA & SYSCON_MAINCLKSELA_SEL_MASK)
            {
                case 0x00: /* FRO 12 MHz (fro_12m) */
                    clkSource = kFRO12M_to_MAIN_CLK;
                    break;
                case 0x01: /* CLKIN (clk_in) */
                    clkSource = kEXT_CLK_to_MAIN_CLK;
                    break;
                case 0x02: /* Watchdog oscillator (wdt_clk) */
                    clkSource = kWDT_OSC_to_MAIN_CLK;
                    break;
                default: /* = 0x03 = FRO 96 or 48 MHz (fro_hf) */
                    clkSource = kFRO_HF_to_MAIN_CLK;
                    break;
            }
            break;
        case 0x02: /* System PLL clock (pll_clk)*/
            clkSource = kSYS_PLL_to_MAIN_CLK;
            break;
        case 0x03: /* RTC oscillator 32 kHz output (32k_clk) */
            clkSource = kOSC32K_to_MAIN_CLK;
            break;
        default:
            break;
    }

    return clkSource;
}

/* Additional low power setup */
void Power_Init(void)
{
    uint32_t portIndex;
    uint32_t pinIndex;
    gpio_pin_config_t pinConfig;

/* OK to use a lower system clock with DMA */
#if LOWPOWERCLOCKMODE == 1
    POWER_SetVoltageForFreq(power_GetMainClock());
#else
#if LOWPOWERCLOCKMODE == 4
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);
    Chip_POWER_SetVoltage(POWER_LOW_POWER_MODE, power_GetMainClock());
    CLOCK_SetFLASHAccessCycles(0); /* 1 system clock wait state time for 12MHz */
#else
    LPC_SYSCON->AHBCLKDIV = (LOWPOWERCLOCKMODE - 1);
    Chip_POWER_SetVoltage(POWER_LOW_POWER_MODE, 48000000 / LOWPOWERCLOCKMODE);
    CLOCK_SetFLASHAccessCycles(1); /* 2 systems clock wait state time for up to 30MHz */
#endif
#endif

    SystemCoreClockUpdate();
    SYSCON->RTCOSCCTRL     = 0;
    SYSCON->PDRUNCFGSET[0] = SYSCON_PDRUNCFG_PDEN_TS_MASK | SYSCON_PDRUNCFG_PDEN_BOD_RST_MASK |
                             SYSCON_PDRUNCFG_PDEN_BOD_INTR_MASK | SYSCON_PDRUNCFG_PDEN_ADC0_MASK;

    // Chip_Clock_SetCLKOUTSource(SYSCON_CLKOUTSRC_DISABLED, 1);
    CLOCK_AttachClk(kMAIN_CLK_to_CLKOUT);
    SYSCON->CLKOUTDIV = 0;

#if 0
    /* Unused IRAM, power down with CARE */
    SYSCON->PDRUNCFGSET[0] = (SYSCON_PDRUNCFG_PDEN_SRAM1_MASK | SYSCON_PDRUNCFG_PDEN_SRAM2_MASK);
#endif

    CLOCK_EnableClock(kCLOCK_Iocon);

    pinConfig.pinDirection = kGPIO_DigitalInput;
    for (portIndex = 0; portIndex < 5; ++portIndex)
    {
        for (pinIndex = 0; pinIndex < 5; ++pinIndex)
        {
            /* debug: 0-15/16/17; I2S: 2-18/19/20, 4-1/2/3, 3-11; I2C: 3-23/24; debugconsole: 0-29/30; usb vbus:0-22 */
            if (((portIndex == 0) && (pinIndex == 30)) || ((portIndex == 0) && (pinIndex == 29)) ||
                ((portIndex == 0) && (pinIndex == 22)) || ((portIndex == 2) && (pinIndex == 18)) ||
                ((portIndex == 2) && (pinIndex == 19)) || ((portIndex == 2) && (pinIndex == 20)) ||
                ((portIndex == 3) && (pinIndex == 11)) || ((portIndex == 3) && (pinIndex == 23)) ||
                ((portIndex == 3) && (pinIndex == 24)) || ((portIndex == 4) && (pinIndex == 1)) ||
                ((portIndex == 4) && (pinIndex == 2)) || ((portIndex == 4) && (pinIndex == 3)) ||
                ((portIndex == 0) && (pinIndex == 15)) || ((portIndex == 0) && (pinIndex == 16)) ||
                ((portIndex == 0) && (pinIndex == 17)))
            {
                continue;
            }
            IOCON_PinMuxSet(IOCON, portIndex, pinIndex, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN));
            GPIO_PinInit(GPIO, portIndex, pinIndex, &pinConfig);
        }
    }

    CLOCK_DisableClock(kCLOCK_Iocon);
    CLOCK_DisableClock(kCLOCK_InputMux);
}

void Power_Tasks(void)
{
    clock_attach_id_t clkSource;

    if ((powerFlags & POWERFLAGS_SUSPEND) != 0)
    {
        if ((SYSCON->USB0CLKSTAT & 1) == 0)
        {
            powerFlags &= ~POWERFLAGS_SUSPEND;
            __disable_irq();

            Codec_Suspend();

            NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
            NVIC_EnableIRQ(USB0_NEEDCLK_IRQn);
#if defined(USEINRAMDEEPSLEEP)
            /* UNKNOWN!!!!!!!!
             * Will not wakeup reliably with USBNEEDCLK alone in POWERDOWN.
             */
            NVIC_DisableIRQ(USB0_IRQn);
#endif

            /* Setup wakeup states, disable others */
            SYSCON->STARTERCLR[0] = SYSCON->STARTER[0];
            SYSCON->STARTERSET[0] = SYSCON_STARTER_USB0_NEEDCLK_MASK;

#if defined(USEINRAMDEEPSLEEP)
            /* UNKNOWN!!!!!!!!
             * Will not wakeup reliably with USBNEEDCLK alone in POWERDOWN.
             */
            SYSCON->USB0CLKCTRL = (SYSCON_USB0CLKCTRL_POL_FS_DEV_CLK_MASK);
#endif

            /* These clocks can be safely shut down temporarily
             * during the low power state. */
            SYSCON->PDRUNCFGSET[0] = SYSCON_PDRUNCFG_PDEN_USB0_PHY_MASK;
            CLOCK_DisableClock(kCLOCK_FlexComm6);
            CLOCK_DisableClock(kCLOCK_FlexComm7);

            /* Switching to a lower main clock can save some power */
            clkSource = power_GetMainClockSource();
            if (clkSource != kFRO12M_to_MAIN_CLK)
            {
                CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);
            }

#if defined(USEINRAMDEEPSLEEP)
            /* ALWAYS Deep sleep 1 */
            POWER_EnterDeepSleep(SYSCON_PDRUNCFG_PDEN_SRAM0_MASK | SYSCON_PDRUNCFG_PDEN_SRAM1_2_3_MASK |
                                 SYSCON_PDRUNCFG_PDEN_SRAMX_MASK);

            /* Requires
             * Chip_POWER_EnterPowerModeRel(POWER_DEEP_SLEEP, 0x4);
             * if main clock is operating at 48MHz.
             */

#else
            /* Power down mode, may not work if called when main clock is at 48MHz */
            Chip_POWER_EnterPowerMode(POWER_DEEP_SLEEP2, 0x0);

/* Requires
 * Chip_POWER_EnterPowerMode(POWER_POWER_DOWN, 0x4);
 * if main clock is operating at 48MHz.
 */
#endif

            if (clkSource != kFRO12M_to_MAIN_CLK)
            {
                // CLOCK_AttachClk(clkSource);
                BOARD_BootClockFROHF48M();
            }

            CLOCK_EnableClock(kCLOCK_FlexComm7);
            CLOCK_EnableClock(kCLOCK_FlexComm6);
            SYSCON->PDRUNCFGCLR[0] = SYSCON_PDRUNCFG_PDEN_USB0_PHY_MASK;

            Codec_Resume();

            SYSCON->STARTERCLR[0] = SYSCON_STARTER_USB0_NEEDCLK_MASK;
            NVIC_DisableIRQ(USB0_NEEDCLK_IRQn);
            NVIC_EnableIRQ(USB0_IRQn);

            __enable_irq(); /* Don't trust power API calls to not re-enable IRQs */
        }
    }
    else if ((powerFlags & POWERFLAGS_RESUME) != 0)
    {
        /* Resume */
        powerFlags &= ~POWERFLAGS_RESUME;
        /* Nothing to really do here, as wakeup from suspend does everything */
    }
    else
    {
        /* Sleep until next IRQ happens */
        __WFI();
    }
}
