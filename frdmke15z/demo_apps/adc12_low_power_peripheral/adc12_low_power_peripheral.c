/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_smc.h"
#include "fsl_pmc.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "peripherals.h"
#include "board.h"

#include "fsl_trgmux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_ADC12_CHANNEL_GROUP 0U

#define LED1_INIT() LED_RED1_INIT(LOGIC_LED_OFF)
#define LED1_ON()   LED_RED1_ON()
#define LED1_OFF()  LED_RED1_OFF()

#define LED2_INIT() LED_GREEN1_INIT(LOGIC_LED_OFF)
#define LED2_ON()   LED_GREEN1_ON()
#define LED2_OFF()  LED_GREEN1_OFF()
#define UPPER_VALUE_LIMIT (1U) /*! This value/10 is going to be added to current Temp to set the upper boundary*/
#define LOWER_VALUE_LIMIT                                                                     \
    (1U) /*! This Value/10 is going to be subtracted from current Temp to set the lower \ \ \ \
            boundary*/
#define UPDATE_BOUNDARIES_TIME                                                             \
    (20U) /*! This value indicates the number of cycles needed to update boundaries. \ \ \ \
              To know the Time it will take, multiply this value times LPTMR_COMPARE_VALUE*/

/*!
 * @brief Boundaries struct
 */
typedef struct lowPowerAdcBoundaries
{
    int32_t upperBoundary; /*! upper boundary of ADC sensor value */
    int32_t lowerBoundary; /*! lower boundary of ADC sensor value */
} lowPowerAdcBoundaries_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_ConfigTriggerSource(void);
/*!
 * @brief Calculate current temperature.
 *
 * @param updateBoundariesCounter Indicate number of values into tempArray.
 *
 * @param tempArray Store temperature value.
 *
 * @return lowPowerAdcBoundaries_t Returns upper and lower temperature boundaries.
 */
static lowPowerAdcBoundaries_t TempSensorCalibration(uint32_t updateBoundariesCounter, int32_t *tempArray);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile static uint32_t adcValue = 0;     /*! ADC value */
volatile bool conversionCompleted = false; /*! Conversion is completed Flag */

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_BootClockRUN(void)
{
    const scg_sosc_config_t g_scgSysOscConfig = {.freq        = BOARD_XTAL0_CLK_HZ,
                                                 .enableMode  = kSCG_SysOscEnable | kSCG_SysOscEnableInLowPower,
                                                 .monitorMode = kSCG_SysOscMonitorDisable,
                                                 .div2        = kSCG_AsyncClkDivBy1,
                                                 .workMode    = kSCG_SysOscModeOscLowPower};

    const scg_sys_clk_config_t g_sysClkConfigSircSource = {
        .divSlow = kSCG_SysClkDivBy4, .divCore = kSCG_SysClkDivBy1, .src = kSCG_SysClkSrcSirc};

    const scg_firc_config_t g_scgFircConfig = {
        .enableMode = kSCG_FircEnable, .div2 = kSCG_AsyncClkDivBy1, .range = kSCG_FircRange48M, .trimConfig = NULL};

    const scg_lpfll_config_t g_scgLpFllConfig = {
        .enableMode = kSCG_LpFllEnable, .div2 = kSCG_AsyncClkDivBy2, .range = kSCG_LpFllRange72M, .trimConfig = NULL};

    const scg_sys_clk_config_t g_sysClkConfigNormalRun = {
        .divSlow = kSCG_SysClkDivBy3, .divCore = kSCG_SysClkDivBy1, .src = kSCG_SysClkSrcLpFll};

    const scg_sirc_config_t scgSircConfig = {
        .enableMode = kSCG_SircEnable | kSCG_SircEnableInLowPower | kSCG_SircEnableInStop,
        .div2       = kSCG_AsyncClkDivBy2,
        .range      = kSCG_SircRangeHigh};

    scg_sys_clk_config_t curConfig;

    CLOCK_InitSysOsc(&g_scgSysOscConfig);
    CLOCK_SetXtal0Freq(BOARD_XTAL0_CLK_HZ);

    /* Init Sirc */
    CLOCK_InitSirc(&scgSircConfig);

    /* Change to use SIRC as system clock source to prepare to change FIRCCFG register*/
    CLOCK_SetRunModeSysClkConfig(&g_sysClkConfigSircSource);

    /* Wait for clock source switch finished. */
    do
    {
        CLOCK_GetCurSysClkConfig(&curConfig);
    } while (curConfig.src != g_sysClkConfigSircSource.src);

    /* Init Firc */
    CLOCK_InitFirc(&g_scgFircConfig);

    /* Init LPFLL */
    CLOCK_InitLpFll(&g_scgLpFllConfig);

    /* Use LPFLL as system clock source */
    CLOCK_SetRunModeSysClkConfig(&g_sysClkConfigNormalRun);

    /* Wait for clock source switch finished. */
    do
    {
        CLOCK_GetCurSysClkConfig(&curConfig);
    } while (curConfig.src != g_sysClkConfigNormalRun.src);

    SystemCoreClock = 72000000U;
}

/* Set which ADC's clock source to be generated by PCC. */

