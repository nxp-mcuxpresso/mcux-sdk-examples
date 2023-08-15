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

#include "app_definitions.h"
#include "fsl_codec_common.h"
#include "fsl_wm8960.h"
#include "fsl_codec_adapter.h"
#include "fsl_dmamux.h"
#include "fsl_pdm.h"
#include "fsl_flexram.h"
#include "fsl_flexram_allocate.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
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
codec_handle_t codecHandle   = {0};
wm8960_config_t wm8960Config = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .route     = kWM8960_RoutePlaybackandRecord,
    .leftInputSource  = kWM8960_InputDifferentialMicInput3,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format           = {.mclk_HZ    = 24576000U,
               .sampleRate = kWM8960_AudioSampleRate16KHz,
               .bitWidth   = kWM8960_AudioBitWidth32bit},
    .master_slave     = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm8960Config};

/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (32 + 77/100)  / 2
 *                              = 393.24MHZ
 */
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 0, 1, 2, 3, 4, 5 */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};


static app_handle_t app;
/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_EnableSaiMclkOutput(bool enable)
{
    if (enable)
    {
        IOMUXC_GPR->GPR0 |= IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK;
    }
    else
    {
        IOMUXC_GPR->GPR0 &= (~IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK);
    }
}

int BOARD_CODEC_Init(void)
{
    CODEC_Init(&codecHandle, &boardCodecConfig);

    /* Initial volume kept low for hearing safety. */
    CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, DEMO_VOLUME);

    return 0;
}

void APP_FLEXRAM_IRQ_HANDLER(void)
{
    if (FLEXRAM_GetInterruptStatus(APP_FLEXRAM) & kFLEXRAM_OCRAMAccessError)
    {
        FLEXRAM_ClearInterruptStatus(APP_FLEXRAM, kFLEXRAM_OCRAMAccessError);
    }

    if (FLEXRAM_GetInterruptStatus(APP_FLEXRAM) & kFLEXRAM_DTCMAccessError)
    {
        FLEXRAM_ClearInterruptStatus(APP_FLEXRAM, kFLEXRAM_DTCMAccessError);
    }

    if (FLEXRAM_GetInterruptStatus(APP_FLEXRAM) & kFLEXRAM_ITCMAccessError)
    {
        FLEXRAM_ClearInterruptStatus(APP_FLEXRAM, kFLEXRAM_ITCMAccessError);
    }

    __DSB();
}

void SystemInitHook(void)
{
    IOMUXC_GPR->GPR17 = (kFLEXRAM_BankDTCM << 0) | (kFLEXRAM_BankOCRAM << 2) | (kFLEXRAM_BankOCRAM << 4) |
                        (kFLEXRAM_BankOCRAM << 6) | (kFLEXRAM_BankOCRAM << 8) | (kFLEXRAM_BankOCRAM << 10) |
                        (kFLEXRAM_BankOCRAM << 12) | (kFLEXRAM_BankOCRAM << 14);

    IOMUXC_GPR->GPR18 = (kFLEXRAM_BankOCRAM << 0) | (kFLEXRAM_BankOCRAM << 2) | (kFLEXRAM_BankOCRAM << 4) |
                        (kFLEXRAM_BankOCRAM << 6) | (kFLEXRAM_BankOCRAM << 8) | (kFLEXRAM_BankOCRAM << 10) |
                        (kFLEXRAM_BankOCRAM << 12) | (kFLEXRAM_BankOCRAM << 14);

    IOMUXC_GPR->GPR16 |= IOMUXC_GPR_GPR16_FLEXRAM_BANK_CFG_SEL_MASK;

#if (defined __GNUC__ && !defined __MCUXPRESSO)
    extern uint32_t __bss_ocram_start__;
    extern uint32_t __bss_ocram_end__;

    /* Initialization of OCRAM BSS section. */
    uint32_t startAddr; /* Address of the source memory. */
    uint32_t endAddr;   /* End of copied memory. */

    /* Get the addresses for the OCRAM BSS section (zero-initialized data). */
    startAddr = (uint32_t)&__bss_ocram_start__;
    endAddr   = (uint32_t)&__bss_ocram_end__;

    /* Reset the .bss section. */
    while (startAddr < endAddr)
    {
        /* Clear one byte. */
        *((uint8_t *)startAddr) = 0U;

        /* Increment the pointer. */
        startAddr++;
    }
#endif
}

void update_MPU_config(void)
{
    /* Disable I cache and D cache */
    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
    {
        SCB_DisableICache();
    }
    if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR))
    {
        SCB_DisableDCache();
    }
    /* Disable MPU */
    ARM_MPU_Disable();

    /* Region 7 setting: Memory with Normal type, not shareable, outer/inner write back */
    MPU->RBAR = ARM_MPU_RBAR(7, 0x20300000U);
    MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_1MB);

    /* Enable MPU */
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);

    /* Enable I cache and D cache */
    SCB_EnableDCache();
    SCB_EnableICache();
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

    BOARD_ConfigMPU();
    update_MPU_config();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    EnableIRQ(GPIO13_Combined_0_31_IRQn);

    CLOCK_InitAudioPll(&audioPllConfig);

    FLEXRAM_Init(FLEXRAM);

    /* enable FlexRAM IRQ */
    EnableIRQ(APP_FLEXRAM_IRQ);

    /*Clock setting for LPI2C*/
    CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, 1);

    /*Clock setting for SAI1*/
    CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, 4);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, 16);

    /* 0SC400M */
    /* 24.576m mic root clock */
    CLOCK_SetRootClockMux(kCLOCK_Root_Mic, 6);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Mic, 16);

    /*Enable MCLK clock*/
    BOARD_EnableSaiMclkOutput(true);

    /* Init DMAMUX */
    DMAMUX_Init(DEMO_DMAMUX);
    DMAMUX_SetSource(DEMO_DMAMUX, DEMO_TX_CHANNEL, (uint8_t)DEMO_SAI_TX_SOURCE);
    DMAMUX_EnableChannel(DEMO_DMAMUX, DEMO_TX_CHANNEL);
    DMAMUX_SetSource(DEMO_DMAMUX, DEMO_RX_CHANNEL, (uint8_t)DEMO_PDM_RX_SOURCE);
    DMAMUX_EnableChannel(DEMO_DMAMUX, DEMO_RX_CHANNEL);

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
