/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_smc.h"
#include "fsl_pmc.h"
#include "pin_mux.h"
#include "peripherals.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * These values are used to get the temperature. DO NOT MODIFY
 * The method used in this demo to calculate temperature of chip is mapped to
 * Temperature Sensor for the HCS08 Microcontroller Family document (Document Number: AN3031)
 */
#define ADCR_VDD      (65535U) /* Maximum value when use 16b resolution */
#define V_BG          (1000U)  /* BANDGAP voltage in mV (trim to 1.0V) */
#define V_TEMP25      (716U)   /* Typical VTEMP25 in mV */
#define M             (1620U)  /* Typical slope: (mV x 1000)/oC */
#define STANDARD_TEMP (25U)

#define LED1_ON()  LED_RED_ON()
#define LED1_OFF() LED_RED_OFF()

#define LED2_ON()     LED_GREEN_ON()
#define LED2_OFF()    LED_GREEN_OFF()
#define LED2_TOGGLE() LED_GREEN_TOGGLE()

#define LED3_ON()  LED_BLUE_ON()
#define LED3_OFF() LED_BLUE_OFF()
#define DEMO_ADC16_CHANNEL_GROUP 0U

#define DEMO_ADC16_CHANNEL_TEMPSENSOR 0U
#define DEMO_ADC16_CHANNEL_BANDGAP    1U

#define UPPER_VALUE_LIMIT                                                 \
    (1U) /*!< This value/10 is going to be added to current Temp to set \ \
              the upper boundary */
#define LOWER_VALUE_LIMIT                                                 \
    (1U) /*!< This Value/10 is going to be subtracted from current Temp \ \
              to set the lower boundary */
#define UPDATE_BOUNDARIES_TIME                                              \
    (20U) /*!< This value indicates the number of cycles needed to update \ \
               boundaries. To know the Time it will take, \                 \
               multiply this value by period set in DEMO_LPTMR_INPUT_FREQ \ \
               for LPTMR interrupt time in period of 500 milliseconds */

/*!
 * @brief Boundaries structure
 */
typedef struct lowPowerAdcBoundaries
{
    int32_t upperBoundary; /*!< upper boundary in degree */
    int32_t lowerBoundary; /*!< lower boundary in degree */
} lowPowerAdcBoundaries_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief ADC stop conversion
 *
 * @param base The ADC instance number
 */
static void ADC16_PauseConversion(ADC_Type *base);

/*!
 * @brief calibrate parameters: VDD and ADCR_TEMP25
 *
 * @param base The ADC instance number
 */
static void ADC16_CalibrateParams(ADC_Type *base);

/*!
 * @brief Initialize the ADCx for HW trigger.
 *
 * @param base The ADC instance number
 *
 * @return true if success
 */
static bool ADC16_InitHardwareTrigger(ADC_Type *base);

/*!
 * @brief User-defined function to init trigger source  of LPTimer
 *
 * @param base The LPTMR instance number
 */
static void LPTMR_InitTriggerSourceOfAdc(LPTMR_Type *base);

/*!
 * @brief Calculate current temperature.
 *
 * @return uint32_t Returns current temperature.
 */
static int32_t GetCurrentTempValue(void);

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
volatile static uint32_t g_adcValue = 0; /*!< ADC value */
static uint32_t g_adcrTemp25        = 0; /*!< Calibrated ADCR_TEMP25 */
static uint32_t g_adcr100m          = 0;
volatile bool g_conversionCompleted = false; /*!< Conversion is completed flag */

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Enable the trigger source of LPTimer */
static void LPTMR_InitTriggerSourceOfAdc(LPTMR_Type *base)
{
    /* Start the LPTimer */
    LPTMR_StartTimer(base);

    /* Configure SIM for ADC hardware trigger source selection of Low-power timer (LPTMR) trigger. */
    BOARD_ConfigTriggerSource();
}

/*!
 * @brief ADC stop conversion
 */
static void ADC16_PauseConversion(ADC_Type *base)
{
    adc16_channel_config_t adcChnConfig; /*!< ADC16 channel conversion configuration */

    /* Configure to set ADC channels measurement disabled */
    adcChnConfig.channelNumber                        = 31U;
    adcChnConfig.enableInterruptOnConversionCompleted = false;
#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    adcChnConfig.enableDifferentialConversion = false;
#endif

    /* Configure ADC channels */
    ADC16_SetChannelConfig(base, DEMO_ADC16_CHANNEL_GROUP, &adcChnConfig);
}

