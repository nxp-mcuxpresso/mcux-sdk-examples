/*
 * Copyright 2017-2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "app_srtm.h"
#include "fsl_wdog32.h"

#include "main/imx8qx_pads.h"
#include "svc/pad/pad_api.h"
#include "fsl_gpio.h"
#include "fsl_irqsteer.h"
/*******************************************************************************
 * Struct Definitions
 ******************************************************************************/
#define DEMO_ESAI                    ADMA__ESAI0
#define ESAI_SOURCE_CLOCK_FREQ       (49152000)
#define EXAMPLE_LPI2C                (CM4__LPI2C)                       /*Should change to MIPI_CSI_I2C0*/
#define EXAMPLE_LPI2C_BAUDRATE       (400000)                           /*in i2c example it is 100000*/
#define I2C_SOURCE_CLOCK_FREQ_M4     CLOCK_GetIpFreq(kCLOCK_M4_0_Lpi2c) /*M4_LPI2C*/
#define I2C_SOURCE_CLOCK_FREQ_LPI2C1 CLOCK_GetIpFreq(kCLOCK_DMA_Lpi2c1) /*ADMA_LPI2C1*/
#define AUDIO_IRQHandler             IRQSTEER_6_IRQHandler
#define CODEC_CS42888                (1)
#define ESAI_TX_CHANNEL              (2)
/* SAI and I2C instance and clock */
#define DEMO_CODEC_WM8960
#define DEMO_SAI          ADMA__SAI1
#define DEMO_SAI_CHANNEL  (0)
#define DEMO_SAI_BITWIDTH (kSAI_WordWidth16bits)
// #define DEMO_I2C CM4__LPI2C
#define DEMO_SAI_CLK_FREQ (24576000U)
// #define DEMO_I2C_CLK_FREQ CLOCK_GetIpFreq(kCLOCK_M4_0_Lpi2c)
#define CODEC_CYCLE (30000000)

#define EXAMPLE_I2C_EXPANSION_B_ADDR (0x1D)
#define PCA9557_RST_GPIO             LSIO__GPIO1 /*SPI2_SDO, GPIO1, IO1, ALT4*/
#define PCA9557_RST_PIN              1

/*! @brief PCA9557 Registers address definition. */
#define PCA9557_REG_INTPUT_PORT        (0x00)
#define PCA9557_REG_OUTPUT_PORT        (0x01)
#define PCA9557_REG_POLARITY_INVERSION (0x02)
#define PCA9557_REG_CONFIGURATION      (0x03)

// #define EXAMPLE_I2C_SWITCH_ADDR (0x71)
/*
 * We use 32KHz Clk divided by 256 as WDOG Clock Source
 */
#define WDOG_TIMEOUT_1S (32768 / 256)
#define WDOG_TIMEOUT    (5 * WDOG_TIMEOUT_1S)
/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function Code
 ******************************************************************************/
static void WDOG32_Configure(uint16_t timeout)
{
    wdog32_config_t config;

    WDOG32_GetDefaultConfig(&config);
    config.testMode = kWDOG32_UserModeEnabled;

    config.clockSource  = kWDOG32_ClockSource1; // 2, internal clock 8MHz, 1, LPO OSC 32KHz
    config.prescaler    = kWDOG32_ClockPrescalerDivide256;
    config.windowValue  = 0U;
    config.timeoutValue = timeout;

    config.enableWindowMode = false;
    config.enableWdog32     = true;

    WDOG32_Init(CM4__WDOG, &config);
}

static void srtm_codec_demo(void *pvParameters)
{
    char c;
    PRINTF("\r\n####################  CODEC SRTM DEMO ####################\n\r\n");
    PRINTF("    Build Time: %s--%s \n\r\n", __DATE__, __TIME__);
    PRINTF("##########################################################\r\n");

    APP_SRTM_StartCommunication();

    for (;;)
    {
        PRINTF("Press 'r' to do M4 partition reset\r\n");
        c = GETCHAR();
        PRINTF("%c\r\n", c);
        if (c == 'r')
        {
            WDOG32_Configure(WDOG_TIMEOUT);
            PRINTF("Wait a while to reboot\r\n");
            break;
        }
    }

    while (true)
        ;
}

void vApplicationMallocFailedHook(void)
{
    PRINTF("Malloc Failed!!!\r\n");
}

static void APP_PeerCoreResetHandler(void *param)
{
    /*
     * In this App simply call Handle function from APP SRTM
     */
    APP_SRTM_HandlePeerReboot();
}

/*! @brief Main function */
int main(void)
{
    sc_ipc_t ipc;
    uint32_t freq;
    sc_pm_clock_rate_t src_rate = SC_133MHZ;

    ipc = BOARD_InitRpc();
    BOARD_InitPins(ipc);
    BOARD_PowerOnBaseBoard();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();
    BOARD_InitMemory();

    /* Power on LPI2C. */
    if (sc_pm_set_resource_power_mode(ipc, SC_R_M4_0_I2C, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on LPI2C\r\n");
    }
    /* Set LPI2C clock */
    freq = CLOCK_SetIpFreq(kCLOCK_M4_0_Lpi2c, SC_24MHZ);
    if (freq == 0)
    {
        PRINTF("Error: Failed to set LPI2C frequency\r\n");
    }
    if (!CLOCK_EnableClockExt(kCLOCK_M4_0_Lpi2c, 0))
    {
        PRINTF("Error: sc_pm_clock_enable failed\r\n");
    }

    /*
     * LPI2C1
     */

    if (sc_pm_set_resource_power_mode(ipc, SC_R_I2C_1, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on LPI2C\r\n");
    }
    if (sc_pm_clock_enable(ipc, SC_R_I2C_1, SC_PM_CLK_PER, true, false) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to enable SC_R_I2C_1 clock \r\n");
    }

    if (sc_pm_set_clock_rate(ipc, SC_R_I2C_1, SC_PM_CLK_PER, &src_rate) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to set SC_R_I2C_1 clock rate\r\n");
    }

    /*
     *
     */
    if (sc_pm_set_resource_power_mode(ipc, SC_R_MU_5B, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on MU_5B!\r\n");
    }

    if (sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_0, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on IRQSTEER!\r\n");
    }

    IRQSTEER_Init(IRQSTEER);
    IRQSTEER_EnableInterrupt(IRQSTEER, LSIO_MU8_INT_B_IRQn);

    if (sc_pm_set_resource_power_mode(ipc, SC_R_MU_8B, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on MU_8B!\r\n");
    }

    /*
     * Enable A35 Reboot IRQ
     */
    BOARD_RegisterEventHandler(kSCEvent_PeerCoreReboot, APP_PeerCoreResetHandler, NULL);

    BOARD_EnableSCEvent(SC_EVENT_MASK(kSCEvent_PeerCoreReboot), true);
    BOARD_Enable_SCIRQ(true);
    /*
     * Output Demo Information
     */
    APP_SRTM_Init();

    if (xTaskCreate(srtm_codec_demo, "SRTMDemo", 512U, NULL, tskIDLE_PRIORITY + 1U, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
    }

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Application should never reach this point. */
    for (;;)
    {
    }
}
