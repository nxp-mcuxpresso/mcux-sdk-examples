/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sdmmc_config.h"
#include "fsl_iocon.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
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
#ifdef SDIO_ENABLED
static sdio_card_int_t s_sdioInt;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
uint32_t BOARD_SDIF0ClockConfiguration(void)
{
    /* attach main clock to SDIF */
    CLOCK_AttachClk(kMAIN_CLK_to_SDIO_CLK);
    /* need call this function to clear the halt bit in clock divider register */
    CLOCK_SetClkDiv(kCLOCK_DivSdioClk, (uint32_t)(SystemCoreClock / FSL_FEATURE_SDIF_MAX_SOURCE_CLOCK + 1U), true);

    return CLOCK_GetSdioClkFreq();
}

#if defined(SDIO_ENABLED) || defined(SD_ENABLED)
void BOARD_SDCardDetectInit(sd_cd_t cd, void *userData)
{
    /* install card detect callback */
    s_cd.cdDebounce_ms = BOARD_SDMMC_SD_CARD_DETECT_DEBOUNCE_DELAY_MS;
    s_cd.type          = BOARD_SDMMC_SD_CD_TYPE;
    s_cd.callback      = cd;
    s_cd.userData      = userData;
}
#endif

#ifdef SD_ENABLED
void BOARD_SD_Config(void *card, sd_cd_t cd, uint32_t hostIRQPriority, void *userData)
{
    assert(card);

    s_host.dmaDesBuffer                                      = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum                              = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
    ((sd_card_t *)card)->host                                = &s_host;
    ((sd_card_t *)card)->host->hostController.base           = BOARD_SDMMC_SD_HOST_BASEADDR;
    ((sd_card_t *)card)->host->hostController.sourceClock_Hz = BOARD_SDIF0ClockConfiguration();

    ((sd_card_t *)card)->usrParam.cd = &s_cd;

    BOARD_SDCardDetectInit(cd, userData);

    NVIC_SetPriority(BOARD_SDMMC_SD_HOST_IRQ, hostIRQPriority);
}
#endif

#ifdef SDIO_ENABLED
void BOARD_SDIO_Config(void *card, sd_cd_t cd, uint32_t hostIRQPriority, sdio_int_t cardInt)
{
    assert(card);

    s_host.dmaDesBuffer                                        = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum                                = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
    ((sdio_card_t *)card)->host                                = &s_host;
    ((sdio_card_t *)card)->host->hostController.base           = BOARD_SDMMC_SDIO_HOST_BASEADDR;
    ((sdio_card_t *)card)->host->hostController.sourceClock_Hz = BOARD_SDIF0ClockConfiguration();

    ((sdio_card_t *)card)->usrParam.cd = &s_cd;
    if (cardInt != NULL)
    {
        s_sdioInt.cardInterrupt                 = cardInt;
        ((sdio_card_t *)card)->usrParam.sdioInt = &s_sdioInt;
    }

    BOARD_SDCardDetectInit(cd, NULL);

    NVIC_SetPriority(BOARD_SDMMC_SDIO_HOST_IRQ, hostIRQPriority);
}
#endif

#ifdef MMC_ENABLED
void BOARD_MMC_Config(void *card, uint32_t hostIRQPriority)

{
    assert(card);

    s_host.dmaDesBuffer                                       = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum                               = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
    ((mmc_card_t *)card)->host                                = &s_host;
    ((mmc_card_t *)card)->host->hostController.base           = BOARD_SDMMC_MMC_HOST_BASEADDR;
    ((mmc_card_t *)card)->host->hostController.sourceClock_Hz = BOARD_SDIF0ClockConfiguration();

    ((mmc_card_t *)card)->hostVoltageWindowVCC  = BOARD_SDMMC_MMC_VCC_SUPPLY;
    ((mmc_card_t *)card)->hostVoltageWindowVCCQ = BOARD_SDMMC_MMC_VCCQ_SUPPLY;

    NVIC_SetPriority(BOARD_SDMMC_MMC_HOST_IRQ, hostIRQPriority);
}
#endif
