/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define EXAMPLE_FLEXSPI                 FLEXSPI
#define EXAMPLE_CACHE                   CACHE64
#define FLASH_SIZE                      0x10000 /* 512Mb/KByte */
#define EXAMPLE_FLEXSPI_AMBA_BASE       FlexSPI_AMBA_BASE
#define FLASH_PAGE_SIZE                 256
#define EXAMPLE_SECTOR                  1000
#define SECTOR_SIZE                     0x1000 /* 4K */
#define FLASH_PORT                      kFLEXSPI_PortB1
#define EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK kFLEXSPI_ReadSampleClkLoopbackInternally
#define XIP_EXTERNAL_FLASH              (1) /* Set this macro to skip chip erase. */

#define NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD     0
#define NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG     1
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE        2
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR        3
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD   4
#define NOR_CMD_LUT_SEQ_IDX_ERASECHIP          5
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE 6
#define NOR_CMD_LUT_SEQ_IDX_READ_NORMAL        7
#define NOR_CMD_LUT_SEQ_IDX_READID             8
/* NOTE: Workaround for debugger.
   Must define AHB write FlexSPI sequence index to 9 to avoid debugger issue.
   Debugger can attach to the CM33 core only when ROM executes to certain place.
   At that point, AHB write FlexSPI sequence index is set to 9, but in LUT, the
   command is not filled by ROM. If the debugger sets software breakpoint at flash
   after reset/attachment, FlexSPI AHB write command will be triggered. It may
   cause AHB bus hang if the command in LUT sequence index 9 is any read opeartion.
   So we need to ensure at any time, the FlexSPI LUT sequence 9 for the flash must
   be set to STOP command to avoid unexpected debugger behaivor.
 */
#define NOR_CMD_LUT_SEQ_IDX_WRITE         9
#define NOR_CMD_LUT_SEQ_IDX_ENTERQPI      10
#define NOR_CMD_LUT_SEQ_IDX_EXITQPI       11
#define NOR_CMD_LUT_SEQ_IDX_READSTATUSREG 12
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST     13

#define CUSTOM_LUT_LENGTH        60
#define FLASH_QUAD_ENABLE        0x40
#define FLASH_BUSY_STATUS_POL    1
#define FLASH_BUSY_STATUS_OFFSET 0
#define CACHE_MAINTAIN           1

#define EXAMPLE_INVALIDATE_FLEXSPI_CACHE()                                                                    \
    do                                                                                                        \
    {                                                                                                         \
        CACHE64->CCR |= CACHE64_CTRL_CCR_INVW0_MASK | CACHE64_CTRL_CCR_INVW1_MASK | CACHE64_CTRL_CCR_GO_MASK; \
        while (CACHE64->CCR & CACHE64_CTRL_CCR_GO_MASK)                                                       \
        {                                                                                                     \
        }                                                                                                     \
        CACHE64->CCR &= ~(CACHE64_CTRL_CCR_INVW0_MASK | CACHE64_CTRL_CCR_INVW1_MASK);                         \
    } while (0)
/*${macro:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*${variable:start}*/
#if (defined CACHE_MAINTAIN) && (CACHE_MAINTAIN == 1)
typedef struct _flexspi_cache_status
{
    volatile bool CacheEnableFlag;
} flexspi_cache_status_t;
#endif
/*${variable:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
static inline void flexspi_clock_init(void)
{
    /* Attach AUX0_PLL clock to flexspi with divider 8*/
    BOARD_SetFlexspiClock(2, 8);
}
/*${prototype:end}*/

#endif /* _APP_H_ */
