/*
 * Copyright 2024 NXP.
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
void BOARD_InitPins(void) {                                /*!< Function assigned for the core: Cortex-M33[cm33] */
    IO_MUX_SetPinMux(IO_MUX_FC3_USART_DATA);
    /* GPIO pin for PIN1 wakeup */
    IO_MUX_SetPinMux(IO_MUX_GPIO25);
#if CONFIG_NCP
    /* GPIO pin to wakeup external host */
    IO_MUX_SetPinMux(IO_MUX_GPIO50);
#if CONFIG_NCP_SPI
    IO_MUX_SetPinMux(IO_MUX_FC0_SPI_SS0);
    IO_MUX_SetPinConfig(0U, IO_MUX_PinConfigNoPull);
    IO_MUX_SetPinConfig(2U, IO_MUX_PinConfigNoPull);
    IO_MUX_SetPinConfig(3U, IO_MUX_PinConfigNoPull);
    IO_MUX_SetPinConfig(4U, IO_MUX_PinConfigNoPull);
    IO_MUX_SetPinMux(IO_MUX_GPIO27);
    IO_MUX_SetPinMux(IO_MUX_GPIO11);
    IO_MUX_SetPinConfig(11U, IO_MUX_PinConfigNoPull);
    IO_MUX_SetPinConfig(27U, IO_MUX_PinConfigNoPull);
#else
    IO_MUX_SetPinMux(IO_MUX_FC0_USART_DATA);
    IO_MUX_SetPinMux(IO_MUX_FC0_USART_CMD);
#endif
#endif
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
