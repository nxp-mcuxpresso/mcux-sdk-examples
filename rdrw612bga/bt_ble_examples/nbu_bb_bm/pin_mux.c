/*
 * Copyright 2021 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void) {                                /*!< Function assigned for the core: Cortex-M33[cm33] */
   MCI_IO_MUX->FC3 = MCI_IO_MUX_FC3_SEL_FC3_USART_DATA_MASK|MCI_IO_MUX_FC3_SEL_FC3_USART_CMD_MASK;
   AON_SOC_CIU->MCI_IOMUX_EN0 |= 0x05000000U;
   SOCCTRL->MCI_IOMUX_EN0 |= (0x3 << 19) ;  //Enable GPIO19 and GPIO20  on bit 19 and bit 20
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
