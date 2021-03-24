/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_lptmr.h"
#include "fsl_lpadc.h"
#include "fsl_spm.h"

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* LPTMRx. */
#define LPTMRx_BASE         LPTMR0
#define LPTMRx_INTERVAL_MS  1000U /* 50s. */
#define LPTMRx_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_LpoClk)
#define LPTMRx_IRQn         LPTMR0_IRQn
#define LPTMRx_IRQHandler   LPTMR0_IRQHandler

/* LPADCx. */
#define LPADCx_BASE                         LPADC0
#define LPADCx_USER_CMD_IDX                 1U
#define LPADCx_USER_TRIGGER_IDX             0U
#define LPADCx_MEASURE_VBANDGAP_CHANNEL_NUM 27U /* CH27B, Bandgap. */
#define LPADCx_MEASURE_VBATT_CHANNEL_NUM    31U /* CH31B, DCDC. */

/* Keep the global variables for DCDC framework measurement. */
struct
{
    uint32_t vBatteryValue; /* To keep the measured value for battery. */
    uint32_t vBandgapValue; /* To keep the measured value for bandgap. */
    uint32_t vBatteryMv;    /* To keep the battery voltage value in mV. */
} gAppDcdcFrameInfoStruct;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitDcdcFramework(void);
void APP_StartDcdcBattMonitor(void);
void DCDCx_Configuration(void);
void LPTMRx_Configuration(void);
void LPADCx_Configuration(void);
void GPIOx_Configuration(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_InitBootClocks();

    /* Set the clock source for LPADC0. */
    CLOCK_SetIpSrcDiv(kCLOCK_Lpadc0, kCLOCK_IpSrcFircAsync, 2U, 0U);
    BOARD_InitDebugConsole();

    PRINTF("dcdc_framework demo.\r\n");

    GPIOx_Configuration(); /* LED. */

    /* Initialize the DCDC framework. */
    APP_InitDcdcFramework();

    /* Start the timer to trigger the adjustment periodically. */
    APP_StartDcdcBattMonitor();
    PRINTF("Press any key to trigger the measurement.\r\n");

    while (1)
    {
        /* Press any key in terminal to show the current sample value. */
        GETCHAR();

        PRINTF("\r\n");
        PRINTF("vBandgapValue: %d\r\n", gAppDcdcFrameInfoStruct.vBandgapValue);
        PRINTF("vBatteryValue: %d\r\n", gAppDcdcFrameInfoStruct.vBatteryValue);
        PRINTF("vBatteryMv   : %d\r\n", gAppDcdcFrameInfoStruct.vBatteryMv);
    }
}

/*
 * Initialize the usage of DCDC framework.
 */
void APP_InitDcdcFramework(void)
{
    LPADCx_Configuration(); /* Enable the LPADC to get ready for measuring the battery value. */
    DCDCx_Configuration();  /* Initialize the DCDC function and switch to it as the main power. */
    LPTMRx_Configuration(); /* Enable the LPTMR to trigger the adjust of DCDC periodically. */
}

/*
 * Start the timer, which is to trigger the adjustment periodically.
 */
void APP_StartDcdcBattMonitor(void)
{
    LPTMR_StartTimer(LPTMRx_BASE);
}

/*
 * Update the DCDC input voltage value according to the measured value.
 * 1. Trigger the conversion sequence.
 * 2. Wait the conversion for bandgap value to be done and read the adc value.
 * 3. Wait the conversion for battery value to be done and read the adc value.
 * 4. Calculate the voltage value according to the fomula.
 * 5. Return the calculated  battery value in mV.
 */
uint32_t APP_AdcGetBatteryVoltageMv(void)
{
    lpadc_conv_result_t mLpadcConvResultStruct;

    LPADC_DoSoftwareTrigger(LPADCx_BASE, (1U << LPADCx_USER_TRIGGER_IDX));

    /* Read the value for Bandgap value. */
    while (!LPADC_GetConvResult(LPADCx_BASE, &mLpadcConvResultStruct))
        ;
    gAppDcdcFrameInfoStruct.vBandgapValue = mLpadcConvResultStruct.convValue;

    /* Read the value for battery value. */
    while (!LPADC_GetConvResult(LPADCx_BASE, &mLpadcConvResultStruct))
        ;
    gAppDcdcFrameInfoStruct.vBatteryValue = mLpadcConvResultStruct.convValue * 4U;

    /* Calculate the actual DCDC input (battery) voltage value. */
    gAppDcdcFrameInfoStruct.vBatteryMv =
        gAppDcdcFrameInfoStruct.vBatteryValue * 1000 / gAppDcdcFrameInfoStruct.vBandgapValue;

    return gAppDcdcFrameInfoStruct.vBatteryMv;
}

