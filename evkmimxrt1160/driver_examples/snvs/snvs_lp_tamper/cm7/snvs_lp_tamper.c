/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_snvs_hp.h"
#include "fsl_snvs_lp.h"
#include "fsl_iomuxc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define kCLOCK_SnvsHp0          kCLOCK_SnvsHp
#define EXAMPLE_SNVS_IRQn       SNVS_HP_NON_TZ_IRQn
#define EXAMPLE_SNVS_IRQHandler SNVS_HP_NON_TZ_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile bool busyWait;

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_SNVS_TamperFuse()
{
    /* check tamper function setting */
    uint32_t fuse006;
    fuse006 = OCOTP->FUSEN[6].FUSE;
    if ((fuse006 & 0xc000) != 0)
    {
        PRINTF("Error: SNVS Tamper FUSE not enabled! /r/n");
    }
}

/*!
 * @brief Main function
 */

void EXAMPLE_SNVS_IRQHandler(void)
{
    if (SNVS_HP_RTC_GetStatusFlags(SNVS) & kSNVS_RTC_AlarmInterruptFlag)
    {
        busyWait = false;

        /* Clear alarm flag */
        SNVS_HP_RTC_ClearStatusFlags(SNVS, kSNVS_RTC_AlarmInterruptFlag);
    }
    SDK_ISR_EXIT_BARRIER;
}

void EXAMPLE_SNVS_Tamper_PinMuxSetting()
{
    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_00_DIG_SNVS_TAMPER0, /* tamper0 pinmux */
                     0U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_01_DIG_SNVS_TAMPER1, /* tamper0 pinmux */
                     0U);

    IOMUXC_SetPinMux(

        IOMUXC_GPIO_SNVS_02_DIG_SNVS_TAMPER2, /* tamper0 pinmux */
        1U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_03_DIG_SNVS_TAMPER3, /* tamper0 pinmux */
                     0U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_04_DIG_SNVS_TAMPER4, /* tamper0 pinmux */
                     0U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_05_DIG_SNVS_TAMPER5, /* tamper0 pinmux */
                     0U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_06_DIG_SNVS_TAMPER6, /* tamper0 pinmux */
                     0U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_07_DIG_SNVS_TAMPER7, /* tamper0 pinmux */
                     0U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_08_DIG_SNVS_TAMPER8, /* tamper0 pinmux */
                     0U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_09_DIG_SNVS_TAMPER9, /* tamper0 pinmux */
                     0U);
}

void EXAMPLE_SNVS_Tamper_PullUp()
{
    /* Select and enable Pull Up resistors for all tamper pins */
    IOMUXC_SNVS_GPR->GPR37 |=
        (IOMUXC_SNVS_GPR_GPR37_SNVS_TAMPER_PUE(0x3FF) | IOMUXC_SNVS_GPR_GPR37_SNVS_TAMPER_PUS(0x3FF));
}

void print_help()
{
    PRINTF("\r\nSNVS tamper demo \r\n");
    PRINTF("1 - passive tamper pin  \r\n");
    PRINTF("2 - active tamper pin  \r\n");
    PRINTF("3 - voltage tamper enable  \r\n");
    PRINTF("4 - voltage tamper test  \r\n");
    PRINTF("5 - temperature tamper enable \r\n");
    PRINTF("6 - temperature tamper test \r\n");
    PRINTF("7 - clock tamper enable  \r\n");
    PRINTF("8 - clock tamper test  \r\n");
    PRINTF("0 - exit  \r\n");
    PRINTF("\r\n\r\nSelect test and confirm by Enter...\r\n");
}

uint32_t GetInputNumber()
{
    char ch;
    char c;

    ch = GETCHAR();
    if (ch == '\n')
        ch = GETCHAR();

    PUTCHAR(ch);
    if (ch == '1')
    {
        c = GETCHAR();
        PUTCHAR(c);
        if (c == '0')
        {
            GETCHAR();
            return 10;
        }
        else
        {
            GETCHAR();
            return 1;
        }
    }

    GETCHAR();
    return ch - '0';
}

void active_tamper_setting(uint32_t tx, uint32_t rx)
{
    tamper_active_tx_config_t tx_config;
    tamper_active_rx_config_t rx_config;

    /* Get default structure values for TX */
    SNVS_LP_TamperPinTx_GetDefaultConfig(&tx_config);

    /* Get default structure values for RX */
    SNVS_LP_TamperPinRx_GetDefaultConfig(&rx_config);

    /* Enable active tamper TX */
    SNVS_LP_EnableTxActiveTamper(SNVS, (snvs_lp_active_tx_tamper_t)tx, tx_config);

    /* Set coresponding TX pin in RX config structure */
    rx_config.activeTamper = (snvs_lp_active_tx_tamper_t)tx;

    /* Enable RX pin and route active tamper TX to it */
    SNVS_LP_EnableRxActiveTamper(SNVS, (snvs_lp_external_tamper_t)rx, rx_config);
}