/*!
 * @brief calibrate parameters: VDD and ADCR_TEMP25
 */
static void ADC16_CalibrateParams(ADC_Type *base)
{
    uint32_t bandgapValue = 0; /*!< ADC value of BANDGAP */
    uint32_t vdd          = 0; /*!< VDD in mV */

    pmc_bandgap_buffer_config_t pmcBandgapConfig;

    pmcBandgapConfig.enable = true;

#if (defined(FSL_FEATURE_PMC_HAS_BGEN) && FSL_FEATURE_PMC_HAS_BGEN)
    pmcBandgapConfig.enableInLowPowerMode = false;
#endif
#if (defined(FSL_FEATURE_PMC_HAS_BGBDS) && FSL_FEATURE_PMC_HAS_BGBDS)
    pmcBandgapConfig.drive = kPmcBandgapBufferDriveLow;
#endif
    /* Enable BANDGAP reference voltage */
    PMC_ConfigureBandgapBuffer(PMC, &pmcBandgapConfig);

    /*
     * Initialization of ADC for 16-bit resolution, interrupt mode, HW trigger disabled,
     * normal conversion speed, VREFH/L as reference and disabled continuous convert mode.
     */
    BOARD_InitADCPeripheral(); /*!< Initialization of the ADC16 peripheral */

#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    /* Auto calibration */
    if (kStatus_Success == ADC16_DoAutoCalibration(base))
    {
        PRINTF("ADC16_DoAutoCalibration() Done.\r\n");
    }
    else
    {
        PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
    }
#endif

    /* select kAdcChannelBandgap channel for measurement */
    ADC16_SetChannelConfig(base, DEMO_ADC16_CHANNEL_GROUP, &DEMO_ADC16_channelsConfig[DEMO_ADC16_CHANNEL_BANDGAP]);

    /* Wait for the conversion to be done */
    while (!ADC16_GetChannelStatusFlags(base, DEMO_ADC16_CHANNEL_GROUP))
    {
    }

    /* Get current ADC BANDGAP value */
    bandgapValue = ADC16_GetChannelConversionValue(base, DEMO_ADC16_CHANNEL_GROUP);

    ADC16_PauseConversion(base);

    /* Get VDD value measured in mV: VDD = (ADCR_VDD x V_BG) / ADCR_BG */
    vdd = ADCR_VDD * V_BG / bandgapValue;
    /* Calibrate ADCR_TEMP25: ADCR_TEMP25 = ADCR_VDD x V_TEMP25 / VDD */
    g_adcrTemp25 = ADCR_VDD * V_TEMP25 / vdd;
    /* ADCR_100M = ADCR_VDD x M x 100 / VDD */
    g_adcr100m = (ADCR_VDD * M) / (vdd * 10);

    /* Disable BANDGAP reference voltage */
    pmcBandgapConfig.enable = false;
    PMC_ConfigureBandgapBuffer(PMC, &pmcBandgapConfig);
}

/*!
 * @brief Initialize the ADCx for Hardware trigger.
 */
static bool ADC16_InitHardwareTrigger(ADC_Type *base)
{
#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    uint16_t offsetValue = 0; /*!< Offset error from correction value. */

    /* Auto calibration */
    if (kStatus_Success != ADC16_DoAutoCalibration(base))
    {
        return false;
    }
    offsetValue = base->OFS;
    ADC16_SetOffsetValue(base, offsetValue);
#endif

    /* enable hardware trigger  */
    ADC16_EnableHardwareTrigger(base, true);

    /* select kAdcChannelTemperature channel for measurement */
    ADC16_SetChannelConfig(base, DEMO_ADC16_CHANNEL_GROUP, &DEMO_ADC16_channelsConfig[DEMO_ADC16_CHANNEL_TEMPSENSOR]);

    return true;
}

static int32_t GetCurrentTempValue(void)
{
    int32_t currentTemperature = 0;
    /* Temperature = 25 - (ADCR_T - ADCR_TEMP25) * 100 / ADCR_100M */
    currentTemperature =
        (int32_t)(STANDARD_TEMP - ((int32_t)g_adcValue - (int32_t)g_adcrTemp25) * 100 / (int32_t)g_adcr100m);
    return currentTemperature;
}

