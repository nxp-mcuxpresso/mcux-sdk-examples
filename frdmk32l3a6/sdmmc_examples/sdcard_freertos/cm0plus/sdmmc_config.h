/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SDMMC_CONFIG_H_
#define _SDMMC_CONFIG_H_

#include "fsl_common.h"

#ifdef SD_ENABLED
#include "fsl_sd.h"
#endif
#ifdef MMC_ENABLED
#include "fsl_mmc.h"
#endif
#include "clock_config.h"
#include "fsl_adapter_gpio.h"
#include "fsl_sdmmc_host.h"
#include "fsl_sdmmc_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* @brief host basic configuration */
#define BOARD_SDMMC_SD_HOST_BASEADDR USDHC0

#define BOARD_SDMMC_HOST_IRQ    USDHC0_IRQn
#define BOARD_SDMMC_SD_HOST_IRQ BOARD_SDMMC_HOST_IRQ
/* @brief card detect configuration */
#define BOARD_SDMMC_SD_CD_GPIO_BASE      GPIOC
#define BOARD_SDMMC_SD_CD_GPIO_PORT      2
#define BOARD_SDMMC_SD_CD_GPIO_PIN       27U
#define BOARD_SDMMC_SD_CD_INTTERUPT_TYPE kHAL_GpioInterruptEitherEdge
#define BOARD_SDMMC_SD_CD_INSERT_LEVEL   (1U)

#define BOARD_SDMMC_SD_CD_TYPE                       kSD_DetectCardByGpioCD
#define BOARD_SDMMC_SD_CARD_DETECT_DEBOUNCE_DELAY_MS (100U)

#define BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE (4U)
/* @brief mmc configuration */
#define BOARD_SDMMC_HOST_VCC_SUPPLY kMMC_VoltageWindows270to360

/*!@ brief host interrupt priority*/
#define BOARD_SDMMC_SD_HOST_IRQ_PRIORITY  (5U)
#define BOARD_SDMMC_MMC_HOST_IRQ_PRIORITY (5U)
/*!@brief dma descriptor buffer size */
#define BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE (32U)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief BOARD SD configurations.
 * @param card card descriptor
 * @param cd card detect callback
 * @param userData user data for callback
 */
#ifdef SD_ENABLED
void BOARD_SD_Config(void *card, sd_cd_t cd, uint32_t hostIRQPriority, void *userData);
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
