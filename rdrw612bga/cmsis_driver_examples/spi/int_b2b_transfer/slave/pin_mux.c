/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "pin_mux.h"
#include "fsl_io_mux.h"

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
/* Function assigned for the Cortex-M33 */
void BOARD_InitPins(void)
{
    IO_MUX_SetPinMux(IO_MUX_FC3_USART_DATA);
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : SPI0_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Cortex-M33 */
void SPI0_InitPins(void)
{
   IO_MUX_SetPinMux(IO_MUX_FC0_SPI_SS0);
   IO_MUX_SetPinConfig(0U, IO_MUX_PinConfigNoPull);
   IO_MUX_SetPinConfig(2U, IO_MUX_PinConfigNoPull);
   IO_MUX_SetPinConfig(3U, IO_MUX_PinConfigNoPull);
   IO_MUX_SetPinConfig(4U, IO_MUX_PinConfigNoPull);
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : SPI0_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Cortex-M33 */
void SPI0_DeinitPins(void)
{

}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
