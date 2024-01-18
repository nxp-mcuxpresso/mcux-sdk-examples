/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_lpi2c.h"
#include "fsl_rgpio.h"
#include "fsl_upower.h"
#include "fsl_reset.h"
#include "fsl_epdc.h"
#include "fsl_pca6416a.h"
#include "fsl_debug_console.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EPDC_PMIC_I2C_ADDR       0x18
#define EPDC_PMIC_I2C            LPI2C1
#define EPDC_PMIC_I2C_CLOCK_FREQ CLOCK_GetLpi2cClkFreq(1)
#define EPDC_FRONT_LIGHT_RGPIO   GPIOA
#define EPDC_FRONT_LIGHT_PIN     3U
#define EPDC_POWER_ENABLE_RGPIO  GPIOE
#define EPDC_POWER_ENABLE_PIN    18U

#define FP9931_TMST_VALUE        0x00
#define FP9931_VCOM_SETTING      0x01
#define FP9931_VPOS_VNEG_SETTING 0x02
#define FP9931_PWRON_DELAY       0x03
#define FP9931_CONTROL_REG1      0x0B
#define FP9931_CONTROL_REG2      0x0C

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_PowerOnEpdc(void)
{
    RESET_PeripheralReset(kRESET_Epdc);
    UPOWER_PowerOnMemPart(kUPOWER_MP0_EPDC_A | kUPOWER_MP0_EPDC_B, 0U);
    CLOCK_SetIpSrcDiv(kCLOCK_Epdc, kCLOCK_Pcc5PlatIpSrcPll4Pfd3Div2, 6, 0U);
}

void DEMO_PowerOnPxp(void)
{
    RESET_PeripheralReset(kRESET_Pxp);
    UPOWER_PowerOnSwitches(kUPOWER_PS_AV_NIC);
    UPOWER_PowerOnMemPart(kUPOWER_MP0_PXP, 0U);
}

status_t DEMO_InitEpdcPanel(void)
{
    status_t status = kStatus_Success;
    uint8_t txData;
    uint8_t rxData;
    /* Define the init structure for the output panel front light pin*/
    rgpio_pin_config_t front_light_config = {
        kRGPIO_DigitalOutput,
        0,
    };

    /* Define the init structure for the output panel power enable pin*/
    rgpio_pin_config_t power_enable_config = {
        kRGPIO_DigitalOutput,
        0,
    };

    RGPIO_PinInit(EPDC_FRONT_LIGHT_RGPIO, EPDC_FRONT_LIGHT_PIN, &front_light_config);
    RGPIO_PinInit(EPDC_POWER_ENABLE_RGPIO, EPDC_POWER_ENABLE_PIN, &power_enable_config);

    PRINTF("\r\n Connect the MIPI panel to MIMX8ULP-EVK EPDC socket.\r\n");
    PRINTF("\r\n Press any key to power on panel.\r\n");
    /* Wait for press any key */
    GETCHAR();

    /* Power on panel. */
    RGPIO_PortSet(EPDC_FRONT_LIGHT_RGPIO, 1U << EPDC_POWER_ENABLE_PIN);
    /* Power on panel frount light. */
    RGPIO_PortSet(EPDC_POWER_ENABLE_RGPIO, 1U << EPDC_FRONT_LIGHT_PIN);

    /* Route EPDC signal. */
    if (kStatus_Success != PCA6416A_SetPins(&g_pca6416aHandle, (1U << BOARD_PCA6416A_EPDC_SWITCH)))
    {
        PRINTF("ERROR: EPDC_SWITCH pin configure failed\r\n");
    }
    if (kStatus_Success !=
        PCA6416A_SetDirection(&g_pca6416aHandle, (1U << BOARD_PCA6416A_EPDC_SWITCH), kPCA6416A_Output))
    {
        PRINTF("ERROR: EPDC_SWITCH pin configure failed\r\n");
    }

    BOARD_LPI2C_Init(EPDC_PMIC_I2C, EPDC_PMIC_I2C_CLOCK_FREQ);

    while (1)
    {
        txData = 0x82;
        status = BOARD_LPI2C_Send(EPDC_PMIC_I2C, EPDC_PMIC_I2C_ADDR, FP9931_VCOM_SETTING, 1, &txData, 1, 0);
        if (kStatus_Success != status)
        {
            break;
        }

        status = BOARD_LPI2C_Receive(EPDC_PMIC_I2C, EPDC_PMIC_I2C_ADDR, FP9931_VCOM_SETTING, 1, &rxData, 1, 0);
        if (kStatus_Success != status)
        {
            break;
        }
        if (txData != rxData)
        {
            continue;
        }

        txData = 0x28;
        status = BOARD_LPI2C_Send(EPDC_PMIC_I2C, EPDC_PMIC_I2C_ADDR, FP9931_VPOS_VNEG_SETTING, 1, &txData, 1, 0);
        if (kStatus_Success != status)
        {
            break;
        }

        status = BOARD_LPI2C_Receive(EPDC_PMIC_I2C, EPDC_PMIC_I2C_ADDR, FP9931_VPOS_VNEG_SETTING, 1, &rxData, 1, 0);
        if (kStatus_Success != status)
        {
            break;
        }
        if (txData != rxData)
        {
            continue;
        }

        txData = 0xE4;
        status = BOARD_LPI2C_Send(EPDC_PMIC_I2C, EPDC_PMIC_I2C_ADDR, FP9931_PWRON_DELAY, 1, &txData, 1, 0);
        if (kStatus_Success != status)
        {
            break;
        }
        status = BOARD_LPI2C_Receive(EPDC_PMIC_I2C, EPDC_PMIC_I2C_ADDR, FP9931_PWRON_DELAY, 1, &rxData, 1, 0);
        if (kStatus_Success != status)
        {
            break;
        }
        if (txData != rxData)
        {
            continue;
        }

        txData = 0x02;
        status = BOARD_LPI2C_Send(EPDC_PMIC_I2C, EPDC_PMIC_I2C_ADDR, FP9931_CONTROL_REG1, 1, &txData, 1, 0);
        if (kStatus_Success != status)
        {
            break;
        }
        status = BOARD_LPI2C_Receive(EPDC_PMIC_I2C, EPDC_PMIC_I2C_ADDR, FP9931_CONTROL_REG1, 1, &rxData, 1, 0);
        if (kStatus_Success != status)
        {
            break;
        }
        if (txData != rxData)
        {
            continue;
        }
        break;
    }

    if (kStatus_Success != status)
    {
        PRINTF("ERROR: EPDC panel communication failed\r\n");
        return -1;
    }

    /* power wake. */
    /* Initializes the EPDC to reset state. */
    EPDC_Init(EPDC);
    const epdc_gpio_config_t gpioConfig = {
        .pwrWakeOutput = 1U, .bdrOutput = 0U, .pwrCtrlOutput = 0U, .pwrComOutput = 0U};
    EPDC_SetGpioOutput(EPDC, &gpioConfig);

    return status;
}

status_t DEMO_GetEpdcTemp(uint8_t *temp)
{
    return BOARD_LPI2C_Receive(EPDC_PMIC_I2C, EPDC_PMIC_I2C_ADDR, FP9931_TMST_VALUE, 1, temp, 1, 0);
}
