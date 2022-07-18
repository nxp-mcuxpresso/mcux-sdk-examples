/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sdmmc_config.h"
#if defined CPU_K32L3A60VPJ1A_cm0plus
#include "fsl_intmux.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
/*!brief sdmmc dma buffer */
SDK_ALIGN(static uint32_t s_sdmmcHostDmaBuffer[BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE],
          SDMMCHOST_DMA_DESCRIPTOR_BUFFER_ALIGN_SIZE);
#if defined(SDIO_ENABLED) || defined(SD_ENABLED)
static sd_detect_card_t s_cd;
#endif
static sdmmchost_t s_host;

const scg_lpfll_config_t g_appScgLpFllConfig_BOARD_BootClockRUN = {
    .enableMode = kSCG_LpFllEnable,     /* LPFLL clock disabled */
    .div1       = kSCG_AsyncClkDivBy1,  /* Low Power FLL Clock Divider 1: Clock output is disabled */
    .div2       = kSCG_AsyncClkDisable, /* Low Power FLL Clock Divider 2: Clock output is disabled */
    .div3       = kSCG_AsyncClkDisable, /* Low Power FLL Clock Divider 3: Clock output is disabled */
    .range      = kSCG_LpFllRange72M,   /* LPFLL is trimmed to 72MHz */
    .trimConfig = NULL,
};

GPIO_HANDLE_DEFINE(s_CardDetectGpioHandle);
/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(SDIO_ENABLED) || defined(SD_ENABLED)
bool BOARD_SDCardGetDetectStatus(void)
{
    uint8_t pinState;

    if (HAL_GpioGetInput(s_CardDetectGpioHandle, &pinState) == kStatus_HAL_GpioSuccess)
    {
        if (pinState == BOARD_SDMMC_SD_CD_INSERT_LEVEL)
        {
            return true;
        }
    }

    return false;
}

void SDMMC_SD_CD_Callback(void *param)
{
    if (s_cd.callback != NULL)
    {
        s_cd.callback(BOARD_SDCardGetDetectStatus(), s_cd.userData);
    }
}

void BOARD_SDCardDetectInit(sd_cd_t cd, void *userData)
{
    uint8_t pinState;

    /* install card detect callback */
    s_cd.cdDebounce_ms = BOARD_SDMMC_SD_CARD_DETECT_DEBOUNCE_DELAY_MS;
    s_cd.type          = BOARD_SDMMC_SD_CD_TYPE;
    s_cd.cardDetected  = BOARD_SDCardGetDetectStatus;
    s_cd.callback      = cd;
    s_cd.userData      = userData;

    hal_gpio_pin_config_t sw_config = {
        kHAL_GpioDirectionIn,
        0,
        BOARD_SDMMC_SD_CD_GPIO_PORT,
        BOARD_SDMMC_SD_CD_GPIO_PIN,
    };
    HAL_GpioInit(s_CardDetectGpioHandle, &sw_config);
    HAL_GpioSetTriggerMode(s_CardDetectGpioHandle, BOARD_SDMMC_SD_CD_INTTERUPT_TYPE);
    HAL_GpioInstallCallback(s_CardDetectGpioHandle, SDMMC_SD_CD_Callback, NULL);

    if (HAL_GpioGetInput(s_CardDetectGpioHandle, &pinState) == kStatus_HAL_GpioSuccess)
    {
        if (pinState == BOARD_SDMMC_SD_CD_INSERT_LEVEL)
        {
            if (cd != NULL)
            {
                cd(true, userData);
            }
        }
    }
}
#endif

uint32_t BOARD_SDHC_ClockConfiguration(void)
{
    /* Init LPFLL */
    CLOCK_InitLpFll(&g_appScgLpFllConfig_BOARD_BootClockRUN);
    /* set SDHC0 clock source */
    CLOCK_SetIpSrc(kCLOCK_Sdhc0, kCLOCK_IpSrcLpFllAsync);

    return CLOCK_GetIpFreq(kCLOCK_Sdhc0);
}

#ifdef SD_ENABLED
void BOARD_SD_Config(void *card, sd_cd_t cd, uint32_t hostIRQPriority, void *userData)
{
    assert(card);

    s_host.dmaDesBuffer                                      = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum                              = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
    ((sd_card_t *)card)->host                                = &s_host;
    ((sd_card_t *)card)->host->hostController.base           = BOARD_SDMMC_SD_HOST_BASEADDR;
    ((sd_card_t *)card)->host->hostController.sourceClock_Hz = BOARD_SDHC_ClockConfiguration();

    ((sd_card_t *)card)->usrParam.cd = &s_cd;

#if defined CPU_K32L3A60VPJ1A_cm0plus
    /* config INTMUX for USDHC/PORTC */
    INTMUX_Init(INTMUX1);
    INTMUX_EnableInterrupt(INTMUX1, 0, BOARD_SDMMC_SD_HOST_IRQ);
#endif

    /* config INTMUX for USDHC/PORTC */
    BOARD_SDCardDetectInit(cd, userData);

#if __CORTEX_M == 4
    NVIC_SetPriority(BOARD_SDMMC_SD_HOST_IRQ, hostIRQPriority);
#endif
}
#endif