int main(void)
{
    snvs_hp_rtc_config_t snvsRtcConfig;
    snvs_lp_srtc_config_t snvsSrtcConfig;
    uint8_t tamper_type;

    static uint32_t ZMKey[] = {
        0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    };

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_SNVS_TamperFuse();

    /* Init SNVS_HP */
    /*
     * config->rtcCalEnable = false;
     * config->rtcCalValue = 0U;
     * config->periodicInterruptFreq = 0U;
     */
    SNVS_HP_RTC_GetDefaultConfig(&snvsRtcConfig);
    SNVS_HP_RTC_Init(SNVS, &snvsRtcConfig);

    /* Init SNVS_LP */
    /*
     * config->srtcCalEnable = false;
     * config->srtcCalValue = 0U;
     */
    SNVS_LP_SRTC_GetDefaultConfig(&snvsSrtcConfig);
    SNVS_LP_SRTC_Init(SNVS, &snvsSrtcConfig);

    SNVS_LP_Init(SNVS);

    /* Set tamper 0-9 in IOMUXC setting */
    EXAMPLE_SNVS_Tamper_PinMuxSetting();

    /* LP security violation is a non-fatal violation */
    SNVS->HPSVCR |= SNVS_HPSVCR_LPSV_CFG(1);

    print_help();

    while ((tamper_type = GetInputNumber()))
    {
        /* Check user response */
        if ((tamper_type < 1) || (tamper_type > 8))
        {
            print_help();
            continue;
        }

        /* Clear all external tampers */
        SNVS_LP_ClearAllExternalTamperStatus(SNVS);

        /* Write ZMK to non-zero */
        SNVS_LP_WriteZeroizableMasterKey(SNVS, ZMKey);

        /* Select zeroizable master key when MKS_EN bit is set */
        SNVS_LP_SetMasterKeyMode(SNVS, kSNVS_ZMK);

        /* ZMK is in the software programming mode */
        SNVS_LP_SetZeroizableMasterKeyProgramMode(SNVS, kSNVS_ZMKSoftwareProgram);

        /* ZMK is valid */
        SNVS_LP_SetZeroizableMasterKeyValid(SNVS, true);

        /* Trigger to program Zeroizable Master Key */
        SNVS_HP_ProgramZeroizableMasterKey(SNVS);

        /* ZMK ECC check is enabled */
        SNVS_LP_EnableZeroizableMasterKeyECC(SNVS, true);

        switch (tamper_type)
        {
            /* passive tamper */
            case 1:
            {
                /* Get number of pin */
                PRINTF("Select passive tamper pin to be used (1~10) and confirm by Enter...");
                snvs_lp_external_tamper_t tamper_pin_index = (snvs_lp_external_tamper_t)GetInputNumber();

                if (tamper_pin_index < 1 || tamper_pin_index > 10)
                {
                    PRINTF("r/n/Wrong input!/r/n");
                    break;
                }

                snvs_lp_passive_tamper_t taper_cfg;

                /* Fill default values into passive tamper pin config structure */
                SNVS_LP_PassiveTamperPin_GetDefaultConfig(&taper_cfg);

                taper_cfg.polarity = 1U, taper_cfg.filterenable = 0U, taper_cfg.filter = 3U;

                /* Select and enable Pull Up resistors for all tamper pins */
                EXAMPLE_SNVS_Tamper_PullUp();

                /* Enable coresponding tamper pin in SNVS */
                SNVS_LP_EnablePassiveTamper(SNVS, tamper_pin_index, taper_cfg);
                PRINTF("\r\n If tamper pin %d is not low level, will trigger tamper violation\r\n", tamper_pin_index);

                /* if filter enable,wait some time  */
                for (uint32_t i = 0; i < 0xfffffff; i++)
                    ;

                /* Check for tamper detection */
                if (SNVS_LP_GetExternalTamperStatus(SNVS, tamper_pin_index) == kSNVS_TamperDetected)
                {
                    PRINTF("External Tampering %d Detected\r\n", tamper_pin_index);
                }
                else
                {
                    PRINTF("No External Tampering Detected\r\n");
                }

                /* Check if ZMK is cleared */
                if (SNVS_HP_GetStatusFlags(SNVS) & (uint32_t)kSNVS_ZMK_ZeroFlag)
                {
                    PRINTF("ZMK is cleared\r\n");
                }
                else
                {
                    PRINTF("ZMK is not zero!\r\n");
                }

                PRINTF("\r\npress any key to disable all pins and return to test menu ...\r\n");
                GETCHAR();

                /* Disable all external pins */
                SNVS_LP_DisableAllExternalTamper(SNVS);
                break;

            } /* End of passive tamper case */
            /* active tamper */
            case 2:
            {
                /* Get number of pins */
                PRINTF("\r\nSelect tamper active tx pin (1~5) and confirm by Enter...\r\n");
                snvs_lp_external_tamper_t tamper_pin_tx = (snvs_lp_external_tamper_t)GetInputNumber();
                PRINTF("\r\nSelect tamper ecternal rx pin input(6-10) and confirm by Enter...\r\n");
                snvs_lp_external_tamper_t tamper_pin_rx = (snvs_lp_external_tamper_t)GetInputNumber();

                active_tamper_setting(tamper_pin_tx, tamper_pin_rx);
                PRINTF(
                    "\r\nif tamper pin tx %d and rx pin %d don't connect together, will trigger tamper violation\r\n",
                    tamper_pin_tx, tamper_pin_rx);

                /* if filter enable,wait some time  */
                for (uint32_t i = 0; i < 0xfffffff; i++)
                    ;
                /* Check for tamper detection */

                /* Check if violation detected */
                bool detected = false;
                for (int pin = (int32_t)kSNVS_ExternalTamper1; pin <= (int32_t)SNVS_LP_MAX_TAMPER; pin++)
                {
                    if (SNVS_LP_GetExternalTamperStatus(SNVS, (snvs_lp_external_tamper_t)pin) == kSNVS_TamperDetected)
                    {
                        detected = true;
                    }
                }

                if (detected != true)
                {
                    PRINTF("External Tampering not detected.\r\n");
                }
                else
                {
                    PRINTF("External Tampering Detected!\r\n");
                }

                PRINTF("\r\npress any key to continue ...\r\n");
                GETCHAR();
                /* Disable all external pins */
                SNVS_LP_DisableAllExternalTamper(SNVS);

                /* Erase routing and disable active pins */
                SNVS->LPATRC1R = 0;
                SNVS->LPATRC2R = 0;
                SNVS->LPATCTLR = 0;
                break;
            }
            /* enable voltage tamper */
            case 3:
            {
                SNVS_LP_SetVoltageTamper(SNVS, true);
                PRINTF("\r\nVoltage tamper enabled!\r\n");
                PRINTF("\r\npress any key to continue ...\r\n");
                GETCHAR();
                break;
            }
            /* test voltage tamper */
            case 4:
            {
                if (SNVS_LP_CheckVoltageTamper(SNVS) == kSNVS_TamperDetected)
                {
                    PRINTF("\r\nVoltage tamper detected!\r\n");
                }
                else
                {
                    PRINTF("\r\nNo voltage tamper detected!\r\n");
                }
                PRINTF("\r\npress any key to continue ...\r\n");
                GETCHAR();
                break;
            }
            /* enable temperature tamper */
            case 5:
            {
                SNVS_LP_SetTemperatureTamper(SNVS, true);
                PRINTF("\r\nTemperature tamper enabled!\r\n");
                PRINTF("\r\npress any key to continue ...\r\n");
                GETCHAR();
                break;
            }
            /* test temperature tamper */
            case 6:
            {
                if (SNVS_LP_CheckTemperatureTamper(SNVS) == kSNVS_TamperDetected)
                {
                    PRINTF("\r\nTemperature tamper detected!\r\n");
                }
                else
                {
                    PRINTF("\r\nNo Temperature tamper detected!\r\n");
                }
                PRINTF("\r\npress any key to continue ...\r\n");
                GETCHAR();
                break;
            }
            /* enable clock tamper */
            case 7:
            {
                SNVS_LP_SetClockTamper(SNVS, true);
                PRINTF("\r\nClock tamper enabled!\r\n");
                PRINTF("\r\npress any key to continue ...\r\n");
                GETCHAR();
                break;
            }
            /* test clock tamper */
            case 8:
            {
                if (SNVS_LP_CheckClockTamper(SNVS) == kSNVS_TamperDetected)
                {
                    PRINTF("\r\nClock tamper detected!\r\n");
                }
                else
                {
                    PRINTF("\r\nNo Clock tamper detected!\r\n");
                }
                PRINTF("\r\npress any key to continue ...\r\n");
                GETCHAR();
                break;
            }
            default:
            {
                print_help();
                break;
            }
        } /* End of switch */
    }

    PRINTF("\r\nSNVS Tamper demo exit \r\n");

    while (1)
    {
    }
}
