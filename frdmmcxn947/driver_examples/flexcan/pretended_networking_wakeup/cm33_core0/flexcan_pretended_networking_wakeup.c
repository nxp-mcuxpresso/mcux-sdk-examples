/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_flexcan.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_cmc.h"
#include "fsl_spc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_CAN                CAN0
#define TX_MESSAGE_BUFFER_NUM      (9U)
#define EXAMPLE_CAN_CLK_FREQ       CLOCK_GetFlexcanClkFreq(0U)
#define USE_IMPROVED_TIMING_CONFIG (1U)
#ifndef DEMO_WAKEUP_FRAME_NUM
#define DEMO_WAKEUP_FRAME_NUM (4U)
#endif
#ifndef DEMO_WAKEUP_FRAME_DLC
#define DEMO_WAKEUP_FRAME_DLC (8U)
#endif
#ifndef DEMO_WAKEUP_FRAME_ID
#define DEMO_WAKEUP_FRAME_ID (0x123U)
#endif
/* Fix MISRA_C-2012 Rule 17.7. */
#define LOG_INFO (void)PRINTF
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_SetLowerPowerConfig(void);
void APP_EnterLowerPowerMode(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
flexcan_handle_t flexcanHandle;
volatile bool txComplete = false;
volatile bool wakenUp    = false;
flexcan_mb_transfer_t txXfer;
flexcan_frame_t frame;

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_SetLowerPowerConfig(void)
{
    CMC_EnableDebugOperation(CMC0, true);
    CMC_SetPowerModeProtection(CMC0, kCMC_AllowAllLowPowerModes);
    CMC_LockPowerModeProtectionSetting(CMC0);
}
void APP_EnterLowerPowerMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepSleepMode;
    config.wake_domain = kCMC_ActiveOrSleepMode;

    SCG0->FIRCCSR |= SCG_FIRCCSR_FIRCSTEN_MASK;

    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(SPC0, true);

    CMC_EnterLowPowerMode(CMC0, &config);
}
/*!
 * @brief FlexCAN Call Back function
 */
