/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Board includes */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "main.h"
#include "cmd.h"
#include "fsl_os_abstraction.h"

#ifdef SD_ENABLED
#include "fsl_sd.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "sdmmc_config.h"
#endif

#include "fsl_debug_console.h"

#include "fsl_codec_common.h"
#include "fsl_dialog7212.h"
#include "fsl_codec_adapter.h"
#include "fsl_sai.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#include "app_definitions.h"
#define APP_SHELL_TASK_STACK_SIZE (256)
#define SDCARD_TASK_STACK_SIZE    (512)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int BOARD_CODEC_Init(void);
#ifdef SD_ENABLED
static void APP_SDCARD_DetectCallBack(bool isInserted, void *userData);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
codec_handle_t codecHandle    = {0};
da7212_pll_config_t pllConfig = {
    .source         = kDA7212_PLLClkSourceMCLK,
    .refClock_HZ    = 12288000U,
    .outputClock_HZ = kDA7212_PLLOutputClk12288000,
};
da7212_config_t da7212Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = 12000000},
    .dacSource    = kDA7212_DACSourceInputStream,
    .slaveAddress = DA7212_ADDRESS,
    .protocol     = kDA7212_BusI2S,
    .format       = {.mclk_HZ = 12288000U, .sampleRate = DEMO_AUDIO_SAMPLE_RATE, .bitWidth = DEMO_AUDIO_BIT_WIDTH},
    .pll          = &pllConfig,
    .sysClkSource = kDA7212_SysClkSourcePLL,
    .isMaster     = true,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_DA7212, .codecDevConfig = &da7212Config};

sai_master_clock_t mclkConfig;


static app_handle_t app;
/*******************************************************************************
 * Code
 ******************************************************************************/
int BOARD_CODEC_Init(void)
{
    CODEC_Init(&codecHandle, &boardCodecConfig);

    /* Initial volume kept low for hearing safety. */
    CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, 50);

    return 0;
}


void BOARD_MASTER_CLOCK_CONFIG(void)
{
    mclkConfig.mclkOutputEnable = true, mclkConfig.mclkHz = 12288000U;
    mclkConfig.mclkSourceClkHz = 12288000U;
    SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
}

#ifdef SD_ENABLED
static void APP_SDCARD_DetectCallBack(bool isInserted, void *userData)
{
    app_handle_t *app = (app_handle_t *)userData;

    app->sdcardInserted = isInserted;
    xSemaphoreGiveFromISR(app->sdcardSem, NULL);
}

bool SDCARD_inserted(void)
{
    return (app.sdcardInserted);
}

void APP_SDCARD_Task(void *param)
{
    const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};
    FRESULT error;
    app_handle_t *app = (app_handle_t *)param;

    app->sdcardSem = xSemaphoreCreateBinary();

    BOARD_SD_Config(&g_sd, APP_SDCARD_DetectCallBack, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY, app);

    PRINTF("[APP_SDCARD_Task] start\r\n");

    /* SD host init function */
    if (SD_HostInit(&g_sd) != kStatus_Success)
    {
        PRINTF("[APP_SDCARD_Task] SD host init failed.\r\n");
        vTaskSuspend(NULL);
    }

    /* Small delay for SD card detection logic to process */
    vTaskDelay(100 / portTICK_PERIOD_MS);

    while (1)
    {
        /* Block waiting for SDcard detect interrupt */
        xSemaphoreTake(app->sdcardSem, portMAX_DELAY);

        if (app->sdcardInserted != app->sdcardInsertedPrev)
        {
            app->sdcardInsertedPrev = app->sdcardInserted;

            if (app->sdcardInserted)
            {
                SD_SetCardPower(&g_sd, false);
                /* power on the card */
                SD_SetCardPower(&g_sd, true);
                if (f_mount(&app->fileSystem, driverNumberBuffer, 0U))
                {
                    PRINTF("[APP_SDCARD_Task] Mount volume failed.\r\n");
                    continue;
                }

#if (FF_FS_RPATH >= 2U)
                error = f_chdrive((char const *)&driverNumberBuffer[0U]);
                if (error)
                {
                    PRINTF("[APP_SDCARD_Task] Change drive failed.\r\n");
                    continue;
                }
#endif

                PRINTF("[APP_SDCARD_Task] SD card drive mounted\r\n");

            }
            else
            {
                PRINTF("[APP_SDCARD_Task] SD card removed\r\n");
            }
        }
    }
}
#endif

