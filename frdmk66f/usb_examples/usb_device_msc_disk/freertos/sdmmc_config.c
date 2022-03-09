/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sdmmc_config.h"
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

/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(SDIO_ENABLED) || defined(SD_ENABLED)
bool BOARD_SDCardGetDetectStatus(void)
{
    return GPIO_PinRead(BOARD_SDMMC_SD_CD_GPIO_BASE, BOARD_SDMMC_SD_CD_GPIO_PIN) == BOARD_SDMMC_SD_CD_INSERT_LEVEL;
}

void BOARD_SDMMC_SD_CD_PORT_IRQ_HANDLER(void)
{
    if (PORT_GetPinsInterruptFlags(BOARD_SDMMC_SD_CD_PORT_BASE) & (1U << BOARD_SDMMC_SD_CD_GPIO_PIN))
    {
        if (s_cd.callback != NULL)
        {
            s_cd.callback(BOARD_SDCardGetDetectStatus(), s_cd.userData);
        }
    }
    /* Clear interrupt flag.*/
    PORT_ClearPinsInterruptFlags(BOARD_SDMMC_SD_CD_PORT_BASE, ~0U);
}

void BOARD_SDCardDAT3PullFunction(uint32_t status)
{
    port_pin_config_t porte4_pinE3_config = {/* Internal pull-up resistor is enabled */
                                             kPORT_PullUp,
                                             /* Fast slew rate is configured */
                                             kPORT_FastSlewRate,
                                             /* Passive filter is disabled */
                                             kPORT_PassiveFilterDisable,
                                             /* Open drain is disabled */
                                             kPORT_OpenDrainDisable,
                                             /* Low drive strength is configured */
                                             kPORT_LowDriveStrength,
                                             /* Pin is configured as SDHC0_D3 */
                                             kPORT_MuxAlt4,
                                             /* Pin Control Register fields [15:0] are not locked */
                                             kPORT_UnlockRegister};
    if (status == kSD_DAT3PullDown)
    {
        porte4_pinE3_config.pullSelect = kPORT_PullDisable;
    }

    PORT_SetPinConfig(PORTE, 4U, &porte4_pinE3_config);
}

void BOARD_SDCardDetectInit(sd_cd_t cd, void *userData)
{
    /* install card detect callback */
    s_cd.cdDebounce_ms = BOARD_SDMMC_SD_CARD_DETECT_DEBOUNCE_DELAY_MS;
    s_cd.type          = BOARD_SDMMC_SD_CD_TYPE;
    s_cd.cardDetected  = BOARD_SDCardGetDetectStatus;
    s_cd.callback      = cd;
    s_cd.userData      = userData;

    if (BOARD_SDMMC_SD_CD_TYPE == kSD_DetectCardByGpioCD)
    {
        /* Card detection pin will generate interrupt on either eage */
        PORT_SetPinInterruptConfig(BOARD_SDMMC_SD_CD_PORT_BASE, BOARD_SDMMC_SD_CD_GPIO_PIN,
                                   BOARD_SDMMC_SD_CD_INTTERUPT_TYPE);
        /* set IRQ priority */
        NVIC_SetPriority(BOARD_SDMMC_SD_CD_PORT_IRQ, BOARD_SDMMC_SD_CD_IRQ_PRIORITY);
        /* Open card detection pin NVIC. */
        EnableIRQ(BOARD_SDMMC_SD_CD_PORT_IRQ);

        if (GPIO_PinRead(BOARD_SDMMC_SD_CD_GPIO_BASE, BOARD_SDMMC_SD_CD_GPIO_PIN) == BOARD_SDMMC_SD_CD_INSERT_LEVEL)
        {
            if (cd != NULL)
            {
                cd(true, userData);
            }
        }
    }

    /* register DAT3 pull function switch function pointer */
    if (BOARD_SDMMC_SD_CD_TYPE == kSD_DetectCardByHostDATA3)
    {
        s_cd.dat3PullFunc = BOARD_SDCardDAT3PullFunction;
    }
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
    ((sd_card_t *)card)->host->hostController.sourceClock_Hz = CLOCK_GetFreq(kCLOCK_CoreSysClk);

    ((sd_card_t *)card)->usrParam.cd = &s_cd;

    BOARD_SDCardDetectInit(cd, userData);

    NVIC_SetPriority(BOARD_SDMMC_SD_HOST_IRQ, hostIRQPriority);
}
#endif
