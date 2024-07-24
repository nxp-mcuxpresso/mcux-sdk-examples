/*
 * Copyright 2020-2023 NXP
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
#include "app_streamer.h"

#include "fsl_sd_disk.h"
#include "sdmmc_config.h"

#include "fsl_debug_console.h"

#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_dialog7212.h"
#include "fsl_sai.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#include "app_definitions.h"
#define APP_SHELL_TASK_STACK_SIZE (1024)
#define SDCARD_TASK_STACK_SIZE    (512)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int BOARD_CODEC_Init(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
codec_handle_t codecHandle    = {0};
da7212_pll_config_t pllConfig = {
    .source = kDA7212_PLLClkSourceMCLK,
#ifdef SUPPORT_48KHZ
    .refClock_HZ    = 12288000U,
    .outputClock_HZ = kDA7212_PLLOutputClk12288000,
#else
    .refClock_HZ    = 11289600U,
    .outputClock_HZ = kDA7212_PLLOutputClk11289600,
#endif
};
da7212_config_t da7212Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = 12000000},
    .dacSource    = kDA7212_DACSourceInputStream,
    .slaveAddress = DA7212_ADDRESS,
    .protocol     = kDA7212_BusI2S,
#ifdef SUPPORT_48KHZ
    .format = {.mclk_HZ = 12288000U, .sampleRate = 48000U, .bitWidth = 16U},
#else
    .format         = {.mclk_HZ = 11289600U, .sampleRate = 44100U, .bitWidth = 16U},
#endif
    .pll          = &pllConfig,
    .sysClkSource = kDA7212_SysClkSourcePLL,
    .isMaster     = true,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_DA7212, .codecDevConfig = &da7212Config};

sai_master_clock_t mclkConfig;


app_handle_t app;
/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_MASTER_CLOCK_CONFIG(void);

int BOARD_CODEC_Init(void)
{
    CODEC_Init(&codecHandle, &boardCodecConfig);

    /* Initial volume kept low for hearing safety. */
    CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, 50);

    return 0;
}


void BOARD_MASTER_CLOCK_CONFIG(void)
{
#ifdef SUPPORT_48KHZ
    mclkConfig.mclkOutputEnable = true, mclkConfig.mclkHz = 12288000U;
    mclkConfig.mclkSourceClkHz = 24576000U;
#else
    mclkConfig.mclkOutputEnable = true, mclkConfig.mclkHz = 11289600U;
    mclkConfig.mclkSourceClkHz = 22579200U;
#endif
    SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
}

static void APP_SDCARD_DetectCallBack(bool isInserted, void *userData)
{
    app_handle_t *app = (app_handle_t *)userData;

    app->sdcardInserted = isInserted;
    xSemaphoreGiveFromISR(app->sdcardSem, NULL);
}