static lowPowerAdcBoundaries_t TempSensorCalibration(uint32_t updateBoundariesCounter, int32_t *tempArray)
{
    uint32_t avgTemp = 0;
    lowPowerAdcBoundaries_t boundaries;

    for (int i = 0; i < updateBoundariesCounter; i++)
    {
        avgTemp += tempArray[i];
    }
    /* Get average temperature */
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
 * Get current ADC value and set g_conversionCompleted flag.
 */
void DEMO_ADC16_IRQHANDLER(void)
{
    /* GREEN LED is blinking indicating the initial temperature measurement is still in progress */
    LED2_TOGGLE();

    /* Get current ADC value */
    g_adcValue = ADC16_GetChannelConversionValue(DEMO_ADC16_PERIPHERAL, DEMO_ADC16_CHANNEL_GROUP);
    /* Set conversion completed flag. This prevents an wrong conversion in main function */
    g_conversionCompleted = true;
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief main function
 */
int main(void)
{
    int32_t currentTemperature       = 0;
    uint32_t updateBoundariesCounter = 0;
    int32_t tempArray[UPDATE_BOUNDARIES_TIME * 2];
    lowPowerAdcBoundaries_t boundaries;

    /* Init board hardware */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitPeripherals();

    /* Set to allow entering vlps mode */
    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlp);

    PRINTF("\n\r ADC LOW POWER PERIPHERAL DEMO... ");

    /* Calibrate param Temperature sensor */
    ADC16_CalibrateParams(DEMO_ADC16_PERIPHERAL);

    /* Initialize Demo ADC */
    if (!ADC16_InitHardwareTrigger(DEMO_ADC16_PERIPHERAL))
    {
        PRINTF("Failed to finish the initialization.\r\n");
        return -1;
    }

    PRINTF("\r\n OPERATING INSTRUCTIONS:");
    PRINTF(
        "\r\n The advanced Low Power ADC project is designed to work with the Tower System or in a stand alone "
        "setting.");
    PRINTF(
        "\r\n The code of this demo has been prepared and updated for use with the MCUXpresso Configuration Tools "
        "(Pins/Clocks/Peripherals).");
    PRINTF("\r\n 1. Set your target board in a place where the temperature is constant.");
    PRINTF("\r\n 2. Wait until the green LED light turns on, after initial temperature measurement finished.");
    PRINTF(
        "\r\n 3. Increment or decrement the temperature to see the changes, red lights for higher and blue one for "
        "lower than average counted temperatures.");
    PRINTF("\r\n Now wait until LED stops blinking...\r\n");

    /* setup the HW trigger source for ADC */
    LPTMR_InitTriggerSourceOfAdc(LPTMR0);

    /* Warm up microcontroller and allow to set first boundaries */
    while (updateBoundariesCounter < (UPDATE_BOUNDARIES_TIME * 2))
    {
        while (!g_conversionCompleted)
        {
        }
        currentTemperature                 = GetCurrentTempValue();
        tempArray[updateBoundariesCounter] = currentTemperature;
        updateBoundariesCounter++;
        g_conversionCompleted = false;
    }

    /* Temp Sensor Calibration */
    boundaries              = TempSensorCalibration(updateBoundariesCounter, tempArray);
    updateBoundariesCounter = 0;

    LPTMR_StopTimer(LPTMR0);

    /* GREEN LED is turned on indicating that initial measurement is finished */
    LED_GREEN_ON();

    /* Wait for user input before beginning demo */
    PRINTF("\r Enter any character to begin the demo...\n");
    GETCHAR();
    PRINTF("\r ---> OK! Main process is running...!\n");

    LPTMR_StartTimer(LPTMR0);

    while (1)
    {
        /* Prevents the use of wrong values */
        while (!g_conversionCompleted)
        {
        }

        /* Get current Temperature Value */
        currentTemperature = GetCurrentTempValue();
        /* Store temperature values that are going to be use to calculate average temperature */
        tempArray[updateBoundariesCounter] = currentTemperature;

        if (currentTemperature > boundaries.upperBoundary)
        {
            LED1_ON(); /*! RED LED is turned on when temperature is above the average */
            LED2_OFF();
            LED3_OFF();
        }
        else if (currentTemperature < boundaries.lowerBoundary)
        {
            LED1_OFF();
            LED2_OFF();
            LED3_ON(); /*! BLUE LED is turned on when temperature is below the average */
        }
        else
        {
            LED1_OFF();
            LED2_ON(); /*! GREEN LED is turned on when temperature is around the average */
            LED3_OFF();
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

        /* Clear conversion completed flag */
        g_conversionCompleted = false;

        /* Enter to Very Low Power Stop Mode */
        SMC_SetPowerModeVlps(SMC);
    }
}

// EOF.