void APP_Shell_Task(void *param)
{
    PRINTF("[APP_Shell_Task] start\r\n");

    /* Handle shell commands. */
    shellCmd();
    vTaskSuspend(NULL);
    while (1)
        ;
}

int main(void)
{
    int ret;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO 12M to LPFLEXCOMM2 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

    /* attach FRO HF to SAI1 */
    CLOCK_SetClkDiv(kCLOCK_DivSai1Clk, 1u);
    CLOCK_AttachClk(kPLL1_CLK0_to_SAI1);

    BOARD_InitPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_SetupExtClocking(24000000U);
    /* Set up PLL1 */
    const pll_setup_t pll1Setup = {.pllctrl = SCG_SPLLCTRL_SOURCE(0U) | SCG_SPLLCTRL_SELI(15U) | SCG_SPLLCTRL_SELP(31U),
                                   .pllndiv = SCG_SPLLNDIV_NDIV(25U),
                                   .pllpdiv = SCG_SPLLPDIV_PDIV(2U),
                                   .pllmdiv = SCG_SPLLMDIV_MDIV(512U),
                                   .pllRate = 122880000U};
    /* Configure PLL1 to the 122.88MHz */
    CLOCK_SetPLL1Freq(&pll1Setup);

    /* Set PLL1CLK0DIV divider to value 10, PLL1_CLK0 = 12.288MHz */
    CLOCK_SetClkDiv(kCLOCK_DivPLL1Clk0, 10U);

    /* attach PLL1_CLK0 to PDM, PDM_CLK = 12.288MHz */
    CLOCK_SetClkDiv(kCLOCK_DivMicfilFClk, 1U);
    CLOCK_AttachClk(kPLL1_CLK0_to_MICFILF);

    PRINTF("\r\n");
    PRINTF("*******************************\r\n");
    PRINTF("Maestro audio record demo start\r\n");
    PRINTF("*******************************\r\n");
    PRINTF("\r\n");

    /* Initialize OSA*/
    OSA_Init();

    ret = BOARD_CODEC_Init();
    if (ret)
    {
        PRINTF("CODEC_Init failed\r\n");
        return -1;
    }

#ifdef SD_ENABLED
    if (xTaskCreate(APP_SDCARD_Task, "SDCard Task", SDCARD_TASK_STACK_SIZE, &app, configMAX_PRIORITIES - 4, NULL) !=
        pdPASS)
    {
        PRINTF("\r\nFailed to create application task\r\n");
        while (1)
            ;
    }
#endif

    /* Set shell command task priority = 1 */
    if (xTaskCreate(APP_Shell_Task, "Shell Task", APP_SHELL_TASK_STACK_SIZE, &app, configMAX_PRIORITIES - 5,
                    &app.shell_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create application task\r\n");
        while (1)
            ;
    }

    /* Run RTOS */
    vTaskStartScheduler();

    /* Should not reach this statement */
    return 0;
}

/**
 * @brief Loop forever if stack overflow is detected.
 *
 * If configCHECK_FOR_STACK_OVERFLOW is set to 1,
 * this hook provides a location for applications to
 * define a response to a stack overflow.
 *
 * Use this hook to help identify that a stack overflow
 * has occurred.
 *
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    portDISABLE_INTERRUPTS();

    /* Loop forever */
    for (;;)
        ;
}

/**
 * @brief Warn user if pvPortMalloc fails.
 *
 * Called if a call to pvPortMalloc() fails because there is insufficient
 * free memory available in the FreeRTOS heap.  pvPortMalloc() is called
 * internally by FreeRTOS API functions that create tasks, queues, software
 * timers, and semaphores.  The size of the FreeRTOS heap is set by the
 * configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
 *
 */
void vApplicationMallocFailedHook()
{
    PRINTF(("\r\nERROR: Malloc failed to allocate memory\r\n"));

    /* Loop forever */
    for (;;)
        ;
}
