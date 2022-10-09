/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*${header:start}*/
#include "pin_mux.h"
#include "board.h"
#include <stdbool.h>
#include "fsl_power.h"
#include "radio.h"

/*${header:end}*/

/*${function:start}*/
void BOARD_InitClocks(void)
{
    /* Already set to mainCLK by default */
    CLOCK_AttachClk(kMAIN_CLK_to_ASYNC_APB);

    /* 32MHz clock */
    CLOCK_EnableClock(kCLOCK_Xtal32M);    /* Normally started already */
    CLOCK_EnableClock(kCLOCK_Fro32M);     /* derived from 192MHz FRO */

    /* 32KHz clock */
    CLOCK_EnableClock(CLOCK_32k_source); /* CLOCK_32k_source can be either Fro32k or Xtal32k */

    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_InputMux);

    /* Enable GPIO for LED controls */
    CLOCK_EnableClock(kCLOCK_Gpio0);

    /* needed for Timer manager if RTC is used */
    //CLOCK_EnableClock(kCLOCK_Rtc);

    CLOCK_AttachClk(kOSC32M_to_USART_CLK);
    CLOCK_EnableClock(kCLOCK_Usart0);
    CLOCK_EnableClock(kCLOCK_Usart1);

    // Alternative setting for UART
    //CLOCK_AttachClk(FRG_CLK_to_USART_CLK);
    //CLOCK_AttachClk(kOSC32M_to_FRG_CLK);

    //CLOCK_AttachClk(kOSC32M_to_I2C_CLK);
    //CLOCK_AttachClk(kFRO48M_to_DMI_CLK);
    //CLOCK_AttachClk(kMAIN_CLK_to_DMI_CLK);

}


void hardware_init(void)
{
    BOARD_common_hw_init();
}

/*${function:end}*/