void handleShellMessage(void *arg)
{
    /* Wait for response message to be processed before returning to shell. */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

app_data_t app_data = {.logEnabled      = 0,
                       .status          = kAppIdle,
                       .lastError       = kAppCodeOk,
                       .trackTotal      = 0,
                       .trackCurrent    = 0,
                       .input           = "",
                       .availableInputs = {{0}},
                       .volume          = 75,
                       .seek_time       = 0};

app_data_t *get_app_data()
{
    return &app_data;
}

status_t list_files(bool autoInput)
{
    FRESULT error;
    DIR directory           = {0};
    FILINFO fileInformation = {0};
    char *dot               = NULL;
    uint32_t count          = 0;

    if (!app.sdcardInserted)
    {
        PRINTF("Please insert an SD card with audio files and retry this command\r\n");
        return kStatus_Fail;
    }

    error = f_opendir(&directory, "/");
    if (error)
    {
        PRINTF("Failed to open root directory of SD card\r\n");
        return kStatus_Fail;
    }

    PRINTF("Available audio files:\r\n");

    while (1)
    {
        error = f_readdir(&directory, &fileInformation);

        /* When dir end or error detected, break out */
        if ((error != FR_OK) || (fileInformation.fname[0U] == 0U))
        {
            break;
        }
        /* Skip root directory */
        if (fileInformation.fname[0] == '.')
        {
            continue;
        }
        if (!(fileInformation.fattrib & AM_DIR))
        {
            /* Check file for supported audio extension */
            dot = strrchr(fileInformation.fname, '.');
            if (
#ifdef MULTICHANNEL_EXAMPLE
                (dot && strncmp(dot + 1, "pcm", 4) == 0)
#else
#if (OGG_OPUS_DEC == 1)
                (dot && strncmp(dot + 1, "opus", 4) == 0) || (dot && strncmp(dot + 1, "ogg", 3) == 0) ||
#endif
#if (AAC_DEC == 1)
                (dot && strncmp(dot + 1, "aac", 3) == 0) ||
#endif
#if (WAV_DEC == 1)
                (dot && strncmp(dot + 1, "wav", 3) == 0) ||
#endif
#if (FLAC_DEC == 1)
                (dot && strncmp(dot + 1, "flac", 3) == 0) ||
#endif
#if (MP3_DEC == 1)
                (dot && strncmp(dot + 1, "mp3", 3) == 0)
#endif
#endif
            )
            {
                if (count < MAX_FILES_LIST)
                {
                    if (strlen(fileInformation.fname) < MAX_FILE_NAME_LENGTH)
                    {
                        strncpy(get_app_data()->availableInputs[count], fileInformation.fname,
                                MAX_FILE_NAME_LENGTH - 1);
                        PRINTF("  %s\r\n", get_app_data()->availableInputs[count]);
                        count++;
                    }
                }
                else
                {
                    PRINTF("File globing reaches files limit (%d)", count);
                    break;
                }
            }
        }
    }

    if (error == FR_OK)
    {
        f_closedir(&directory);
    }

    if (autoInput == true)
    {
        if (strlen(get_app_data()->availableInputs[0]) > 0)
        {
            strcpy(get_app_data()->input, get_app_data()->availableInputs[0]);
        }
    }

    return kStatus_Success;
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

    BOARD_InitBootPins();
    BOARD_BootClockFROHF144M();
    BOARD_InitDebugConsole();

    /*!< Set the system clock to 144MHz */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U); /*!< Set AHBCLKDIV divider to value 1 */

    CLOCK_EnableClock(kCLOCK_InputMux);

    /* attach FRO 12M to LPFLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO 12M to LPFLEXCOMM2 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

    /* attach FRO HF to USDHC */
    CLOCK_SetClkDiv(kCLOCK_DivUSdhcClk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_USDHC);

    /* Enables the clock for GPIO0 */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    /* Enables the clock for GPIO2 */
    CLOCK_EnableClock(kCLOCK_Gpio2);

    CLOCK_SetupExtClocking(24000000U);
    CLOCK_SetSysOscMonitorMode(kSCG_SysOscMonitorDisable); /* System OSC Clock Monitor is disabled */
#ifdef SUPPORT_48KHZ
    /*!< Set up PLL0 */
    const pll_setup_t pll0Setup = {
        .pllctrl = SCG_APLLCTRL_SOURCE(0U) | SCG_APLLCTRL_LIMUPOFF_MASK | SCG_APLLCTRL_SELI(4U) |
                   SCG_APLLCTRL_SELP(3U) | SCG_APLLCTRL_SELR(4U),
        .pllndiv = SCG_APLLNDIV_NDIV(6U),
        .pllpdiv = SCG_APLLPDIV_PDIV(7U),
        .pllsscg = {(SCG_APLLSSCG0_SS_MDIV_LSB(2886218023U)),
                    ((SCG0->APLLSSCG1 & ~SCG_APLLSSCG1_SS_PD_MASK) | SCG_APLLSSCG1_SS_MDIV_MSB(0U) |
                     (uint32_t)(kSS_MF_512) | (uint32_t)(kSS_MR_K0) | (uint32_t)(kSS_MC_NOC) |
                     SCG_APLLSSCG1_SEL_SS_MDIV_MASK)},
        .pllRate = 24576000U};
#else
    /*!< Set up PLL0 */
    const pll_setup_t pll0Setup = {
                .pllctrl = SCG_APLLCTRL_SOURCE(0U) | SCG_APLLCTRL_LIMUPOFF_MASK | SCG_APLLCTRL_SELI(4U) |
                   SCG_APLLCTRL_SELP(3U) | SCG_APLLCTRL_SELR(4U),
                .pllndiv = SCG_APLLNDIV_NDIV(15U),
                .pllpdiv = SCG_APLLPDIV_PDIV(9U),
                .pllsscg = {(SCG_APLLSSCG0_SS_MDIV_LSB(4228395303U)),
                            ((SCG0->APLLSSCG1 & ~SCG_APLLSSCG1_SS_PD_MASK) | SCG_APLLSSCG1_SS_MDIV_MSB(1U) |
                     (uint32_t)(kSS_MF_512) | (uint32_t)(kSS_MR_K0) | (uint32_t)(kSS_MC_NOC) |
                     SCG_APLLSSCG1_SEL_SS_MDIV_MASK)},
                .pllRate = 22579200U};
#endif
    CLOCK_SetPLL0Freq(&pll0Setup); /*!< Configure PLL0 to the desired values */

    /* force the APLL power on and gate on */
    SCG0->APLL_OVRD |= SCG_APLL_OVRD_APLLPWREN_OVRD_MASK | SCG_APLL_OVRD_APLLCLKEN_OVRD_MASK;
    SCG0->APLL_OVRD |= SCG_APLL_OVRD_APLL_OVRD_EN_MASK;
    CLOCK_SetClkDiv(kCLOCK_DivPllClk, 1U);

    /* attach PLL0 to SAI1 */
    CLOCK_SetClkDiv(kCLOCK_DivSai1Clk, 1u);
    CLOCK_AttachClk(kPLL0_to_SAI1);

    /* Initialize OSA */
    OSA_Init();

    PRINTF("\r\n");
    PRINTF("*********************************\r\n");
    PRINTF("Maestro audio playback demo start\r\n");
    PRINTF("*********************************\r\n");
    PRINTF("\r\n");

    ret = BOARD_CODEC_Init();
    if (ret)
    {
        PRINTF("CODEC_Init failed\r\n");
        return -1;
    }

    if (xTaskCreate(APP_SDCARD_Task, "SDCard Task", SDCARD_TASK_STACK_SIZE, &app, configMAX_PRIORITIES - 2,
                    &app.sdcard_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create SDCard task. Please, fix issue and restart board.\r\n");
        while (1)
            ;
    }

    if (xTaskCreate(APP_Shell_Task, "Shell Task", APP_SHELL_TASK_STACK_SIZE, &app, configMAX_PRIORITIES - 4,
                    &app.shell_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create Shell observer task. Please, fix issue and restart board.\r\n");
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
}
