/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_sinc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SINC             SINC2
#define DEMO_SINC_IRQn        SINC2_CH3_IRQn
#define DEMO_SINC_IRQ_HANDLER SINC2_CH3_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void DEMO_SINC_IRQ_HANDLER(void)
{
    uint32_t fifoData;
    if ((SINC_GetInterruptStatus(DEMO_SINC) & kSINC_CH3ConvCompleteIntStatus) != 0UL)
    {
        while (!SINC_CheckChannelResultDataReady(DEMO_SINC, kSINC_Channel3))
            ;
        SINC_LatchChannelDebugProceduce(DEMO_SINC, kSINC_Channel3);
        while (!SINC_CheckChannelDebugDataValid(DEMO_SINC, kSINC_Channel3))
            ;
        fifoData = SINC_ReadChannelResultData(DEMO_SINC, kSINC_Channel3);
        PRINTF("\r\nAdc Result: %d\r\n", fifoData);
        SINC_ClearInterruptStatus(DEMO_SINC, kSINC_CH3ConvCompleteIntStatus);
    }
}

int main(void)
{
    sinc_config_t sincConfig;
    sinc_channel_config_t sincChannel3Config;
    sinc_channel_input_option_t sincChannel3InputOption;
    sinc_channel_conv_option_t sincChannel3ConvOption;
    sinc_channel_protection_option_t sincChannel3ProtectionOption;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    clock_root_config_t rootCfg = {0};
    rootCfg.mux                 = kCLOCK_BUS_WAKEUP_ClockRoot_MuxSysPll2Out;
    rootCfg.div                 = 4;
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Wakeup, &rootCfg);
    PRINTF("\r\nSINC ADC Example.\r\n");

    sincChannel3InputOption.inputBitFormat = kSINC_InputBit_FormatExternalBitstream;
    sincChannel3InputOption.inputBitDelay  = kSINC_InputBit_DelayDisabled;
    sincChannel3InputOption.inputBitSource = kSINC_InputBit_SourceExternalBitstream;
    sincChannel3InputOption.inputClkEdge   = kSINC_InputClk_EdgePositive;
    sincChannel3InputOption.inputClkSource = kSINC_InputClk_SourceExternalModulatorClk;

    sincChannel3ConvOption.convMode              = kSINC_ConvMode_Single;
    sincChannel3ConvOption.convTriggerSource     = kSINC_ConvTrig_SoftPosEdge;
    sincChannel3ConvOption.enableChPrimaryFilter = true;
    sincChannel3ConvOption.pfBiasSign            = kSINC_PF_BiasPositive;
    sincChannel3ConvOption.pfHpfAlphaCoeff       = kSINC_PF_HPFAlphaCoeff0;
    sincChannel3ConvOption.pfOrder               = kSINC_PF_ThirdOrder;
    sincChannel3ConvOption.pfShiftDirection      = kSINC_PF_ShiftRight;
    sincChannel3ConvOption.u16pfOverSampleRatio  = 127U; // The OSR for equation is 128.
    sincChannel3ConvOption.u32pfBiasValue        = 0U;
    sincChannel3ConvOption.u8pfShiftBitsNum      = 0U;

    sincChannel3ProtectionOption.bEnableCadBreakSignal = false;
    sincChannel3ProtectionOption.bEnableLmtBreakSignal = false;
    sincChannel3ProtectionOption.bEnableScdBreakSignal = false;
    sincChannel3ProtectionOption.cadLimitThreshold     = kSINC_Cad_Disabled;
    sincChannel3ProtectionOption.limitDetectorMode     = kSINC_Lmt_Disabled;
    sincChannel3ProtectionOption.scdOperateMode        = kSINC_Scd_OperateDisabled;
    sincChannel3ProtectionOption.scdOption             = kSINC_Scd_DetectRepeating0And1;
    sincChannel3ProtectionOption.u32HighLimitThreshold = 0XFFFFFFUL;
    sincChannel3ProtectionOption.u32LowLimitThreshold  = 0x0UL;
    sincChannel3ProtectionOption.u8ScdLimitThreshold   = 2U;
    sincChannel3ProtectionOption.zcdOperateMode        = kSINC_ZCD_Disabled;

    sincChannel3Config.bEnableChannel     = true;
    sincChannel3Config.bEnableFifo        = false;
    sincChannel3Config.bEnablePrimaryDma  = false;
    sincChannel3Config.chConvOption       = &sincChannel3ConvOption;
    sincChannel3Config.chInputOption      = &sincChannel3InputOption;
    sincChannel3Config.chProtectionOption = &sincChannel3ProtectionOption;
    sincChannel3Config.dataFormat         = kSINC_LeftJustifiedSigned;
    sincChannel3Config.u8FifoWaterMark    = 1U;

    SINC_GetDefaultConfig(&sincConfig);

    sincConfig.modClkDivider          = 8UL; // MCLK0 is 16.5 MHz
    sincConfig.clockPreDivider        = kSINC_ClkPrescale1;
    sincConfig.channelsConfigArray[3] = &sincChannel3Config;
    sincConfig.enableMaster           = true;
    sincConfig.disableDozeMode        = false;
    sincConfig.disableModClk1Output   = true;
    sincConfig.disableModClk2Output   = true;
    sincConfig.disableModClk0Output   = false;
    SINC_Init(DEMO_SINC, &sincConfig);
    while (!SINC_CheckChannelReadyForConv(DEMO_SINC, kSINC_Channel3))
        ;

    SINC_EnableInterrupts(DEMO_SINC, kSINC_CH3ConvCompleteIntEnable);
    EnableIRQ(DEMO_SINC_IRQn);
    SINC_SetChannelDebugOutput(DEMO_SINC, kSINC_Channel3, kSINC_Debug_CicRawData);
    while (1)
    {
        PRINTF("\r\nPress any key to trigger conversion!\r\n");
        GETCHAR();
        SINC_AffirmChannelSoftwareTrigger(DEMO_SINC, (1UL << (uint32_t)kSINC_Channel3));
    }
}
