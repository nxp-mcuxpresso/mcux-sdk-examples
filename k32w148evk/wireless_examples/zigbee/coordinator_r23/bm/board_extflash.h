/* -------------------------------------------------------------------------- */
/*                           Copyright 2021-2022 NXP                          */
/*                            All rights reserved.                            */
/*                    SPDX-License-Identifier: BSD-3-Clause                   */
/* -------------------------------------------------------------------------- */

#ifndef _BOARD_EXTFLASH_H_
#define _BOARD_EXTFLASH_H_

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include <stdint.h>

#include "fsl_common.h"
#include "fsl_lpspi_mem_adapter.h"

/* -------------------------------------------------------------------------- */
/*                                Public macros                               */
/* -------------------------------------------------------------------------- */

/*! @brief The LPSPI channel used for the external NOR flash */
#define BOARD_EEPROM_LPSPI_BASEADDR  LPSPI1
#define BOARD_LPSPI_MRCC_ADDRESS     kCLOCK_Lpspi1
#define BOARD_LPSPI_CLKSRC           kCLOCK_IpSrcFro192M
#define BOARD_LPSPI_PCS_FOR_INIT     kLPSPI_Pcs0
#define BOARD_LPSPI_PCS_FOR_TRANSFER kLPSPI_MasterPcs0
#define BOARD_LPSPI_MRCC_CLK_DIV     0U
#define BOARD_LPSPI_NOR_BAUDRATE     32000000U

/* -------------------------------------------------------------------------- */
/*                              Public prototypes                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initialize external flash module
 *
 */
void BOARD_InitExternalFlash(void);

/*!
 * \brief Uninitialize external flash module
 *
 */
void BOARD_UninitExternalFlash(void);

/**
 * @brief Get the current LPSPI clock rate
 *
 * @return uint32_t LPSPI clock rate
 */
uint32_t BOARD_GetLpspiClock(void);

/**
 * @brief Get base LPSPI NOR flash base address
 *
 * @return LPSPI_Type* base address
 */
LPSPI_Type *BOARD_GetLpspiForNorFlash(void);

/**
 * @brief Controls LPSPI PCS pin output level
 *
 * @param isSelected
 */
void BOARD_LpspiPcsPinControl(bool isSelected);

/**
 * @brief Configures LPSPI flash pins
 *
 * @param pinMode
 */
void BOARD_LpspiIomuxConfig(spi_pin_mode_t pinMode);

/**
 * @brief Get NOR flash baudrate
 *
 * @return uint32_t baudrate
 */
uint32_t BOARD_GetNorFlashBaudrate(void);

/**
 * @brief Initialize external flash write protect pin
 *
 */
void BOARD_InitExternalFlashWriteProtect(void);

/*!
 * \brief Uninitialize external flash write protect pin
 *
 */
void BOARD_UninitExternalFlashWriteProtect(void);

/**
 * @brief Disable external flash write protect pin
 *
 */
void BOARD_DisableExternalFlashWriteProtect(void);

/**
 * @brief Enable external flash write protect pin
 *
 */
void BOARD_EnableExternalFlashWriteProtect(void);

#endif /* _BOARD_EXTFLASH_H_ */