void BOARD_ConfigTriggerSource(void)
{
    TRGMUX_SetTriggerSource(TRGMUX0, kTRGMUX_Adc0, kTRGMUX_TriggerInput1, kTRGMUX_SourceLptmr0);

    /* Configure SIM for ADC hardware trigger source selection */
    /* Use LPTMR as trigger source, select software pre-trigger 0*/
    SIM->ADCOPT |= SIM_ADCOPT_ADC0TRGSEL(1U);
    SIM->ADCOPT |= SIM_ADCOPT_ADC0PRETRGSEL(2U);
    SIM->ADCOPT |= SIM_ADCOPT_ADC0SWPRETRG(1U);
}
static lowPowerAdcBoundaries_t TempSensorCalibration(uint32_t updateBoundariesCounter, int32_t *tempArray)
{
    uint32_t avgTemp = 0;
    lowPowerAdcBoundaries_t boundaries;

    for (int i = 0; i < updateBoundariesCounter; i++)
    {
        avgTemp += tempArray[i];
    }
    /* Get average value */
    avgTemp /= updateBoundariesCounter;

    /* Set upper boundary */
    boundaries.upperBoundary = avgTemp + UPPER_VALUE_LIMIT;

    /* Set lower boundary */
    boundaries.lowerBoundary = avgTemp - LOWER_VALUE_LIMIT;

    return boundaries;
}

/*!
 * @brief ADC Interrupt handler
 *
 * Get current ADC value and set conversionCompleted flag.
 */
void DEMO_ADC12_IRQHANDLER(void)
{
    /* Get current ADC value */
    adcValue = ADC12_GetChannelConversionValue(DEMO_ADC12_PERIPHERAL, DEMO_ADC12_CHANNEL_GROUP);
    /* Set conversionCompleted flag. This prevents an wrong conversion in main function */
    conversionCompleted = true;
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief main function
 */
int main(void)
{
    uint32_t updateBoundariesCounter = 0;
    int32_t tempArray[UPDATE_BOUNDARIES_TIME * 2];
    lowPowerAdcBoundaries_t boundaries;

    /* Init hardware */
    BOARD_InitBootPins();
    APP_BootClockRUN();

    BOARD_InitDebugConsole();

    /* When in VLPx mode, the ADC clock source is only limited to SOSC and SIRC.*/
    CLOCK_SetIpSrc(kCLOCK_Adc0, kCLOCK_IpSrcSircAsync);
    BOARD_InitPeripherals();
    /* Init used Led in Demo app */
    LED1_INIT();
    LED2_INIT();

    /* Set to allow entering vlps mode */
    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlp);

    /* Configure channel */
    ADC12_SetChannelConfig(DEMO_ADC12_PERIPHERAL, DEMO_ADC12_CHANNEL_GROUP, DEMO_ADC12_channelsConfig);

    /* Start the LPTimer */
    LPTMR_StartTimer(DEMO_LPTMR_PERIPHERAL);
    /* Configure SIM for ADC hw trigger source selection */
    BOARD_ConfigTriggerSource();

    PRINTF("\r\n ADC LOW POWER PERIPHERAL DEMO\r\n");
    PRINTF(" The Low Power ADC project is designed to work with the Kinetis SDK.\r\n");
    PRINTF(" 1. Set your target board in a place where the temperature is constant.\r\n");
    PRINTF(" 2. Wait until two Led light turns on.\r\n");
    PRINTF(" 3. Increment or decrement the temperature to see the changes.\r\n");
    PRINTF(" Wait two led on...\r\n");

    /* Warm up microcontroller and allow to set first boundaries */
    while (updateBoundariesCounter < (UPDATE_BOUNDARIES_TIME * 2))
    {
        while (!conversionCompleted)
        {
        }
        tempArray[updateBoundariesCounter] = adcValue;
        updateBoundariesCounter++;
        conversionCompleted = false;
    }

    /* Temp Sensor Calibration */
    boundaries              = TempSensorCalibration(updateBoundariesCounter, tempArray);
    updateBoundariesCounter = 0;

    /* Two LED is turned on indicating calibration is done */
    LED1_ON();
    LED2_ON();

    /* Wait for user input before beginning demo */
    PRINTF("\r Enter any character to begin...\n");
    GETCHAR();
    PRINTF("\r ---> OK! Main process is running...!\n");

    while (1)
    {
        /* Prevents the use of wrong values */
        while (!conversionCompleted)
        {
        }

        /* Store temperature values that are going to be use to calculate average temperature */
        tempArray[updateBoundariesCounter] = adcValue;

        /* The temperature is higher, the adcValue reads lower */
        if (adcValue > boundaries.upperBoundary)
        {
            LED2_ON();
            LED1_OFF();
        }
        else if (adcValue < boundaries.lowerBoundary)
        {
            LED2_OFF();
            LED1_ON();
        }
        else
        {
            LED2_ON();
            LED1_ON();
        }

        /* Call update function */
        if (updateBoundariesCounter >= (UPDATE_BOUNDARIES_TIME))
        {
            boundaries              = TempSensorCalibration(updateBoundariesCounter, tempArray);
            updateBoundariesCounter = 0;
        }
        else
        {
            updateBoundariesCounter++;
        }

        /* Clear conversionCompleted flag */
        conversionCompleted = false;

        /* Enter to Very Low Power Stop Mode */
        SMC_SetPowerModeVlps(SMC);
    }
}
