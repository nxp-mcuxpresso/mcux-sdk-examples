/*
 * Copyright 2021 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "pin_mux.h"
#include "fsl_io_mux.h"
#include "fsl_inputmux.h"

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
   CLOCK_EnableClock(kCLOCK_InputMux);
   /* SCT0_IN_1 is selected for SCT0 input 1 */
   INPUTMUX_AttachSignal(INPUTMUX, 1, kINPUTMUX_Gpio4Inp1ToSct0);

   IO_MUX_SetPinMux(IO_MUX_FC3_USART_DATA);
   IO_MUX_SetPinMux(IO_MUX_SCT_OUT_0);
   IO_MUX_SetPinMux(IO_MUX_SCT_IN_1);
   IO_MUX_SetPinMux(IO_MUX_SCT_OUT_8);
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