static FLEXCAN_CALLBACK(flexcan_callback)
{
    switch (status)
    {
        case kStatus_FLEXCAN_TxIdle:
            if (TX_MESSAGE_BUFFER_NUM == result)
            {
                txComplete = true;
            }
            break;

        case kStatus_FLEXCAN_WakeUp:
            wakenUp = true;
            break;

        default:
            break;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    flexcan_config_t flexcanConfig;
    flexcan_pn_config_t pnConfig;
    uint8_t node_type;
    uint8_t i;

    /* Initialize board hardware. */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO HF to FLEXCAN0 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcan0Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_FLEXCAN0);

    BOARD_InitPins();
    BOARD_BootClockFROHF48M();
    BOARD_InitDebugConsole();

    LOG_INFO("FlexCAN pretended networking wake up example.\r\n");

    do
    {
        LOG_INFO("Please select local node as A or B:\r\n");
        LOG_INFO("Note: Node B should start first.\r\n");
        LOG_INFO("Node:");
        node_type = GETCHAR();
        LOG_INFO("%c", node_type);
        LOG_INFO("\r\n");
    } while ((node_type != 'A') && (node_type != 'B') && (node_type != 'a') && (node_type != 'b'));

    /* Get FlexCAN module default Configuration. */
    /*
     * flexcanConfig.clkSrc                       = kFLEXCAN_ClkSrc0;
     * flexcanConfig.bitRate                     = 1000000U;
     * flexcanConfig.bitRateFD                   = 2000000U;
     * flexcanConfig.maxMbNum                     = 16;
     * flexcanConfig.enableLoopBack               = false;
     * flexcanConfig.enableSelfWakeup             = false;
     * flexcanConfig.enableIndividMask            = false;
     * flexcanConfig.disableSelfReception         = false;
     * flexcanConfig.enableListenOnlyMode         = false;
     * flexcanConfig.enableDoze                   = false;
     */
    FLEXCAN_GetDefaultConfig(&flexcanConfig);
    flexcanConfig.enablePretendedeNetworking = true;
#if defined(EXAMPLE_ENABLE_FLEXCAN_DOZE_MODE)
    flexcanConfig.enableDoze = true;
#endif

#if defined(EXAMPLE_CAN_CLK_SOURCE)
    flexcanConfig.clkSrc = EXAMPLE_CAN_CLK_SOURCE;
#endif

/* If special quantum setting is needed, set the timing parameters. */
#if (defined(SET_CAN_QUANTUM) && SET_CAN_QUANTUM)
    flexcanConfig.timingConfig.phaseSeg1 = PSEG1;
    flexcanConfig.timingConfig.phaseSeg2 = PSEG2;
    flexcanConfig.timingConfig.propSeg   = PROPSEG;
#endif

/* Calculate bit timing automatically if needed. */
#if (defined(USE_IMPROVED_TIMING_CONFIG) && USE_IMPROVED_TIMING_CONFIG)
    flexcan_timing_config_t timing_config;
    (void)memset(&timing_config, 0, sizeof(flexcan_timing_config_t));

    if (FLEXCAN_CalculateImprovedTimingValues(EXAMPLE_CAN, flexcanConfig.bitRate, EXAMPLE_CAN_CLK_FREQ, &timing_config))
    {
        /* Update the improved timing configuration*/
        memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
    }
    else
    {
        LOG_INFO("No found Improved Timing Configuration. Just used default configuration\r\n\r\n");
    }
#endif

    FLEXCAN_Init(EXAMPLE_CAN, &flexcanConfig, EXAMPLE_CAN_CLK_FREQ);

    /* Create FlexCAN handle structure and set call back function. */
    FLEXCAN_TransferCreateHandle(EXAMPLE_CAN, &flexcanHandle, flexcan_callback, NULL);

    if ((node_type == 'A') || (node_type == 'a'))
    {
        /* Setup Tx Message Buffer. */
        FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
        frame.dataByte1 = 0x55;
        frame.dataByte0 = 0x0;
        frame.id        = FLEXCAN_ID_STD(DEMO_WAKEUP_FRAME_ID);
        frame.format    = (uint8_t)kFLEXCAN_FrameFormatStandard;
        frame.type      = (uint8_t)kFLEXCAN_FrameTypeData;
        frame.length    = (uint8_t)DEMO_WAKEUP_FRAME_DLC;
        txXfer.mbIdx    = (uint8_t)TX_MESSAGE_BUFFER_NUM;
        txXfer.frame    = &frame;
        LOG_INFO("Press any key to trigger one-shot transmission\r\n");
    }
    else
    {
        APP_SetLowerPowerConfig();
        /* Setup Pretended Networking mode to make FlexCAN detect specific wakeup frames under lower power mode. */
        (void)memset(&pnConfig, 0, sizeof(pnConfig));
        pnConfig.enableMatch   = true;
        pnConfig.matchSrc      = kFLEXCAN_PNMatSrcIDAndData;
        pnConfig.matchNum      = DEMO_WAKEUP_FRAME_NUM;
        pnConfig.idMatchMode   = kFLEXCAN_PNMatModeRange;
        pnConfig.dataMatchMode = kFLEXCAN_PNMatModeRange;
        pnConfig.idLower       = FLEXCAN_PN_STD_MASK(DEMO_WAKEUP_FRAME_ID, 0);
        pnConfig.idUpper       = FLEXCAN_PN_STD_MASK(DEMO_WAKEUP_FRAME_ID + 1U, 0);
        pnConfig.lengthLower   = (uint8_t)DEMO_WAKEUP_FRAME_DLC;
        pnConfig.lengthUpper   = (uint8_t)DEMO_WAKEUP_FRAME_DLC;
        pnConfig.lowerByte0    = 0x0;
        pnConfig.lowerByte1    = 0x55;
        pnConfig.upperByte0    = 0x0;
        pnConfig.upperByte1    = 0x56;
        FLEXCAN_SetPNConfig(EXAMPLE_CAN, &pnConfig);
        LOG_INFO("Note B will enter lower power mode and wake up until received %d specific messages.\r\n",
                 DEMO_WAKEUP_FRAME_NUM);
        LOG_INFO("Wake up message format: Standard (11 bit id)\r\n");
        LOG_INFO("Wake up message ID range: 0x%x to 0x%x\r\n", DEMO_WAKEUP_FRAME_ID, DEMO_WAKEUP_FRAME_ID + 1U);
        LOG_INFO("Wake up payload range : 0x%08x%08x to 0x%08x%08x\r\n", pnConfig.lowerWord0, pnConfig.lowerWord1,
                 pnConfig.upperWord0, pnConfig.upperWord1);
    }

    while (true)
    {
        if ((node_type == (uint8_t)'A') || (node_type == (uint8_t)'a'))
        {
            GETCHAR();

            (void)FLEXCAN_TransferSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);

            while (!txComplete)
            {
            };
            txComplete = false;
            LOG_INFO("Send message ID: 0x%3x, payload: 0x%08x%08x\r\n\r\n", frame.id >> CAN_ID_STD_SHIFT,
                     frame.dataWord0, frame.dataWord1);
            LOG_INFO("Press any key to trigger the next transmission!\r\n");

            /* Polling to send normal frames and wake up frames. */
            frame.dataByte0++;
            if ((frame.dataByte0 % 2U) == 0U)
            {
                frame.id = (frame.id == FLEXCAN_ID_STD(DEMO_WAKEUP_FRAME_ID)) ?
                               FLEXCAN_ID_STD(DEMO_WAKEUP_FRAME_ID + 2U) :
                               FLEXCAN_ID_STD(DEMO_WAKEUP_FRAME_ID);
            }
            if ((frame.dataByte0 % 5U) == 0U)
            {
                frame.dataByte1 = (frame.dataByte1 == 0x55U) ? 0x56U : 0x55U;
                frame.dataByte0 = 0x0;
            }
        }
        else
        {
            APP_EnterLowerPowerMode();
            /* Wait wakes up. */
            if (wakenUp)
            {
                wakenUp = false;

                LOG_INFO("Waken up!\r\n");
                /* Print the last received wake up messages, max to 4. */
                for (i = 0;
                     i < ((FLEXCAN_GetPNMatchCount(EXAMPLE_CAN) >= 4U) ? 4U : FLEXCAN_GetPNMatchCount(EXAMPLE_CAN));
                     i++)
                {
                    (void)FLEXCAN_ReadPNWakeUpMB(EXAMPLE_CAN, i, &frame);
                    LOG_INFO("Match message %d ID: 0x%3x, payload: 0x%08x%08x\r\n", i, frame.id >> CAN_ID_STD_SHIFT,
                             frame.dataWord0, frame.dataWord1);
                }
                LOG_INFO("Enter lower power mode again!\r\n\r\n");
            };
        }
    }
}
