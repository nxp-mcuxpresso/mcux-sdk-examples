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

const iocon_group_t unusedPins[] = {
    {
        // P0_2-GPIO_SPI_CS
        0,
        2,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P0_3-GPIO_SPI_CS
        0,
        3,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // BRIDGE_T_INTR-ISP1
        0,
        4,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },

    // P0_5-FC6_RXD_SDA_MOSI_DATA
    // P0_6-FC6_TXD_SCL_MISO_FRAME - not altered
    // P0_7-FC6_SCK - not altered
    //    {
    //        0, 5, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    //    },
    //    {
    //        0, 6, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    //    },
    //    {
    //        0, 7, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    //    },

    {
        // P0_8-FC2_RXD_SDA_MOSI
        0,
        8,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P0_9-FC2_TXD_SCL_MISO
        0,
        9,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P0_10-FC2_SCK-CT32B3_MAT0
        0,
        10,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // BRIDGE_T_SCK
        0,
        11,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // BRIDGE_T_MOSI
        0,
        12,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // BRIDGE_T_MISO
        0,
        13,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // BRIDGE_T_SSEL-SPIFI_IO3
        0,
        14,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
#if !defined(ENABLEJTAG)
    {
        // TDO-SWO_TRGT-SPIFI_IO2
        0,
        15,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // TCK-SWDCLK_TRGT-SPIFI_IO1
        0,
        16,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // IF_TMS_SWDIO-SPIFI_IO0
        0,
        17,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
#endif
    {
        // P0_18-FC5_TXD_SCL_MISO
        0,
        18,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P0_19-FC5_SCK-SPIFI_CSn
        0,
        19,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P0_20-FC5_RXD_SDA_MOSI
        0,
        20,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P0_21-CLKOUT-SPIFI_CLK
        0,
        21,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P0_22-BRIDGE_GPIO
        0,
        22,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // BRIDGE_SCL
        0,
        23,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // BRIDGE_SDA-WAKEUP
        0,
        24,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    //    {
    //        // P0_25-FC4_SCLX
    //        0, 25, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    //    },
    //    {
    //        // P0_26-FC4_SDAX
    //        0, 26, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    //    },

    // USB_DP_RES - not altered
    // USB_DM_RES - not altered

    {
        // P0_29-CT32B0_MAT3-RED
        0,
        29,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P0_30-ADC1
        0,
        30,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P0_31-PDM0_CLK-ISP0_EN
        0,
        31,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P1_0/PDM0_DATA/FC2_RTS_SCL_SSELN1/CT3MAT1//CT0CAP0
        1,
        0,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P1_1//SWO/SCT0_OUT4/FC5_SSELN2/FC4_TXD_SCL_MISO
        1,
        1,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P1_2/MCLK/FC7_SSELN3/SCT0_OUT5/FC5_SSELN3/FC4_RXD_SDA_MOSI
        1,
        2,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P1_3//FC7_SSELN2/SCT0_OUT6//FC3_SCK/CT0CAP1/USB0_LEDN
        1,
        3,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P1_4/PDM1_CLK/FC7_RTS_SCL_SSELN1/SCT0_OUT7//FC3_TXD_SCL_MISO
        1,
        4,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P1_5/PDM1_DATA/FC7_CTS_SDA_SSELN0/CT1CAP0//CT1MAT3/PVT_AM0_ALRT
        1,
        5,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    //    {
    //        // P1_7//FC7_RXD_SDA_MOSI/CT1MAT2//CT1CAP2/PVT_AMBER1_ALERT
    //        1, 7, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    //    },
    //    {
    //        // P1_8//FC7_TXD_SCL_MISO/CT1MAT3//CT1CAP3/PVT_RED1_ALERT
    //        1, 8, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    //    },
    {
        // P1_9//FC3_RXD_SDA_MOSI/CT0CAP2///USB0_LEDN/USB0_CONNECTN
        1,
        9,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P1_10//FC6_TXD_SCL_MISO/SCT0_OUT4/FC1_SCK///USB0_FRAME
        1,
        10,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },

    // P1_11//FC6_RTS_SCL_SSELN1/CT1CAP0/FC4_SCK///USB0_VBUS
    {
        1,
        11,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },

    //    {
    //        // P1_12//FC5_RXD_SDA_MOSI/CT1MAT0/FC7_SCK/UTICK_CAP2
    //        1, 12, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    //    },
    {
        // P1_13//FC5_TXD_SCL_MISO/CT1MAT1/FC7_RXD_SDA_MOSI
        1,
        13,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P1_14//FC2_RXD_SDA_MOSI/SCT0_OUT7/FC7_TXD_SCL_MISO
        1,
        14,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P1_15/PDM0_CLK/SCT0_OUT5/CT1CAP3/FC7_CTS_SDAX_SSELN0
        1,
        15,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    {
        // P1_16/PDM0_DATA/CT0MAT0/CT0CAP0/FC7_RTS_SCLX_SSELN1
        1,
        16,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
    // P1_17////MCLK/UTICK_CAP3 - not altered
    {
        1,
        17,
        (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN),
    },
};

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
    uint32_t index;
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
    CLOCK_AttachClk(kNONE_to_CLKOUT);
    SYSCON->CLKOUTDIV = 0;

    CLOCK_EnableClock(kCLOCK_Iocon);

    pinConfig.pinDirection = kGPIO_DigitalInput;
    for (index = 0; index < (sizeof(unusedPins) / sizeof(unusedPins[0])); index++)
    {
        IOCON_PinMuxSet(IOCON, unusedPins[index].port, unusedPins[index].pin, unusedPins[index].modefunc);
        GPIO_PinInit(GPIO, unusedPins[index].port, unusedPins[index].pin, &pinConfig);
    }

    CLOCK_DisableClock(kCLOCK_Iocon);
    CLOCK_DisableClock(kCLOCK_InputMux);
}

void Power_Tasks(void)
{
    clock_attach_id_t clkSource;

    if ((powerFlags & POWERFLAGS_SUSPEND) != 0)
    {
        if ((SYSCON->USBCLKSTAT & 1) == 0)
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
            SYSCON->USBCLKCTRL = (1 << 1);
#endif

            /* These clocks can be safely shut down temporarily
             * during the low power state. */
            SYSCON->PDRUNCFGSET[0] = SYSCON_PDRUNCFG_PDEN_USB_PHY_MASK;
            CLOCK_DisableClock(kCLOCK_FlexComm6);
            CLOCK_DisableClock(kCLOCK_FlexComm7);
            CLOCK_DisableClock(kCLOCK_Gpio0);
            CLOCK_DisableClock(kCLOCK_Gpio1);

            /* Switching to a lower main clock can save some power */
            clkSource = power_GetMainClockSource();
            if (clkSource != kFRO12M_to_MAIN_CLK)
            {
                CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);
            }

#if defined(USEINRAMDEEPSLEEP)
            /* ALWAYS Deep sleep 1 */
            POWER_EnterDeepSleep(SYSCON_PDRUNCFG_PDEN_SRAM0_MASK | SYSCON_PDRUNCFG_PDEN_SRAMX_MASK);

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

            CLOCK_EnableClock(kCLOCK_Gpio1);
            CLOCK_EnableClock(kCLOCK_Gpio0);
            CLOCK_EnableClock(kCLOCK_FlexComm7);
            CLOCK_EnableClock(kCLOCK_FlexComm6);
            SYSCON->PDRUNCFGCLR[0] = SYSCON_PDRUNCFG_PDEN_USB_PHY_MASK;

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