/*
 * LPTMR ISR entry.
 * Adjust the DCDC with the latest battery sample value periodically.
 */
void LPTMRx_IRQHandler(void)
{
    /* Clear the timeout flag. */
    LPTMR_ClearStatusFlags(LPTMRx_BASE, kLPTMR_TimerCompareFlag);

    /* Get the measure value from ADC and setup the monitor value into DCDC. */
    SPM_SetDcdcBattMonitor(SPM, APP_AdcGetBatteryVoltageMv() / 8U);
    /* Wait for the DCDC to startup and be stable. */
    while ((uint32_t)kSPM_DcdcStableOKFlag != ((uint32_t)kSPM_DcdcStableOKFlag & SPM_GetDcdcStatusFlags(SPM)))
    {
    }

    /* Toggle LED to show that the adjustment is excuted. */
    GPIO_PortToggle(BOARD_LED1_GPIO, 1U << BOARD_LED1_GPIO_PIN);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

/*
 * Initialize the DCDC and enable it as the main power source for the whole SoC.
 */
void DCDCx_Configuration(void)
{
    uint32_t batteryVoltageMv;
    spm_dcdc_integrator_config_t mSpmDcdcIntegratorConfigStruct;

    /* Enable the internal measure channel and set the sample value with div 4. */
    SPM_SetDcdcVbatAdcMeasure(SPM, kSPM_DcdcVbatAdcDivider4);

    /* Set the DCDC output voltage. */
    SPM_SetDcdcVdd1p2ValueBuck(SPM, 0x6); /* Trim the 1P2 to output 1.25V */
    SPM_SetDcdcVdd1p8Value(SPM, 0x14);    /* Trim the 1P8 to output 1.8V */

    /* Trigger the ADC conversion and get the measure value of DCDC. */
    batteryVoltageMv = APP_AdcGetBatteryVoltageMv();

    /* Setup the integrator value into DCDC hardware, as calibration for the environment. */
    mSpmDcdcIntegratorConfigStruct.vddCoreValue = 1.25f;
    mSpmDcdcIntegratorConfigStruct.vBatValue    = (float)batteryVoltageMv / 1000.0f;
    SPM_SetDcdcIntegratorConfig(SPM, &mSpmDcdcIntegratorConfigStruct);

    /* Setup the batt monitor value initially. */
    SPM_SetDcdcBattMonitor(SPM, batteryVoltageMv / 8); /* BattMonitor value is count by 8 mv. */

    /* Disable the step function. */
    SPM_EnableVddxStepLock(SPM, true);

    /* Power down output range comparator. */
    SPM_EnablePowerDownCmpOffset(SPM, false);

    /* Configure the DCDC drive strength. */
    SPM_SetDcdcDriveStrength(SPM, kSPM_DcdcDriveStrengthWithAllFETs);

    /* Enable DCDC, and do the switch from LDO to DCDC. */
    SPM_EnableRegulatorInRunMode(SPM, true, kSPM_DcdcLdo); /* Enable the DCDC. */

    /* Wait for the DCDC to startup and be stable. */
    while ((uint32_t)kSPM_DcdcStableOKFlag != ((uint32_t)kSPM_DcdcStableOKFlag & SPM_GetDcdcStatusFlags(SPM)))
    {
    }
}

/*
 * Initialize the LPTMR to generate the interrupt periodically
 * so that to trigger the update for VBAT monitor value.
 */
void LPTMRx_Configuration(void)
{
    lptmr_config_t mLptmrConfigStruct;
    /* Configure LPTMR */
    /*
     * lptmrConfig.timerMode = kLPTMR_TimerModeTimeCounter;
     * lptmrConfig.pinSelect = kLPTMR_PinSelectInput_0;
     * lptmrConfig.pinPolarity = kLPTMR_PinPolarityActiveHigh;
     * lptmrConfig.enableFreeRunning = false;
     * lptmrConfig.bypassPrescaler = true;
     * lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;
     * lptmrConfig.value = kLPTMR_Prescale_Glitch_0;
     */
    LPTMR_GetDefaultConfig(&mLptmrConfigStruct);
    LPTMR_Init(LPTMRx_BASE, &mLptmrConfigStruct);
    LPTMR_SetTimerPeriod(LPTMRx_BASE, MSEC_TO_COUNT(LPTMRx_INTERVAL_MS, LPTMRx_SOURCE_CLOCK));
    /* Enable LPTMR interrupt. */
    LPTMR_EnableInterrupts(LPTMRx_BASE, kLPTMR_TimerInterruptEnable); /* Enable timer interrupt */
    EnableIRQ(LPTMRx_IRQn);                                           /* Enable at the NVIC */
}

/*
 * Initialize the ADC module.
 * 1. Setup two sample tasks, one is for bandgap, the other is for the DCDC input.
 * 2. The two sample tasks are chained as a conversion sequence.
 * 3. Software trigger is used to trigger the conversion sequence.
 */
void LPADCx_Configuration(void)
{
    lpadc_config_t mLpadcConfigStruct;
    lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
    lpadc_conv_command_config_t mLpadcCommandConfigStruct;

    /* Configure the converter. */
    LPADC_GetDefaultConfig(&mLpadcConfigStruct);
    mLpadcConfigStruct.enableAnalogPreliminary = true; /* Always enable the ADC circuits. */
    LPADC_Init(LPADCx_BASE, &mLpadcConfigStruct);

#if defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && FSL_FEATURE_LPADC_HAS_CFG_CALOFS
    /* Do auto calibration. */
    LPADC_DoAutoCalibration(LPADCx_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CFG_CALOFS */

    /* Set trigger configuration.
     * Software trigger is used for command LPADCx_USER_CMD_IDX.
     */
    LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
    mLpadcTriggerConfigStruct.targetCommandId       = LPADCx_USER_CMD_IDX;
    mLpadcTriggerConfigStruct.enableHardwareTrigger = false;
    LPADC_SetConvTriggerConfig(LPADCx_BASE, LPADCx_USER_TRIGGER_IDX,
                               &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */

    /* Set conversion command.
     * Bandgap voltage is measured with command LPADCx_USER_CMD_ID,
     * VBatt   voltage is measured with command LPADCx_USER_CMD_ID+1.
     * These two conversion tasks are chained as a conversion sequence.
     */
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    /* for bandgap. */
    mLpadcCommandConfigStruct.channelNumber            = LPADCx_MEASURE_VBANDGAP_CHANNEL_NUM;
    mLpadcCommandConfigStruct.hardwareAverageMode      = kLPADC_HardwareAverageCount128;
    mLpadcCommandConfigStruct.sampleChannelMode        = kLPADC_SampleChannelSingleEndSideB;
    mLpadcCommandConfigStruct.chainedNextCommandNumber = LPADCx_USER_CMD_IDX + 1U; /* Chain to next command. */
    LPADC_SetConvCommandConfig(LPADCx_BASE, LPADCx_USER_CMD_IDX, &mLpadcCommandConfigStruct);
    /* for DCDC input voltage. */
    mLpadcCommandConfigStruct.channelNumber            = LPADCx_MEASURE_VBATT_CHANNEL_NUM;
    mLpadcCommandConfigStruct.hardwareAverageMode      = kLPADC_HardwareAverageCount128;
    mLpadcCommandConfigStruct.sampleChannelMode        = kLPADC_SampleChannelSingleEndSideB;
    mLpadcCommandConfigStruct.chainedNextCommandNumber = 0U; /* End of the conversion sequence. */
    LPADC_SetConvCommandConfig(LPADCx_BASE, LPADCx_USER_CMD_IDX + 1U, &mLpadcCommandConfigStruct);

    /* Prior to reading from this ADC BANDGAP channel, ensure that the bandgap buffer is enabled. */
    SPM_SetCoreLdoLowPowerModeConfig(
        SPM, kSPM_CoreLdoLowPowerModeEnableBandgapInVLPx | kSPM_CoreLdoLowPowerModeEnableBandgapBuffer);
}

/*
 * Initialize the GPIO to toggle the LED.
 * 1. LED is used to show that the application is active.
 */
void GPIOx_Configuration(void)
{
    gpio_pin_config_t mGpioPinConfigStruct;

    mGpioPinConfigStruct.outputLogic  = 1U; /* High. */
    mGpioPinConfigStruct.pinDirection = kGPIO_DigitalOutput;
    GPIO_PinInit(BOARD_LED1_GPIO, BOARD_LED1_GPIO_PIN, &mGpioPinConfigStruct);
}
