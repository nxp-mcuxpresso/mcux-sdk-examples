/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
/* XECC */
#define EXAMPLE_XECC                    XECC_FLEXSPI1
#define EXAMPLE_XECC_IRQ                XECC_FLEXSPI1_INT_IRQn
#define EXAMPLE_XECC_IRQ_HANDLER        XECC_FLEXSPI1_INT_IRQHandler
#define EXAMPLE_XECC_SINGLE_BIT_POSTION 2U

/* FLEXSPI*/
#define EXAMPLE_FLEXSPI           FLEXSPI1
#define FLASH_SIZE                0x10000U
#define EXAMPLE_FLEXSPI_AMBA_BASE FlexSPI1_AMBA_BASE
#define FLASH_PAGE_SIZE           256U
#define EXAMPLE_SECTOR            10U
#define SECTOR_SIZE               0x1000U
#define EXAMPLE_FLEXSPI_CLOCK     kCLOCK_Flexspi1

#define NOR_CMD_LUT_SEQ_IDX_READ_NORMAL        7
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST          13
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD     0
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE        2
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR        3
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE 6
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD   4
#define NOR_CMD_LUT_SEQ_IDX_READID             8
#define NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG     9
#define NOR_CMD_LUT_SEQ_IDX_READSTATUSREG      12
#define NOR_CMD_LUT_SEQ_IDX_ERASECHIP          5

#define CUSTOM_LUT_LENGTH        60
#define FLASH_QUAD_ENABLE        0x40
#define FLASH_BUSY_STATUS_POL    1
#define FLASH_BUSY_STATUS_OFFSET 0
/*${macro:end}*/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
static inline void flexspi_clock_init(void)
{
    /*Clock setting for flexspi1: 12Mhz*/
    CLOCK_SetRootClockDiv(kCLOCK_Root_Flexspi1, 2);
    CLOCK_SetRootClockMux(kCLOCK_Root_Flexspi1, 0);
}
/*${prototype:end}*/

#endif /* _APP_H_ */
