/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Board includes */
#include "pin_mux.h"
#include "board.h"
#include "main.h"
#include "cmd.h"
#include "app_streamer.h"

#include "fsl_sd_disk.h"
#include "sdmmc_config.h"

#include "fsl_debug_console.h"

#include <stdbool.h>
#include "fsl_sysctl.h"
#include "fsl_codec_common.h"
#include "fsl_wm8904.h"
#include "fsl_codec_adapter.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#include "app_definitions.h"
#define APP_SHELL_TASK_STACK_SIZE (512)
#define SDCARD_TASK_STACK_SIZE    (512)
#define APP_TASK_STACK_SIZE       (512)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int BOARD_CODEC_Init(void);
static void APP_SDCARD_DetectCallBack(bool isInserted, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
codec_handle_t codecHandle   = {0};
wm8904_config_t wm8904Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .recordSource = kWM8904_RecordSourceLineInput,
    .recordChannelLeft  = kWM8904_RecordChannelLeft2,
    .recordChannelRight = kWM8904_RecordChannelRight2,
    .playSource         = kWM8904_PlaySourceDAC,
    .slaveAddress       = WM8904_I2C_ADDRESS,
    .protocol           = kWM8904_ProtocolI2S,
    .format             = {.sampleRate = kWM8904_SampleRate48kHz, .bitWidth = kWM8904_BitWidth16},
    .mclk_HZ            = DEMO_I2S_MASTER_CLOCK_FREQUENCY,
    .master             = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8904, .codecDevConfig = &wm8904Config};

app_handle_t app;
/*******************************************************************************
 * Code
 ******************************************************************************/

int BOARD_CODEC_Init(void)
{
    CODEC_Init(&codecHandle, &boardCodecConfig);

    /* Invert the DAC data in order to output signal with correct polarity - set DACL_DATINV and DACR_DATINV = 1 */
    WM8904_WriteRegister((wm8904_handle_t *)codecHandle.codecDevHandle, WM8904_AUDIO_IF_0, 0x1850);

    /* Initial volume kept low for hearing safety. */
    CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, 75);

    return 0;
}

void BOARD_InitSysctrl(void)
{
    SYSCTL_Init(SYSCTL);
    /* select signal source for share set */
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet0, kSYSCTL_SharedCtrlSignalSCK, kSYSCTL_Flexcomm7);
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet0, kSYSCTL_SharedCtrlSignalWS, kSYSCTL_Flexcomm7);
    /* select share set for special flexcomm signal */
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm7, kSYSCTL_FlexcommSignalSCK, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm7, kSYSCTL_FlexcommSignalWS, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm6, kSYSCTL_FlexcommSignalSCK, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm6, kSYSCTL_FlexcommSignalWS, kSYSCTL_ShareSet0);
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

app_data_t app_data = {.lastXOOperatingMode = 0, .lastPreset = 0, .logEnabled = 0, .eap_args = {.preset_num = 0}};

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
                (dot && strncmp(dot + 1, "mp3", 3) == 0)
#endif
            )
            {
                if (count < MAX_FILES_LIST)
                {
                    if (strlen(fileInformation.fname) < MAX_FILE_NAME_LENGTH)
                    {
                        strncpy(get_eap_att_control()->availableInputs[count], fileInformation.fname,
                                MAX_FILE_NAME_LENGTH - 1);
                        PRINTF("  %s\r\n", get_eap_att_control()->availableInputs[count]);
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
        if (strlen(get_eap_att_control()->availableInputs[0]) > 0)
        {
            strcpy(get_eap_att_control()->input, get_eap_att_control()->availableInputs[0]);
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

            SD_SetCardPower(&g_sd, false);

            if (app->sdcardInserted)
            {
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

                xSemaphoreGive(app->sdcardSem);

                (void)list_files(true);
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

void APP_main_Task(void *param)
{
    PRINTF("[APP_Main_Task] started\r\n");

    STREAMER_Init();

    while (1)
    {
        eap_att_process();
        vTaskDelay(1);
    }
}

int main(void)
{
    int ret;

    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);

    /* USART0 clock */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* I2C clock */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM4);

    PMC->PDRUNCFGCLR0 |= PMC_PDRUNCFG0_PDEN_XTAL32M_MASK;   /*!< Ensure XTAL16M is on  */
    PMC->PDRUNCFGCLR0 |= PMC_PDRUNCFG0_PDEN_LDOXO32M_MASK;  /*!< Ensure XTAL16M is on  */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK; /*!< Ensure CLK_IN is on  */
    ANACTRL->XO32M_CTRL |= ANACTRL_XO32M_CTRL_ENABLE_SYSTEM_CLK_OUT_MASK;

    /*!< Switch PLL0 clock source selector to XTAL16M */
    CLOCK_AttachClk(kEXT_CLK_to_PLL0);

    const pll_setup_t pll0Setup = {
        .pllctrl = SYSCON_PLL0CTRL_CLKEN_MASK | SYSCON_PLL0CTRL_SELI(2U) | SYSCON_PLL0CTRL_SELP(31U),
        .pllndec = SYSCON_PLL0NDEC_NDIV(125U),
        .pllpdec = SYSCON_PLL0PDEC_PDIV(8U),
        .pllsscg = {0x0U, (SYSCON_PLL0SSCG1_MDIV_EXT(3072U) | SYSCON_PLL0SSCG1_SEL_EXT_MASK)},
        .pllRate = 24576000U,
        .flags   = PLL_SETUPFLAG_WAITLOCK};
    /*!< Configure PLL to the desired values */
    CLOCK_SetPLL0Freq(&pll0Setup);

    CLOCK_SetClkDiv(kCLOCK_DivPll0Clk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivPll0Clk, 1U, false);

    /* I2S clocks */
    CLOCK_AttachClk(kPLL0_DIV_to_FLEXCOMM6);
    CLOCK_AttachClk(kPLL0_DIV_to_FLEXCOMM7);

    /* Attach PLL clock to MCLK for I2S, no divider */
    CLOCK_AttachClk(kPLL0_to_MCLK);
    SYSCON->MCLKDIV = SYSCON_MCLKDIV_DIV(0U);
    SYSCON->MCLKIO  = 1U;

    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(kFC4_RST_SHIFT_RSTn);

    /* reset FLEXCOMM for DMA0 */
    RESET_PeripheralReset(kDMA0_RST_SHIFT_RSTn);

    /* reset FLEXCOMM for I2S */
    RESET_PeripheralReset(kFC6_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);

    /* reset NVIC for FLEXCOMM6 and FLEXCOMM7 */
    NVIC_ClearPendingIRQ(FLEXCOMM6_IRQn);
    NVIC_ClearPendingIRQ(FLEXCOMM7_IRQn);

    /* Enable interrupts for I2S */
    EnableIRQ(FLEXCOMM6_IRQn);
    EnableIRQ(FLEXCOMM7_IRQn);

    /* Initialize the rest */
    BOARD_InitPins();
    BOARD_BootClockPLL1_150M();
    BOARD_InitDebugConsole();
    BOARD_InitSysctrl();

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

    if (xTaskCreate(APP_main_Task, "APP task", APP_TASK_STACK_SIZE, &app, configMAX_PRIORITIES - 1,
                    &app.app_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create Application task. Please, fix issue and restart board.\r\n");
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
